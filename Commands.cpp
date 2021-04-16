#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <cstdio>
using namespace std;
#define WHITESPACE ' '
#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}
int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}
bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}
void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h 

SmallShell::SmallShell() {
    prompt_line = (char *) malloc(strlen(DEF_PROMPT) + 1);
    strcpy(prompt_line, DEF_PROMPT);
}
SmallShell::~SmallShell() {

}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("chprompt") == 0) {
        return new ChangePrompt(cmd_line, prompt_line);

    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);

    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);

    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line, getLastPwd());

    } else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(nullptr, cmd_line, shell_jobs);
    } else if (firstWord.compare("kill") == 0) {
        return new KillCommand(nullptr, cmd_line, shell_jobs);
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(nullptr, cmd_line, shell_jobs);
    } else if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(nullptr, cmd_line, shell_jobs);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(nullptr, cmd_line, shell_jobs);
    } else {
        return new ExternalCommand(cmd_line);
    }
    return nullptr;
}
void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    Command *cmd = CreateCommand(cmd_line);
    cmd->execute();
    delete cmd;
    // Please note that you must fork smash process for some commands (e.g., external commands....)
}

Command::Command(const char *cmd_line) {
    num_of_parms = _parseCommandLine(cmd_line, args);
    is_bg = _isBackgroundComamnd(cmd_line);
}
Command::~Command() {
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {
}
ChangePrompt::ChangePrompt(const char *cmd_line,
                           char *prompt_line_parm) : BuiltInCommand(cmd_line) {
    prompt_line = prompt_line_parm;
}
void ChangePrompt::execute() {
    if (num_of_parms == 1) {
        prompt_line = (char *) realloc(prompt_line, strlen(DEF_PROMPT) + 1);
        strcpy(prompt_line, DEF_PROMPT);
    } else if (num_of_parms == 2) {
        prompt_line = (char *) realloc(prompt_line, strlen(args[1]) + 1);
        strcpy(prompt_line, strcat(args[1], ">"));
    }
}
ShowPidCommand::ShowPidCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {
}
void ShowPidCommand::execute() {
    std::cout << "smash pid is " + to_string(getpid());
}
GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}
void GetCurrDirCommand::execute() {
    /***
     * As an extension to the POSIX.1-2001 standard, glibc's getcwd()
       allocates the buffer dynamically using malloc(3) if buf is NULL.
       In this case, the allocated buffer has the length size unless
       size is zero, when buf is allocated as big as necessary.  The
       caller should free(3) the returned buffer.

     */
    char *buf = nullptr;
    char *res = getcwd(buf, 0);
    if (res) {
        cout << res;
        delete[] buf;
    } else {
        perror("smash error: getcwd failed");
    }
}
char *GetCurrDirCommand::execute_with_return_val() {
    char *buf = nullptr;
    char *res = getcwd(buf, 0);
    if (res) {
        delete[] buf;
        return res;
    } else {
        perror("smash error: getcwd failed");
    }
}
ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char *pwd) : BuiltInCommand(cmd_line) {
    last_pwd = pwd;
}
void ChangeDirCommand::execute() {
    if (num_of_parms > 2) {
        cout << "smash error: cd: too many arguments";
    } else if (strcmp(args[1], "-") == 0) {
        if (last_pwd == nullptr) {
            cout << "smash error: cd: OLDPWD not set";
            return;
        }
        GetCurrDirCommand pwd_instance("");
        char *ret_pwd = pwd_instance.execute_with_return_val();
        int ret = chdir(last_pwd);
        if (ret == -1) {
            delete[] ret_pwd;
            perror("smash error: chdir failed");
        }
        last_pwd = (char *) realloc(last_pwd, strlen(ret_pwd) + 1);
        strcpy(last_pwd, ret_pwd);
    } else {
        GetCurrDirCommand pwd_instance("");
        char *ret_pwd = pwd_instance.execute_with_return_val();
        int ret = chdir(args[1]);
        if (ret == -1) {
            delete[] ret_pwd;
            perror("smash error: chdir failed");
        }
        last_pwd = (char *) realloc(last_pwd, strlen(ret_pwd) + 1);
        strcpy(last_pwd, ret_pwd);
    }
}

ExternalCommand::ExternalCommand(const char *cmd_line) : Command(cmd_line) {

}
void ExternalCommand::execute() {

}
PipeCommand::PipeCommand(const char *cmd_line) : Command(cmd_line) {

}
void PipeCommand::execute() {

}
RedirectionCommand::RedirectionCommand(const char *cmd_line) : Command(cmd_line) {

}
void RedirectionCommand::execute() {

}
QuitCommand::QuitCommand(const char *cmd_line_1,
                         const char *cmd_line,
                         JobsList *jobs) : BuiltInCommand(cmd_line_1) {

}
void QuitCommand::execute() {

}
JobsList::JobEntry *JobsList::getLastStoppedJob(int *jobId) {
    return nullptr;
}
JobsList::JobEntry *JobsList::getLastJob(int *lastJobId) {
    return nullptr;
}
void JobsList::removeJobById(int jobId) {

}
JobsList::JobEntry *JobsList::getJobById(int jobId) {
    return nullptr;
}
void JobsList::removeFinishedJobs() {

}
void JobsList::killAllJobs() {

}
void JobsList::printJobsList() {

}
void JobsList::addJob(Command *cmd, bool isStopped) {

}
JobsList::~JobsList() {

}
JobsList::JobsList() {

}
JobsCommand::JobsCommand(const char *cmd_line_1,
                         const char *cmd_line,
                         JobsList *jobs) : BuiltInCommand(cmd_line_1) {

}
void JobsCommand::execute() {

}
KillCommand::KillCommand(const char *cmd_line_1,
                         const char *cmd_line,
                         JobsList *jobs) : BuiltInCommand(cmd_line_1) {

}
void KillCommand::execute() {

}
void ForegroundCommand::execute() {

}
ForegroundCommand::ForegroundCommand(const char *cmd_line_1,
                                     const char *cmd_line,
                                     JobsList *jobs) : BuiltInCommand(cmd_line_1) {

}
void BackgroundCommand::execute() {

}
BackgroundCommand::BackgroundCommand(const char *cmd_line_1,
                                     const char *cmd_line,
                                     JobsList *jobs) : BuiltInCommand(cmd_line_1) {

}
CatCommand::CatCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {

}
void CatCommand::execute() {

}
