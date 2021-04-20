#include "signals.h"

#include <signal.h>

#include <iostream>

#include "Commands.h"
#include "SmallShell.h"
using namespace std;

void ctrlZHandler(int sig_num) {
    // TODO: Add your implementation
    cout << "smash: got ctrl-C";
    pid_t curr_pid = SmallShell::getInstance().getExternalCommand()->getPid();
    if (curr_pid != getpid()) {
        if (kill(curr_pid, SIGKILL) != 0) {
            perror("smash error: kill failed");
            return;
        }
        cout << "smash: process " + to_string(curr_pid) + " was killed";
    }
}

void ctrlCHandler(int sig_num) {
    // TODO: Add your implementation
    cout << "smash: got ctrl-Z";
    pid_t curr_pid = SmallShell::getInstance().getExternalCommand()->getPid();
    if (curr_pid != getpid()) {
        if (kill(curr_pid, SIGSTOP) != 0) {
            perror("smash error: kill failed");
            return;
        }
        SmallShell::getInstance().getJobList().addJob(
            SmallShell::getInstance().getExternalCommand(), true);
        cout << "smash: process " + to_string(curr_pid) + " was stopped";
    }
}

void alarmHandler(int sig_num) {
    // TODO: Add your implementation
}
