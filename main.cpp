#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include "SmallShell.h"
#include "Signals.h"

using std::cin;
using std::cout;
using std::endl;
using std::flush;
using std::getline;
using std::string;

int main(int argc, char *argv[]) {
    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR) {
        perror(ERR_PREFIX "failed to set ctrl-Z handler");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror(ERR_PREFIX "failed to set ctrl-C handler");
    }

    { // Alarm handler
        struct sigaction sig_act;
        sig_act.sa_handler = alarmHandler;
        sig_act.sa_flags = SA_RESTART;
        sigemptyset(&sig_act.sa_mask);

        if (sigaction(SIGALRM, &sig_act, NULL)) {
            perror(ERR_PREFIX "failed to set alarm handler");
        }
    }

    // TODO: setup sig alarm handler

    SmallShell &smash = SmallShell::getInstance();
    string cmd_line;
    while (true) {
        cout << smash.getPrompt() << flush;
        getline(cin, cmd_line);
        smash.getJobList().removeFinishedJobs();

        try {
            smash.parseAndExecuteCommand(cmd_line);
        } catch(SyscallException &exp){
            // Do nothing
        }catch (CommandException &exp) {
            std::cerr << exp.what() << endl;
        }
    }
    return 0;
}

