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
    // TODO: add your implementation
}

/**
 * Creates and returns a pointer to Command class which matches the given
 * command line (cmd_line)
 */
shared_ptr<Command> SmallShell::createCommand(string cmd_s) {
    string no_background_cmd = removeBackgroundSign(cmd_s);
    // Remove background sign, trim (part of remove background) and split
    vector<string> argv = split(no_background_cmd);
    if (argv.empty()) {
        return shared_ptr<Command>(new NopCommand());
    } else {
        try {
            shared_ptr<Command> new_cmd =
                    commandsCtors.at(argv[0])(subvector(argv, 1, VEC_END));
            return new_cmd;
        } catch (out_of_range &exc) {
            const shared_ptr<ExternalCommand>
                    new_cmd(new ExternalCommand(no_background_cmd, isBackgroundComamnd(cmd_s), cmd_s));
            setExternalCommand(new_cmd);
            return new_cmd;
        }
    }
}

void SmallShell::executeCommand(string cmd_line) {
    auto cmd_tuple = splitPipeRedirect(cmd_line);

    if (get<0>(cmd_tuple) == NORMAL) {
        shared_ptr<Command> cmd = createCommand(cmd_line);
        // TODO: Handle external isBackground (maybe handle9 in execute for prettier
        // handling)
        cmd->execute();
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

        shared_ptr<Command> cmd1 = nullptr, cmd2 = nullptr;
        // Switch on the type of command
        switch (get<0>(cmd_tuple)) {
            case IN_RD:
                proc1_stdin = fileno(fopen(get<2>(cmd_tuple).c_str(), "r"));
                if (proc1_stdin < 0) {
                    throw SyscallException(strerror(errno));
                }
                cmd2 = shared_ptr<Command>(new NopCommand());
                break;
            case OUT_RD_APPEND:
                proc2_stdout = fileno(fopen(get<2>(cmd_tuple).c_str(), "a"));
                if (proc2_stdout < 0) {
                    throw SyscallException(strerror(errno));
                }
                proc1_stdout = fds[1];
                cmd2 = shared_ptr<Command>(new RedirectionCommand());
                break;
            case OUT_RD:
                proc2_stdout = fileno(fopen(get<2>(cmd_tuple).c_str(), "w"));
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

        cmd1 = this->createCommand(get<1>(cmd_tuple));
        if (cmd2 == nullptr) {
            cmd2 = this->createCommand(get<2>(cmd_tuple));
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
                cmd2->execute();
            } catch (CommandException &exp) {
                cerr << exp.what() << endl;
                exit(1);
            }
            exit(0);
        } else {
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
