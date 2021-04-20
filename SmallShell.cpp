#include "SmallShell.h"

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

typedef enum { kCommandCtor } CommandMapKey;
typedef shared_ptr<Command> (*CommandCtorWrapperFuncPtr)(vector<string>);

template <class T>
shared_ptr<Command> constructorWrapper(vector<string> argv) {
    return shared_ptr<Command>(new T(argv));
}

static const map<string, CommandCtorWrapperFuncPtr> commandsCtors = {
    {"chprompt", &constructorWrapper<ChangePromptCommand>},
    {"showpid", &constructorWrapper<ShowPidCommand>},
    {"pwd", &constructorWrapper<GetCurrDirCommand>},
    {"cd", &constructorWrapper<ChangeDirCommand>},
    {"jobs", &constructorWrapper<JobsCommand>},
    {"kill", &constructorWrapper<KillCommand>},
    {"fg", &constructorWrapper<ForegroundCommand>},
    {"bg", &constructorWrapper<BackgroundCommand>},
    {"quit", &constructorWrapper<QuitCommand>},
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
shared_ptr<Command> SmallShell::CreateCommand(string cmd_s) {
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
            return shared_ptr<Command>(new ExternalCommand(
                no_background_cmd, isBackgroundComamnd(cmd_s)));
        }
    }
}

void SmallShell::executeCommand(string cmd_line) {
    shared_ptr<Command> cmd = CreateCommand(cmd_line);
    // TODO: Handle external isBackground (maybe handle in execute for prettier
    // handling)
    cmd->execute();
}

void SmallShell::setPrompt(string new_prompt) { self->prompt = new_prompt; }

string SmallShell::getPrompt() const { return self->prompt + PROMPT_SIGN; }

std::string SmallShell::getLastDir() const { return self->last_dir; }

void SmallShell::setLastDir(std::string new_dir) { self->last_dir = new_dir; }