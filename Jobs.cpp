#include "Jobs.h"

#include <signal.h>

#include <iostream>
#include <string>

#include "Constants.h"
#include "Exceptions.h"
using namespace std;
void JobsList::addJob(const std::shared_ptr<ExternalCommand> &cmd,
                      bool isStopped) {
    removeFinishedJobs();
    this->max_jod_id++;
    jobs.emplace_back(new JobEntry(cmd, isStopped, this->max_jod_id));
}
JobsList::JobsList() : max_jod_id(0), jobs(), foreground_job(NULL) {}

shared_ptr<JobsList::JobEntry> JobsList::getJobById(int job_id) {
    for (auto &it : jobs) {
        if (it->jod_id == job_id) {
            return it;
        }
    }
    throw ItemDoesNotExist(" job-id " + to_string(job_id) + "does not exist");
}

void JobsList::setForegroundJob(int job_id) {
    for (auto it = jobs.begin(); it != this->jobs.end(); it++) {
        if ((*it)->jod_id == job_id) {
            if ((*it)->jod_id == this->max_jod_id) {
                this->max_jod_id--;  // TODO: Fix this bug. There might be
                                     // wholes
            }
            this->foreground_job = *it;
            jobs.erase(it);
            if (kill(this->foreground_job->jod_id, SIGCONT) != 0) {
                perror("smash error: kill failed");
                throw FailedToResumeChild(
                    "child " + to_string(this->foreground_job->jod_id));
            }
            return;
        }
    }
    throw ItemDoesNotExist("job-id " + to_string(job_id) + "does not exist");
}

shared_ptr<JobsList::JobEntry> JobsList::getLastJob(int *lastJobPid) {
    if (jobs.empty()) {
        throw ListIsEmpty("jobs list is empty");
    }
    *lastJobPid = jobs.back()->cmd->getPid();
    return jobs.back();
}

shared_ptr<JobsList::JobEntry> JobsList::getLastStoppedJob(int *job_id) {
    for (auto it = jobs.rbegin(); it != jobs.rend(); ++it) {
        if ((*it)->is_stopped) {
            *job_id = (*it)->jod_id;
            return (*it);
        }
    }
    throw ItemDoesNotExist("there is no stopped jobs to resume");
}

void JobsList::removeFinishedJobs() {
    for (auto it = jobs.begin(); it != jobs.end(); it++) {
        if (waitpid((*it)->cmd->getPid(), nullptr, WNOHANG) != 0) {
            if ((*it)->jod_id == max_jod_id) {
                // TODO: Fix bug
                max_jod_id--;
            }
            jobs.erase(it);
        }
    }
}

void JobsList::killAllJobs() {
    for (auto it = jobs.begin(); it != jobs.end(); it++) {
        kill((*it)->cmd->getPid(), SIG_KILL);
        cout << to_string((*it)->cmd->getPid()) + ": " +
                    (*it)->cmd->getCommand()
             << endl;
        jobs.erase(it);
    }
    max_jod_id = 0;
}

bool JobsList::compare(const shared_ptr<JobEntry> first_entry,
                       const shared_ptr<JobEntry> second_entry) {
    if (first_entry->jod_id < second_entry->jod_id) {
        return true;
    } else {
        return false;
    }
}

JobsList::JobEntry::JobEntry(std::shared_ptr<ExternalCommand> cmd,
                             bool isStopped, int job_id)
    : is_stopped(isStopped), jod_id(job_id) {
    if (cmd == nullptr) {
        throw NoJobProvided("On JobEntry c'tor");
    }
    time(&time_inserted);
    this->cmd = cmd;
}

void JobsList::printJobsList() {
    removeFinishedJobs();
    jobs.sort(compare);
    for (auto &it : jobs) {
        if (it->is_stopped) {
            cout << "[" + to_string(it->jod_id) + "] " +
                        it->cmd->getCommandName() + " : " +
                        to_string(it->cmd->getPid()) + " " +
                        to_string(
                            (int)difftime(time(nullptr), it->time_inserted)) +
                        " (stopped)";
        } else {
            cout << "[" + to_string(it->jod_id) + "] " +
                        it->cmd->getCommandName() + " : " +
                        to_string(it->cmd->getPid()) + " " +
                        to_string(
                            (int)difftime(time(nullptr), it->time_inserted));
        }
        cout << endl;
    }
}
JobsList::~JobsList() {}
