#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include "Commands.h"
#include "SmallShell.h"
#include "Jobs.h"
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

ChangeDirCommand::ChangeDirCommand(vector<string> &argv) {
    if (argv.size() < 1) {
        ostringstream err_msg;
        err_msg << "Got " << argv.size() << " arguments, expected 1";
        throw MissingRequiredArgumentsException(err_msg.str());
    }
    self->new_dir = argv[0];  // The rest of the arguments are ignored
}

std::string getPwd() {
    char pwd[PATH_MAX];
    return string(getwd(pwd));
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
    cout << "smash pid is " + to_string(getpid());
}

GetCurrDirCommand::GetCurrDirCommand(vector<std::string> &argv) {}

void GetCurrDirCommand::execute() { cout << getPwd(); }

JobsCommand::JobsCommand(vector<std::string> &argv) {}

void JobsCommand::execute() {
    SmallShell::getInstance().getJobList().printJobsList();
}

KillCommand::KillCommand(vector<string> &argv) {
    if (argv[0][0] != '-') {
        throw (CommandNotFoundException("kill: invalid arguments"));
    }
    sig_num = int(argv[0][1]);
    jod_id = int(argv[1][0]);
}

void KillCommand::execute() {
    pid_t res_pid;
    try {
        res_pid = SmallShell::getInstance().getJobList().getJobById(jod_id).cmd->getPid();
    } catch (ItemDoesNotExist &exp) {
        throw exp;
    }
    if (kill(res_pid, SIG_KILL) != 0) {
        perror("smash error: kill failed");
    }
    cout << "signal number 9 was sent to pid " + to_string(sig_num);
}

QuitCommand::QuitCommand(vector<std::string> &argv) {
    if (argv[0] == "kill") {
        kill_all = true;
    }
}

void QuitCommand::execute() {
    if (kill_all) {
        int size = SmallShell::getInstance().getJobList().size();
        cout << "smash: sending SIGKILL signal to " + to_string(size) + " jobs:";
        SmallShell::getInstance().getJobList().killAllJobs();
    }
    exit(0);
}

ExternalCommand::ExternalCommand(vector<string> &argv) : pid(0), argv(argv) {
    if (argv.size() == 1) {
        throw CommandNotFoundException("No command specified");
    } else if (not can_exec(argv[0].c_str())) {
        throw CommandNotFoundException("Command " + argv[0] + " not found");
    }
}

void ExternalCommand::execute() {
    pid_t pid = fork();
    if (pid == 0) {
        // Forked
        // const int size = this->argv.size();
        // char *argv[size];
        // for (int i = 0; i < size; i++) {
        //     argv[i] = new char[this->argv[i].length()];
        //     strcpy(argv[i], this->argv[i].c_str());
        // }
        // execvp(this->argv[0].c_str(), argv);
        char *argv[3];
        string cmd = this->getCommand();
        argv[0] = BASH_PATH;
        argv[1] = "-c";
        argv[2] = new char[cmd.length()];
        strcpy(argv[2], cmd.c_str());
        execvp(argv[0], argv);
    } else {
        this->pid = pid;
    }
}

string ExternalCommand::getCommandName() const { return this->argv[0]; }
pid_t ExternalCommand::getPid() const { return this->pid; }
string ExternalCommand::getCommand() const {
    const char *const delim = " ";

    ostringstream imploded;
    copy(this->argv.begin(), this->argv.end(),
         ostream_iterator<string>(imploded, delim));

    return imploded.str();
}

ForegroundCommand::ForegroundCommand(vector<std::string> &argv) {
    if (argv.size() > 1) {
        throw MissingRequiredArgumentsException("fg: invalid arguments");
    }
    if (argv.size() == 1) {
        jod_id = stoi(argv[0]);
    }
}
void ForegroundCommand::execute() {
    try {
        pid_t job_pid;
        string job_command;
        if (jod_id != 0) {
            job_command = SmallShell::getInstance().getJobList().getJobById(jod_id).cmd->getCommand();
            job_pid = SmallShell::getInstance().getJobList().getJobById(jod_id).cmd->getPid();
        } else {
            job_command = SmallShell::getInstance().getJobList().getLastJob(&job_pid).cmd->getCommand();
        }
        cout << job_command + " : " + to_string(job_pid);
        if (kill(job_pid, SIGCONT) != 0) {
            perror("smash error: kill failed");
            return;
        }
        SmallShell::getInstance().getJobList().removeJobById(job_pid);
        int wStatus;
        if (waitpid(job_pid, &wStatus, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
            return;
        }
    } catch (CommandException &exp) {
        string prompt = ERR_PREFIX;
        string error_message = string(exp.what());
        throw ItemDoesNotExist(" fg:" + error_message.substr(prompt.length(), error_message.length()));
    }
}
