#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "SmallShell.h"
#include "signals.h"

using std::cin;
using std::cout;
using std::flush;
using std::getline;
using std::string;
using std::endl;

int main(int argc, char *argv[]) {
    if (signal(SIGTSTP, ctrlZHandler) == SIG_ERR) {
        perror(ERR_PREFIX "failed to set ctrl-Z handler");
    }
    if (signal(SIGINT, ctrlCHandler) == SIG_ERR) {
        perror(ERR_PREFIX "failed to set ctrl-C handler");
    }

    // TODO: setup sig alarm handler

    SmallShell &smash = SmallShell::getInstance();
    string cmd_line = "";
    int c;
    while (true) {
        smash.getJobList().removeFinishedJobs();
        cout << smash.getPrompt() << flush;
        // switch ((c = getch())) {
        //     case KEY_UP:
        //         cout << endl << "Up" << endl;  // key up
        //         break;
        //     case KEY_DOWN:
        //         cout << endl << "Down" << endl;  // key down
        //         break;
        //     case KEY_LEFT:
        //         cout << endl << "Left" << endl;  // key left
        //         break;
        //     case KEY_RIGHT:
        //         cout << endl << "Right" << endl;  // key right
        //         break;
        //     case '\n':
        //         smash.executeCommand(cmd_line.c_str());
        //         break;
        //     default:
        //         cmd_line += c;
        //         break;
        // }
        getline(cin, cmd_line);

        try {
            smash.executeCommand(cmd_line);
        } catch (CommandException &exp) {
            cout << exp.what() << endl;
        }

    }
    return 0;
}