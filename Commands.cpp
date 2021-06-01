#include "Commands.h"

#include <linux/limits.h>
#include <sys/wait.h>
#include <unistd.h>

#include <string.h>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

#include "Exceptions.h"
#include "Jobs.h"
#include "SmallShell.h"
#include "Utils.h"

using namespace std;

ChangePromptCommand::ChangePromptCommand(vector<string> &argv) {
    if (argv.empty()) {
        new_prompt = SHELL_NAME;
    } else {
        new_prompt = argv[0];
    }
}

void ChangePromptCommand::execute() {
    SmallShell::getInstance().setPrompt(self->new_prompt);
}

CatCommand::CatCommand(vector<string> &argv) : argv(argv) {
    if (argv.empty()) {
        throw MissingRequiredArgumentsException("cat: not enough arguments");
    }
}

void CatCommand::execute() {
    ifstream file;
    vector<char> buffer(1024, 0);
    for (const auto &it : argv) {
        // TODO: Add support for piping and redirection
        file.open(it);
        if (!file.is_open()) {
            throw FailedToOpenFileException(string("open failed: ") + strerror(errno));
        }

        do {
            memset(buffer.data(), 0, buffer.size());
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
    return string(getcwd(pwd, PATH_MAX));
}

ChangeDirCommand::ChangeDirCommand(vector<string> &argv) {
    if (argv.empty()) {
        self->new_dir = "";
    } else if (argv.size() > 1) {
        throw TooManyArgumentsException("cd: too many arguments");
    } else {
        self->new_dir = argv[0];  // The rest of the arguments are ignored
    }
}


void ChangeDirCommand::execute() {
    if (self->new_dir == "") {
        return; // Do nothing
    }
    if (self->new_dir == "-") {
        string last_pwd = SmallShell::getInstance().getLastDir();
        if (last_pwd.empty()) {
            throw ItemDoesNotExist("cd: OLDPWD not set");
        }
        self->new_dir = last_pwd;
        string curr_pwd = getPwd();
        if (chdir(self->new_dir.c_str()) != 0) {
            // Failed
            throw CommandException(string("chdir failed: ") + strerror(errno));
        }
        SmallShell::getInstance().setLastDir(curr_pwd);
    } else {
        string last_pwd = getPwd();
        if (chdir(self->new_dir.c_str()) != 0) {
            // Failed
            throw CommandException(string("chdir failed: ") + strerror(errno));
        }
        SmallShell::getInstance().setLastDir(last_pwd);
    }
}

ShowPidCommand::ShowPidCommand(vector<std::string> &argv) {}

void ShowPidCommand::execute() {
    cout << "smash pid is " + to_string(SmallShell::getInstance().getShellPid()) << endl;
}

GetCurrDirCommand::GetCurrDirCommand(vector<std::string> &argv) {}

void GetCurrDirCommand::execute() { cout << getPwd() << endl; }

JobsCommand::JobsCommand(vector<std::string> &argv) {}

void JobsCommand::execute() {
    SmallShell::getInstance().getJobList().printJobsList();
}

KillCommand::KillCommand(vector<string> &argv) {
    try {
        if (argv.size() != 2) {
            throw MissingRequiredArgumentsException("kill: invalid arguments");
        }
        if (argv[0][0] != '-' || argv.size() != 2) {
            throw MissingRequiredArgumentsException("kill: invalid arguments");
        }
        sig_num = stoi(argv[0].substr(1));
        jod_id = stoi(argv[1]);
    } catch (exception &exp) {
        throw MissingRequiredArgumentsException("kill: invalid arguments");
    }
}

void KillCommand::execute() {
    pid_t res_pid;
    SmallShell &smash = SmallShell::getInstance();
    try {
        res_pid = smash.getJobList().getJobById(jod_id)->cmd->getPid();
    } catch (ItemDoesNotExist &exp) {
        string prompt = ERR_PREFIX;
        string error_message = string(exp.what());
        throw ItemDoesNotExist("kill:" + error_message.substr(prompt.length(), error_message.length()));
    }
    if (kill(res_pid, sig_num) != 0) {
        throw CommandException(string("kill failed: ") + strerror(errno));
    }
    cout << "signal number " + to_string(sig_num) + " was sent to pid " + to_string(res_pid) << endl;
    smash.getJobList().removeFinishedJobs();
}

QuitCommand::QuitCommand(vector<std::string> &argv) : kill_all((not argv.empty()) and argv[0] == "kill") {}

void QuitCommand::execute() {
    if (this->kill_all) {
        int size = SmallShell::getInstance().getJobList().size();
        cout << "smash: sending SIGKILL signal to " + to_string(size) +
                " jobs:" << endl;
        SmallShell::getInstance().getJobList().killAllJobs();
    }
    exit(EXIT_SUCCESS);
}

ExternalCommand::ExternalCommand(
        const string &command,
        bool isBackground,
        const string &command_with_background)
        : pid(0), command(command), command_with_background(command_with_background), isBackground(isBackground) {
    if (command.empty()) {
        throw CommandNotFoundException("No command specified");
    }
}

void ExternalCommand::execute() {
    pid_t pid = fork();

    if (pid == -1) {
        throw CommandException(string("fork failed: ") + strerror(errno));
    } else if (pid == 0) {
        setpgrp();

        // Replace with the target process image
        execlp(BASH_PATH, BASH_PATH, "-c", this->command.c_str(), NULL);
        // If we're here, then something failed
        throw CommandException(string("execvp failed: ") + strerror(errno));
    } else {
        this->pid = pid;
        SmallShell &smash = SmallShell::getInstance();
        auto &job_list = smash.getJobList();
        if (this->timeout != -1) {
            smash.registerTimeoutProcess(this->pid, this->timeout, this->command_with_background);
        }
        // Either put in jobslist, or waitpid for the process to finish
        if (this->isBackground) {
            job_list.addJob(smash.getExternalCommand(), false);
        } else {
            int stat = 0;
            if (waitpid(pid, &stat, WUNTRACED) < 0) {
                if (this->timeout != -1) {
                    smash.removeFromTimers(pid);
                }
                throw SyscallException(string("waitpid failed: ") + strerror(errno));
            }
            if (this->timeout != -1) {
                smash.removeFromTimers(pid);
            }
        }
    }
}

string ExternalCommand::getCommandName() const {
    return split(this->command)[0];
}

void ExternalCommand::setTimeout(int to) {
    this->timeout = to;
}

pid_t ExternalCommand::getPid() const { return this->pid; }

string ExternalCommand::getCommand() const { return this->command_with_background; }

bool ExternalCommand::operator==(const ExternalCommand &other) const {
    if (other.isBackground == this->isBackground &&
        other.getCommand() == this->getCommand() &&
        other.getPid() == this->getPid()) { return true; }
    return false;
}

ForegroundCommand::ForegroundCommand(vector<std::string> &argv)
        : job_id(0 /* 0 means last job */) {
    if (argv.size() > 1) {
        throw MissingRequiredArgumentsException("fg: invalid arguments");
    }
    if (argv.size() == 1) {
        try {
            this->job_id = stoi(argv[0]);
        } catch (invalid_argument &exp) {
            throw MissingRequiredArgumentsException("fg: invalid arguments");
        } catch (out_of_range &exp) {
            throw MissingRequiredArgumentsException("fg: invalid arguments");
        }
    }

}

void ForegroundCommand::execute() {
    try {
        pid_t job_pid;
        string job_command;
        shared_ptr<ExternalCommand> foreground_cmd;
        SmallShell &smash = SmallShell::getInstance();
        JobsList &job_list = smash.getJobList();
        if (this->job_id != 0) {
            foreground_cmd = job_list.getJobById(this->job_id)->cmd;
            job_command = foreground_cmd->getCommand();
            job_pid = foreground_cmd->getPid();
        } else {
            foreground_cmd = job_list.getLastJob(&job_pid, &job_id)->cmd;
            job_command = foreground_cmd->getCommand();
        }
        cout << job_command + " : " + to_string(job_pid) << endl;
        if (kill(job_pid, SIGCONT) != 0) {
            throw CommandException(string("kill failed: ") + strerror(errno));
        }
        smash.getJobList().setForegroundJob(this->job_id);
        SmallShell::getInstance().setExternalCommand(foreground_cmd);

        int stat = 0;
        if (waitpid(job_pid, &stat, WUNTRACED) < 0) {
            throw CommandException(string("waitpid failed: ") + strerror(errno));
        }
    } catch (CommandException &exp) {
        string err_prefix = ERR_PREFIX;
        string error_message = string(exp.what());
        throw ItemDoesNotExist(
                "fg: " +
                error_message.substr(err_prefix.length(), error_message.length()));
    } catch (exception &exp) {
        throw exp;
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
        pid_t job_pid = 0;
        string job_command;
        if (this->job_id != 0) {
            if (!job_list.getJobById(this->job_id)->is_stopped) {
                throw AlreadyRunningInBackGround(
                        "job-id " + to_string(this->job_id) + " is already running in the background");
            }
            job_command = job_list.getJobById(this->job_id)->cmd->getCommand();
            job_pid = job_list.getJobById(this->job_id)->cmd->getPid();
        } else {
            job_command =
                    job_list.getLastStoppedJob(&(this->job_id))->cmd->getCommand();
            job_pid = job_list.getLastStoppedJob(&(this->job_id))->cmd->getPid();
        }
        cout << job_command + " : " + to_string(job_pid) << endl;
        job_list.getJobById(this->job_id)->is_stopped = false;
        if (kill(job_pid, SIGCONT) != 0) {
            job_list.getJobById(this->job_id)->is_stopped = true;
            throw CommandException(string("kill failed: ") + strerror(errno));
        }
    } catch (CommandException &exp) {
        string prompt = ERR_PREFIX;
        string error_message = string(exp.what());
        throw ItemDoesNotExist(
                "bg: " +
                error_message.substr(prompt.length(), error_message.length()));
    } catch (exception &exp) {
        throw exp;
    }
}

RedirectionCommand::RedirectionCommand() {}

void RedirectionCommand::execute() {
    int c;
    while ((c = getchar()) != EOF) {
        putchar(c);
    }
}