#include "signals.h"

#include <signal.h>
#include <unistd.h>

#include <iostream>

#include "Commands.h"
#include "SmallShell.h"
using namespace std;

void ctrlCHandler(int sig_num) {
    // cout << "smash: got ctrl-C" << endl;  //  TODO: Remove call
    write(STDOUT_FILENO, "got CTRL-C\n", strlen("got CTRL-C\n"));

    shared_ptr<ExternalCommand> fg_cmd =
        SmallShell::getInstance().getExternalCommand();

    if (fg_cmd != nullptr) {
        pid_t curr_pid = fg_cmd->getPid();
        if (kill(curr_pid, SIGKILL) != 0) {
            perror("smash error: kill failed");
            return;
        }
        char *msg = strdup(SHELL_NAME ": process ");
        write(STDOUT_FILENO, msg, strlen(msg));
        free(msg);
        msg = strdup(to_string(curr_pid).c_str());
        write(STDOUT_FILENO, msg, strlen(msg));
        free(msg);
        msg = strdup(" was killed\n");
        write(STDOUT_FILENO, msg, strlen(msg));
        free(msg);
        msg = NULL;
        // cout << "smash: process " + to_string(curr_pid) + " was killed" <<
        // endl;
    }
}

void ctrlZHandler(int sig_num) {

    SmallShell *smash = &SmallShell::getInstance();

    cout << "smash: got ctrl-Z" << smash->getExternalCommand() << endl;

    if (smash->getExternalCommand() != nullptr) {
        pid_t curr_pid = smash->getExternalCommand()->getPid();

        if (kill(curr_pid, SIGSTOP) != 0) {
            perror("smash error: kill failed");
            return;
        }
        // TODO: Replace cout << with write() directly
        smash->getJobList().addJob(smash->getExternalCommand(), true);
        cout << "smash: process " + to_string(curr_pid) + " was stopped";
    }
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}
