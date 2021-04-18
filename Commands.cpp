#include "Commands.h"

#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using namespace std;

#if 0
#define FUNC_ENTRY() cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

typedef enum { kCommandCtor } CommandMapKey;
typedef Command *(*CommandCtorWrapperFuncPtr)(vector<string>);

template<class T>
Command *constructorWrapper(vector<string> argv) {
    return new T(argv);
}

static const map<string, CommandCtorWrapperFuncPtr> commandsCtors = {
    {"chprompt", &constructorWrapper<ChangePromptCommand>},
    {"showpid", &constructorWrapper<ShowPidCommand>},
    {"pwd", &constructorWrapper<GetCurrDirCommand>},
    {"cd", &constructorWrapper<ChangeDirCommand>},
    {"cat", &constructorWrapper<CatCommand>}
    /* Add more commands here */
};

vector<string> split(const string &str, const string &sep) {
    vector<string> argv;
    for (size_t curr_pos = str.find(sep, 0), prev_pos = 0;
         curr_pos < str.length() || prev_pos < str.length();
         prev_pos = curr_pos + sep.length(),
             curr_pos = str.find(sep, prev_pos)) {
        // No next delim
        if (curr_pos == string::npos) {
            curr_pos = str.length();
        }
        // Retreive current arg
        string arg = str.substr(prev_pos, curr_pos - prev_pos);
        if (!arg.empty()) {
            argv.push_back(arg);
        }
    }
    return argv;
}

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) { return _rtrim(_ltrim(s)); }

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
    size_t idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing
    // spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() : prompt(SHELL_NAME), smash_job_list() {}

SmallShell::~SmallShell() {
    // TODO: add your implementation
}

/**
 * Creates and returns a pointer to Command class which matches the given
 * command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line) {
    // For example:
    /*
      string cmd_s = _trim(string(cmd_line));
      string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

      if (firstWord.compare("pwd") == 0) {
        return new GetCurrDirCommand(cmd_line);
      }
      else if (firstWord.compare("showpid") == 0) {
        return new ShowPidCommand(cmd_line);
      }
      else if ...
      .....
      else {
        return new ExternalCommand(cmd_line);
      }
      */

    string cmd_s = _trim(string(cmd_line));
    char *args[MAX_ARG_COUNT];  // TODO: Support unlimited number of arguments
    // Split on space
    vector<string> argv = split(cmd_s, ARG_SEPARATOR);
    if (argv.size() == 0) {
        return new NopCommand();
    } else {
        try {
            return commandsCtors.at(argv[0])(
                vector<string>(argv.begin() + 1, argv.end()));
        } catch (out_of_range &exc) {
            raise CommandNotFoundException(argv[0]);
        }
    }
}

void SmallShell::executeCommand(const char *cmd_line) {
    Command *cmd = CreateCommand(cmd_line);
    cmd->execute();
}

void SmallShell::setPrompt(string new_prompt) { self->prompt = new_prompt; }

string SmallShell::getPrompt() const { return self->prompt + PROMPT_SIGN; }

std::string SmallShell::getLastDir() const {
    return self->last_dir;
}

void SmallShell::setLastDir(std::string new_dir) {
    self->last_dir = new_dir;
}

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
            raise FailedToOpenFileException(it + ": " + strerror(errno));
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
        raise MissingRequiredArgumentsException(err_msg.str());
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
            cout << "smash error: cd: OLDPWD not set"; //TODO:: check with gur how to raise the exception
        }
        self->new_dir = last_pwd;
        string curr_pwd = getPwd();
        if (chdir(self->new_dir.c_str()) != 0) {
            // Failed
            // TODO: Maybe find a more meaningful exception
            raise CommandException(strerror(errno));
        }
        SmallShell::getInstance().setLastDir(curr_pwd);
    } else {
        string last_pwd = getPwd();
        if (chdir(self->new_dir.c_str()) != 0) {
            // Failed
            // TODO: Maybe find a more meaningful exception
            raise CommandException(strerror(errno));
        }
        SmallShell::getInstance().setLastDir(last_pwd);
    }
}

ShowPidCommand::ShowPidCommand(vector<std::string> &argv) {
}

void ShowPidCommand::execute() {
    cout << "smash pid is " + to_string(getpid());
}

GetCurrDirCommand::GetCurrDirCommand(vector<std::string> &argv) {
}

void GetCurrDirCommand::execute() {
    cout << getPwd();
}

void JobsList::addJob(ExternalCommand *cmd, bool isStopped) {
    removeFinishedJobs();
    max_jod_id++;
    job_list.emplace_back(cmd, isStopped);

}
JobsList::JobsList() {
    max_jod_id = 0;
}
JobsList::JobEntry &JobsList::getJobById(int jobId) {
    for (auto &it : job_list) {
        if (it.jod_id == jobId) {
            return it;
        }
    }
}
void JobsList::removeJobById(int jobId) {
    for (auto it = job_list.begin(); it != job_list.end(); it++) {
        if (it->jod_id == jobId) {
            if (it->jod_id == max_jod_id) {
                max_jod_id--;
            }
            job_list.erase(it);
            return;
        }
    }
}

JobsList::JobEntry &JobsList::getLastJob(int *lastJobId) {
    *lastJobId = job_list.back().jod_id;
    return job_list.back();
}

JobsList::JobEntry &JobsList::getLastStoppedJob(int *jobId) {
    for (auto it = job_list.rbegin(); it != job_list.rend(); ++it) {
        if (it->is_stopped) {
            *jobId = it->jod_id;
            return *it;
        }
    }
}

void JobsList::removeFinishedJobs() {
    for (auto it = job_list.begin(); it != job_list.end(); it++) {
        if (waitpid(it->pid, nullptr, WNOHANG) != 0) {
            if (it->jod_id == max_jod_id) { max_jod_id--; }
            job_list.erase(it);
        }
    }
}

void JobsList::killAllJobs() {
    for (auto it = job_list.begin(); it != job_list.end(); it++) {
        kill(it->pid, SIG_KILL);
        job_list.erase(it);
    }
    max_jod_id = 0;
}

bool JobsList::compare(const JobsList::JobEntry &first_entry, const JobsList::JobEntry &second_entry) {
    if (first_entry.jod_id < second_entry.jod_id) {
        return true;
    } else { return false; }
}

JobsList::JobEntry::JobEntry(ExternalCommand *cmd, bool isStopped) {
    command = cmd;
    is_stopped = isStopped;
    time(&time_inserted);
    jod_id = max_jod_id;
    //TODO:: decide how to get the PID of the job. save it in BuiltCommand or something else
}

void JobsList::printJobsList() {
    removeFinishedJobs();
    job_list.sort(compare);
    for (auto &it : job_list) {
        if (it.is_stopped) {
            cout << "[" + to_string(it.jod_id) + "] " + it.command->getCommandName() + " : " + to_string(it.pid) + " "
                + to_string(difftime(time(nullptr), it.time_inserted)) + " (stopped)";
        } else {
            cout << "[" + to_string(it.jod_id) + "] " + it.command->getCommandName() + " : " + to_string(it.pid) + " "
                + to_string(difftime(time(nullptr), it.time_inserted));
        }
    }
}
