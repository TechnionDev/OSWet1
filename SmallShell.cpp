#include "SmallShell.h"
#include <unistd.h>
#include <map>
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
                new_cmd(new ExternalCommand(no_background_cmd, isBackgroundComamnd(cmd_s),cmd_s));
            setExternalCommand(new_cmd);
            return new_cmd;
        }
    }
}

void SmallShell::executeCommand(string cmd_line) {
    auto cmd_tuple = splitPipeRedirect(cmd_line);
    int fds[2] = {0};
    int old_fds[2] = {STDIN_FILENO, STDIN_FILENO};
    // Switch on the type of command
//    switch (cmd_tuple[0]) {
//        case PIPE:
//            auto cmd1 = this->createCommand(cmd_tuple[1]);
//            auto cmd2 = this->createCommand(cmd_tuple[2]);
//            dup
//            break;
//
//        default:
//
//    }

    if (get<0>(cmd_tuple) == NORMAL) {
        shared_ptr<Command> cmd_new = createCommand(cmd_line);
        // TODO: Handle external isBackground (maybe handle in execute for prettier handling)
        cmd_new->execute();
        this->cmd = nullptr;
    } else {
        shared_ptr<Command> cmd1 = createCommand(get<1>(cmd_tuple)),
            cmd2 = createCommand(get<2>(cmd_tuple));

    }
}

void SmallShell::setPrompt(string new_prompt) { self->prompt = new_prompt; }

string SmallShell::getPrompt() const { return self->prompt + PROMPT_SIGN; }

std::string SmallShell::getLastDir() const { return self->last_dir; }

void SmallShell::setLastDir(std::string new_dir) { self->last_dir = new_dir; }

void SmallShell::setExternalCommand(shared_ptr<ExternalCommand> parm_cmd) {
    this->cmd = parm_cmd;
}
