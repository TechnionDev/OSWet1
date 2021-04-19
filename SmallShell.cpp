#include "SmallShell.h"
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
#include "Utils.h"
using namespace std;

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
    {"jobs", &constructorWrapper<JobsCommand>},
    {"kill", &constructorWrapper<KillCommand>},
    {"quit",&constructorWrapper<QuitCommand>},
    {"cat", &constructorWrapper<CatCommand>}
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
Command *SmallShell::CreateCommand(const char *cmd_line) {
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
            // TODO: Run external command here);
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