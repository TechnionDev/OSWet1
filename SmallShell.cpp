#include "SmallShell.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <map>
#include <vector>
#include <sys/fcntl.h>
#include "Utils.h"

using namespace std;

typedef enum {
    kCommandCtor
} CommandMapKey;

typedef shared_ptr<Command> (*CommandCtorWrapperFuncPtr)(vector<string>);

template<class T>
shared_ptr<Command> constructorWrapper(vector<string> argv) {
    return shared_ptr<Command>(new T(argv));
}

static const map<string, CommandCtorWrapperFuncPtr> commandsCtors = {
        {"chprompt", &constructorWrapper<ChangePromptCommand>},
        {"showpid",  &constructorWrapper<ShowPidCommand>},
        {"pwd",      &constructorWrapper<GetCurrDirCommand>},
        {"cd",       &constructorWrapper<ChangeDirCommand>},
        {"jobs",     &constructorWrapper<JobsCommand>},
        {"kill",     &constructorWrapper<KillCommand>},
        {"fg",       &constructorWrapper<ForegroundCommand>},
        {"bg",       &constructorWrapper<BackgroundCommand>},
        {"quit",     &constructorWrapper<QuitCommand>},
        {"cat",      &constructorWrapper<CatCommand>}
        /* Add more commands here */
};

SmallShell::SmallShell() : prompt(SHELL_NAME), smash_job_list() {}

SmallShell::~SmallShell() {
}

/**
 * Creates and returns a pointer to Command class which matches the given
 * command line (cmd_line)
 */
shared_ptr<Command> SmallShell::createCommand(string cmd_s) {
    string no_background_cmd = removeBackgroundSign(cmd_s);
    // Remove background sign, trim (part of remove background) and split
    vector<string> argv = split(no_background_cmd);
    int timeout = -1;
    if (argv.empty()) {
        return shared_ptr<Command>(new NopCommand());
    } else {
        if (argv[0] == "timeout") {
            stringstream timeout_ss(argv[1]);
            timeout_ss >> timeout;
            if (timeout_ss.fail()) {
                throw TimeoutInvalidArguments("timeout: invalid arguments");
            }
        }
        try {
            shared_ptr<Command> new_cmd =
                    commandsCtors.at(argv[0])(subvector(argv, 1, VEC_END));
            return new_cmd;
        } catch (out_of_range &exc) {
            string s_cmd_to_run;
            if (timeout != -1) {
                s_cmd_to_run = no_background_cmd.substr(no_background_cmd.find_first_of(argv[1]) + argv[1].length());
            } else {
                s_cmd_to_run = no_background_cmd;
            }
            const shared_ptr<ExternalCommand>
                    new_cmd(new ExternalCommand(s_cmd_to_run, isBackgroundCommand(cmd_s), cmd_s));
            // If background, it will be unset right after execute
            this->setExternalCommand(new_cmd);
            if (timeout != -1) {
                new_cmd->setTimeout(timeout);
            }
            return new_cmd;
        }
    }
}

void SmallShell::parseAndExecuteCommand(string cmd_line) {
    auto cmd_tuple = splitPipeRedirect(cmd_line);

    if (get<0>(cmd_tuple) == NORMAL) {
        shared_ptr<Command> cmd_ptr = createCommand(cmd_line);
        cmd_ptr->execute();
        this->cmd = nullptr;
    } else {
        vector<int> fd_to_close;
        int fds[2] = {0};
        int pid1, pid2;
        int proc1_stdin = -1, proc1_stdout = -1, proc1_stderr = -1, proc2_stdout = -1;

        if (pipe(fds) != 0) {
            throw SyscallException(string("pipe failed") + strerror(errno));
        }

        fd_to_close.push_back(fds[0]);
        fd_to_close.push_back(fds[1]);

        shared_ptr<Command> cmd2 = nullptr;
        FILE *file = NULL;
        // Switch on the type of command
        switch (get<0>(cmd_tuple)) {
            case IN_RD:
                syscall(fopen, file = fopen(get<2>(cmd_tuple).c_str(), "r"));
                proc1_stdin = fileno(file);
                if (proc1_stdin < 0) {
                    throw SyscallException(strerror(errno));
                }
                cmd2 = shared_ptr<Command>(new NopCommand());
                break;
            case OUT_RD_APPEND:
                syscall(fopen, file = fopen(get<2>(cmd_tuple).c_str(), "a"));
                proc2_stdout = fileno(file);
                if (proc2_stdout < 0) {
                    throw SyscallException(strerror(errno));
                }
                proc1_stdout = fds[1];
                cmd2 = shared_ptr<Command>(new RedirectionCommand());
                break;
            case OUT_RD:
                syscall(fopen, file = fopen(get<2>(cmd_tuple).c_str(), "w"));
                proc2_stdout = fileno(file);
                if (proc2_stdout < 0) {
                    throw SyscallException(strerror(errno));
                }
                proc1_stdout = fds[1];
                cmd2 = shared_ptr<Command>(new RedirectionCommand());
                break;
            case PIPE:
                proc1_stdout = fds[1];
                break;
            case PIPE_ERR:
                proc1_stderr = fds[1];
                break;
            case NORMAL:
                throw ImpossibleException("This shouldn't happen in production."
                                          "The if before the switch makes sure of that.");
        }

        // Run first command
        if ((pid1 = fork()) == 0) {
            // Child
            if (proc1_stdout != -1) {
                dup2(proc1_stdout, STDOUT_FILENO);
            }
            if (proc1_stderr != -1) {
                dup2(proc1_stderr, STDERR_FILENO);
            }
            if (proc1_stdin != -1) {
                dup2(proc1_stdin, STDIN_FILENO);
            }
            close(fds[0]);
            close(fds[1]);
            try {
                auto cmd1 = this->createCommand(get<1>(cmd_tuple));
                cmd1->execute();
            } catch (CommandException &exp) {
                cerr << exp.what() << endl;
                exit(1);
            }
            exit(0);
        } else if ((pid2 = fork()) == 0) {
            // Child
            dup2(fds[0], STDIN_FILENO);

            if (proc2_stdout != -1) {
                dup2(proc2_stdout, STDOUT_FILENO);
            }

            close(fds[0]);
            close(fds[1]);
            try {
                if (cmd2 == nullptr) {
                    cmd2 = this->createCommand(get<2>(cmd_tuple));
                }
                cmd2->execute();
            } catch (CommandException &exp) {
                cerr << exp.what() << endl;
                exit(1);
            }
            exit(0);
        } else {
            // This is the main smash process
            // We don't need those fds, only used by children
            close(fds[0]);
            close(fds[1]);
            int stat;
            if (waitpid(pid1, &stat, WUNTRACED) < 0) {
                throw FailedToWaitOnChild("Failed to wait for " +
                                          to_string(pid1) + " " +
                                          strerror(errno));
            } else if (waitpid(pid2, &stat, 0) < 0) {
                throw FailedToWaitOnChild("Failed to wait for " +
                                          to_string(pid2) + " " +
                                          strerror(errno));
            }
            this->cmd = nullptr;
        }
    }
}

void SmallShell::setPrompt(string new_prompt) { self->prompt = new_prompt; }

string SmallShell::getPrompt() const { return self->prompt + PROMPT_SIGN; }

std::string SmallShell::getLastDir() const { return self->last_dir; }

void SmallShell::setLastDir(std::string new_dir) { self->last_dir = new_dir; }

void SmallShell::setExternalCommand(shared_ptr<ExternalCommand> parm_cmd) {
    this->cmd = parm_cmd;
}


void SmallShell::registerTimeoutProcess(int timeout_pid, int timeout_seconds, string command) {
    time_t target_time = time(NULL) + timeout_seconds;
    this->timers.insert(tuple<time_t, pid_t, string>(target_time, timeout_pid, command));
    alarm(get<0>(*this->timers.begin()) - time(NULL));
}

std::set<std::tuple<time_t, pid_t, std::string>> &SmallShell::getTimers() {
    return this->timers;
}

void SmallShell::removeFromTimers(pid_t timeout_pid) {
    auto it = this->timers.begin();

    while (it != this->timers.end()) {
        if (get<1>(*it) == timeout_pid) {
            it = this->timers.erase(it);
            if (it == this->timers.end()) {
                break;
            }
            ++it;
        }
    }
}