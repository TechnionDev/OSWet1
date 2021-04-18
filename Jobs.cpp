#include "Jobs.h"
#include <string>
#include "Constants.h"
#include <iostream>
#include "Exceptions.h"
#include <signal.h>

using namespace std;
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
    throw ItemDoesNotExist(" job-id " + to_string(jobId) + "does not exist");
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
    throw ItemDoesNotExist(" job-id " + to_string(jobId) + "does not exist");
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
    throw ItemDoesNotExist("getLastStoppedJob");
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
//    pid = cmd.
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
