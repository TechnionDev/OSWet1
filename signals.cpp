#include "signals.h"

#include <signal.h>
#include <unistd.h>

#include <iostream>

#include "Commands.h"
#include "SmallShell.h"

using namespace std;

void ctrlCHandler(int sig_num) {
    // cout << "smash: got ctrl-C" << endl;  //  TODO: Remove call
    write(STDOUT_FILENO, "smash: got ctrl-C\n", strlen("smash: got ctrl-C\n"));
    SmallShell &smash = SmallShell::getInstance();
    shared_ptr<ExternalCommand> fg_cmd = smash.getExternalCommand();

    if (fg_cmd != nullptr) {
        pid_t curr_pid = fg_cmd->getPid();
        smash.removeFromTimers(curr_pid);
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

//    SmallShell *smash = &SmallShell::getInstance();

    cout << "smash: got ctrl-Z" << endl;
    SmallShell &smash = SmallShell::getInstance();

    if (smash.getExternalCommand() != nullptr) {
        pid_t curr_pid = smash.getExternalCommand()->getPid();
        if (kill(curr_pid, SIGSTOP) != 0) {
            perror("smash error: kill failed");
            return;
        }
        //TODO: Replace cout << with write() directly
        smash.getJobList().addJob(smash.getExternalCommand(), true);
        cout << "smash: process " + to_string(curr_pid) + " was stopped" << endl;
        //smash->setExternalCommand(nullptr);
    }
}

void alarmHandler(int sig_num) {
    SmallShell &smash = SmallShell::getInstance();
    shared_ptr<ExternalCommand> cmd = smash.getExternalCommand();
    time_t now = time(NULL);
    auto &timers = smash.getTimers();
    auto it = timers.begin();

    while (it != timers.end()) {
        if (get<0>(*it) > now) {
            alarm(get<0>(*it) - now);
            break;
        } else {
            cout << "smash: got an alarm" << endl;
            cout << SHELL_NAME << ": " << get<2>(*it) << " timed out!" << endl;
            kill(get<1>(*it), SIGKILL);
            it = timers.erase(it);
        }
    }
}
