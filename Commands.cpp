#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

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
// TODO: add your implementation
}
SmallShell::~SmallShell() {
// TODO: add your implementation
}
/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command *SmallShell::CreateCommand(const char *cmd_line) {

    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

    if (firstWord.compare("chprompt") == 0) {
        return new ChangePrompt(cmd_line, get_prompt());
    } else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
    } else if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
    } else if (firstWord.compare("cd") == 0) {
        return new ChangeDirCommand(cmd_line, last_pwd);
    } else if (firstWord.compare("jobs") == 0) {
        return new JobsCommand(cmd_line, shell_jobs);
    } else if (firstWord.compare("kill") == 0) {
        return new KillCommand(cmd_line, shell_jobs);
    } else if (firstWord.compare("fg") == 0) {
        return new ForegroundCommand(cmd_line, shell_jobs);
    } else if (firstWord.compare("bg") == 0) {
        return new BackgroundCommand(cmd_line, shell_jobs);
    } else if (firstWord.compare("quit") == 0) {
        return new QuitCommand(cmd_line, shell_jobs);
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
    delete args;
}

BuiltInCommand::BuiltInCommand(const char *cmd_line) : Command(cmd_line) {
}
ChangePrompt::ChangePrompt(const char *cmd_line,
                           std::string *prompt_line) : BuiltInCommand(cmd_line) {
    prompt_line = prompt_line;
}
void ChangePrompt::execute() {
    if (num_of_parms == 0) {
        *prompt_line = DEF_PROMPT;
        return;
    }
    if (num_of_parms == 1) {
        *prompt_line = args[1];//TODO:: NEED TO CHECK IF ITS args[1] OR args[0]
    } else {
        return;
    }
}
