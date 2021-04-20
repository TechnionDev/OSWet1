#include "Commands.h"

#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "Exceptions.h"
#include "Jobs.h"
#include "SmallShell.h"
#include "Utils.h"

using namespace std;

// TODO: Add your implementation for classes in Commands.h

ChangePromptCommand::ChangePromptCommand(vector<string> &argv) {
    if (argv.size() == 0) {
        new_prompt = SHELL_NAME;
    } else {
        new_prompt = argv[0];
    }
}

void ChangePromptCommand::execute() {
    SmallShell::getInstance().setPrompt(self->new_prompt);
}

CatCommand::CatCommand(vector<string> &argv) : argv(argv) {}

void CatCommand::execute() {
    ifstream file;
    vector<char> buffer(1024, 0);
    for (auto it : argv) {
        // TODO: Add support for piping and redirection
        file.open(it);
        if (!file.is_open()) {
            throw FailedToOpenFileException(it + ": " + strerror(errno));
        }

        do {
            buffer.empty();
            file.read(buffer.data(), buffer.size());
            if (file.gcount() == 0) {
                break;
            }
            buffer[file.gcount()] = '\0';
            cout << buffer.data();
        } while ((!file.eof()));
        file.close();
        cout << flush;
    }
}

std::string getPwd() {
    char pwd[PATH_MAX];
    return string(getwd(pwd));
}

ChangeDirCommand::ChangeDirCommand(vector<string> &argv) {
    if (argv.size() < 1) {
        ostringstream err_msg;
        err_msg << "Got " << argv.size() << " arguments, expected 1";
        throw MissingRequiredArgumentsException(err_msg.str());
    }
    self->new_dir = argv[0];  // The rest of the arguments are ignored
}

void ChangeDirCommand::execute() {
    if (self->new_dir == "-") {
        string last_pwd = SmallShell::getInstance().getLastDir();
        if (last_pwd.empty()) {
            throw ItemDoesNotExist("cd: OLDPWD not set");
        }
        self->new_dir = last_pwd;
        string curr_pwd = getPwd();
        if (chdir(self->new_dir.c_str()) != 0) {
            // Failed
            // TODO: Maybe find a more meaningful exception
            throw CommandException(strerror(errno));
        }
        SmallShell::getInstance().setLastDir(curr_pwd);
    } else {
        string last_pwd = getPwd();
        if (chdir(self->new_dir.c_str()) != 0) {
            // Failed
            // TODO: Maybe find a more meaningful exception
            throw CommandException(strerror(errno));
        }
        SmallShell::getInstance().setLastDir(last_pwd);
    }
}

ShowPidCommand::ShowPidCommand(vector<std::string> &argv) {}

void ShowPidCommand::execute() {
    cout << "smash pid is " + to_string(getpid()) << endl;
}

GetCurrDirCommand::GetCurrDirCommand(vector<std::string> &argv) {}

void GetCurrDirCommand::execute() { cout << getPwd() << endl; }

JobsCommand::JobsCommand(vector<std::string> &argv) {}

void JobsCommand::execute() {
    SmallShell::getInstance().getJobList().printJobsList();
}

KillCommand::KillCommand(vector<string> &argv) {
    try {
        if (argv[0][0] != '-') {
            throw CommandNotFoundException("kill: invalid arguments");
        }
        sig_num = stoi(argv[0].substr(1));
        jod_id = stoi(argv[1]);
    } catch (exception &exp) {
        throw CommandNotFoundException("kill: invalid arguments");
    }
}

void KillCommand::execute() {
    pid_t res_pid;
    try {
        res_pid = SmallShell::getInstance()
                      .getJobList()
                      .getJobById(jod_id)
                      .cmd->getPid();
    } catch (ItemDoesNotExist &exp) {
        throw exp;
    }
    if (kill(res_pid, sig_num) != 0) {
        perror("smash error: kill failed");
    }
    cout << "signal number " + to_string(sig_num) + " was sent to pid " +
                to_string(res_pid);
}

QuitCommand::QuitCommand(vector<std::string> &argv) {
    if (argv.size() > 0 and argv[0] == "kill") {
        this->kill_all = true;
    }
}

void QuitCommand::execute() {
    if (this->kill_all) {
        int size = SmallShell::getInstance().getJobList().size();
        cout << "smash: sending SIGKILL signal to " + to_string(size) +
                    " jobs:";
        SmallShell::getInstance().getJobList().killAllJobs();
    }
    exit(EXIT_SUCCESS);
}

ExternalCommand::ExternalCommand(string command, bool isBackground)
    : pid(0), command(command), isBackground(isBackground) {
    if (command == "") {
        throw CommandNotFoundException("No command specified");
    } /* else if (not can_exec(argv[0].c_str())) {
        throw CommandNotFoundException("Command " + argv[0] + " not found");
    } */
}

void ExternalCommand::execute() {
    pid_t pid = fork();
    if (pid == 0) {
        // Forked - setup arguments
        setpgrp();
        char **argv = new char *[4];
        argv[0] = strdup(BASH_PATH);
        argv[1] = strdup("-c");
        argv[2] = strdup(this->command.c_str());
        argv[3] = NULL;

        // Replace with the target process image
        execvp(argv[0], argv);
        // If we're here, then something failed
        perror(this->command.c_str());
        return;
    } else {
        this->pid = pid;
        if (not this->isBackground) {
            int stat = 0;
            if (waitpid(pid, &stat, 0) < 0) {
                throw FailedToWaitOnChild("Failed to wait for " +
                                          to_string(pid) + " " +
                                          strerror(errno));
            }
        }
    }
}

string ExternalCommand::getCommandName() const {
    return split(this->command)[0];
}

pid_t ExternalCommand::getPid() const { return this->pid; }

string ExternalCommand::getCommand() const { return this->command; }

ForegroundCommand::ForegroundCommand(vector<std::string> &argv) {
    try {
        if (argv.size() > 1) {
            throw MissingRequiredArgumentsException("fg: invalid arguments");
        }
        if (argv.size() == 1) {
            job_id = stoi(argv[0]);
        }
    } catch (exception &exp) {
        throw MissingRequiredArgumentsException("fg: invalid arguments");
    }
}

void ForegroundCommand::execute() {
    try {
        pid_t job_pid;
        string job_command;
        if (job_id != 0) {
            job_command = SmallShell::getInstance()
                              .getJobList()
                              .getJobById(job_id)
                              .cmd->getCommand();
            job_pid = SmallShell::getInstance()
                          .getJobList()
                          .getJobById(job_id)
                          .cmd->getPid();
        } else {
            job_command = SmallShell::getInstance()
                              .getJobList()
                              .getLastJob(&job_pid)
                              .cmd->getCommand();
        }
        cout << job_command + " : " + to_string(job_pid);
        if (kill(job_pid, SIGCONT) != 0) {
            perror("smash error: kill failed");
            return;
        }
        SmallShell::getInstance().getJobList().removeJobById(job_pid);
        if (waitpid(job_pid, nullptr, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
            return;
        }
    } catch (CommandException &exp) {
        string prompt = ERR_PREFIX;
        string error_message = string(exp.what());
        throw ItemDoesNotExist(
            " fg:" +
            error_message.substr(prompt.length(), error_message.length()));
    }
}

BackgroundCommand::BackgroundCommand(vector<std::string> &argv) {
    try {
        if (argv.size() > 1) {
            throw MissingRequiredArgumentsException("bg: invalid arguments");
        }
        if (argv.size() == 1) {
            job_id = stoi(argv[0]);
        }
    } catch (exception &exp) {
        throw MissingRequiredArgumentsException("bg: invalid arguments");
    }
}

void BackgroundCommand::execute() {
    try {
        JobsList &job_list = SmallShell::getInstance().getJobList();
        pid_t job_pid;
        string job_command;
        if (job_id != 0) {
            if (!job_list.getJobById(job_id).is_stopped) {
                throw AlreadyRunningInBackGround(
                    "job-id " + to_string(job_id) +
                    " is already running in the background");
            }
            job_command = job_list.getJobById(job_id).cmd->getCommand();
            job_pid = job_list.getJobById(job_id).cmd->getPid();
        } else {
            job_command =
                job_list.getLastStoppedJob(&job_pid).cmd->getCommand();
        }
        cout << job_command + " : " + to_string(job_pid);
        job_list.getJobById(job_id).is_stopped = false;
        if (kill(job_pid, SIGCONT) != 0) {
            job_list.getJobById(job_id).is_stopped = true;
            perror("smash error: kill failed");
            return;
        }
    } catch (CommandException &exp) {
        string prompt = ERR_PREFIX;
        string error_message = string(exp.what());
        throw ItemDoesNotExist(
            " bg:" +
            error_message.substr(prompt.length(), error_message.length()));
    }
}
