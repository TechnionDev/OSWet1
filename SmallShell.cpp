#include "SmallShell.h"

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

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
    if (argv.size() == 0) {
        return shared_ptr<Command>(new NopCommand());
    } else {
        try {
            shared_ptr<Command> cmd =
                    commandsCtors.at(argv[0])(subvector(argv, 1, VEC_END));
            return cmd;
        } catch (out_of_range &exc) {
            const shared_ptr<ExternalCommand> cmd(new ExternalCommand(no_background_cmd, isBackgroundComamnd(cmd_s)));
            setExternalCommand(cmd);
            return cmd;
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
        int fds[2] = {0};
        int pid1, pid2;
        if(pipe(fds) != 0){
            throw SyscallException(string("pipe failed") + strerror(errno));
        }
        shared_ptr<Command> cmd1, cmd2;
        // Switch on the type of command
        switch (get<0>(cmd_tuple)) {
            case PIPE:
                cmd1 = this->createCommand(get<1>(cmd_tuple));
                cmd2 = this->createCommand(get<2>(cmd_tuple));
                // Run first command
                if ((pid1 = fork()) == 0) {
                    // Child
                    dup2(fds[1], STDOUT_FILENO);
                    close(fds[0]);
                    close(fds[1]);
                    try {
                        cmd1->execute();
                    } catch (CommandException &exp) {
                        cerr << exp.what() << endl;
                        exit(1);
                    }
                    exit(0);
                }
                if ((pid2 = fork()) == 0) {
                    // Child
                    dup2(fds[0], STDIN_FILENO);
                    close(fds[0]);
                    close(fds[1]);
                    try {
                        cmd2->execute();
                    } catch (CommandException &exp) {
                        cerr << exp.what() << endl;
                        exit(1);
                    }
                    exit(0);
                }
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
                break;
            case PIPE_ERR:
                cmd1 = dynamic_pointer_cast<ExternalCommand>(this->createCommand(get<1>(cmd_tuple)));
                cmd2 = dynamic_pointer_cast<ExternalCommand>(this->createCommand(get<2>(cmd_tuple)));
                // Run first command
                if ((pid1 = fork()) == 0) {
                    // Child
                    dup2(fds[0], STDERR_FILENO);
                    // We don't need the pipe anymore
                    close(fds[0]);
                    close(fds[1]);
                    cmd1->execute();
                }
                if ((pid2 = fork()) == 0) {
                    // Child
                    dup2(fds[1], STDIN_FILENO);
                    // We don't need the pipe anymore
                    close(fds[0]);
                    close(fds[1]);
                    cmd2->execute();
                }

                waitpid(pid1, NULL, WUNTRACED);
                waitpid(pid2, NULL, WUNTRACED);
                break;
        }
        close(fds[0]);
        close(fds[1]);
    }
}

void SmallShell::setPrompt(string new_prompt) { self->prompt = new_prompt; }

string SmallShell::getPrompt() const { return self->prompt + PROMPT_SIGN; }

std::string SmallShell::getLastDir() const { return self->last_dir; }

void SmallShell::setLastDir(std::string new_dir) { self->last_dir = new_dir; }

void SmallShell::setExternalCommand(const shared_ptr<ExternalCommand> &parm_cmd) {
    this->cmd = parm_cmd;
}
