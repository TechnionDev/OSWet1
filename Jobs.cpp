#include <csignal>
#include <iostream>
#include <string>

#include "Jobs.h"
#include "Constants.h"
#include "Exceptions.h"

using namespace std;

void JobsList::addJob(std::shared_ptr<ExternalCommand> new_cmd, bool isStopped) {
    if (isJobEntryExits(new_cmd)) {
        //can be entered only through CTRL_Z
        shared_ptr<JobEntry> res_cmd = getJobEntryExits(new_cmd);
        time(&(res_cmd->time_inserted));
        res_cmd->is_stopped = true;
        this->last_stopped_job = res_cmd;
        return;
    }
    removeFinishedJobs();
    this->max_jod_id++;
    jobs.emplace_back(new JobEntry(new_cmd, isStopped, this->max_jod_id));
}

JobsList::JobsList() : max_jod_id(0), jobs() {}

shared_ptr<JobsList::JobEntry> JobsList::getJobById(int job_id) {
    for (auto &it : jobs) {
        if (it->jod_id == job_id) {
            return it;
        }
    }
    throw ItemDoesNotExist(" job-id " + to_string(job_id) + " does not exist");
}

void JobsList::setForegroundJob(int job_id) {
    for (auto it = jobs.begin(); it != this->jobs.end(); it++) {
        if ((*it)->jod_id == job_id) {
            (*it)->is_stopped = false;
            return;
        }
    }
    throw ItemDoesNotExist("job-id " + to_string(job_id) + " does not exist");
}

shared_ptr<JobsList::JobEntry> JobsList::getLastJob(int *lastJobPid, int *lastJobId) {
    if (jobs.empty()) {
        throw ListIsEmpty("jobs list is empty");
    }
    this->jobs.sort(compare);
    *lastJobPid = jobs.back()->cmd->getPid();
    *lastJobId = jobs.back()->jod_id;
    return jobs.back();
}

shared_ptr<JobsList::JobEntry> JobsList::getLastStoppedJob(int *jobId) {
    if (last_stopped_job == nullptr) {
        throw ItemDoesNotExist("there is no stopped jobs to resume");
    }
    *jobId = this->last_stopped_job->jod_id;
    return this->last_stopped_job;
}

void JobsList::removeFinishedJobs() {
    jobs.sort(rcompare);
    for (auto it = jobs.begin(); it != jobs.end(); it++) {
        if (waitpid((*it)->cmd->getPid(), nullptr, WNOHANG) != 0) {
            jobs.erase(it);
        }
    }
    // set current max_job_id
    this->max_jod_id = 0;
    for (auto &job : jobs) {
        if (job->jod_id > (this->max_jod_id)) {
            this->max_jod_id = job->jod_id;
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

bool JobsList::compare(shared_ptr<JobEntry> first_entry,
                       shared_ptr<JobEntry> second_entry) {
    if (first_entry->jod_id < second_entry->jod_id) {
        return true;
    } else {
        return false;
    }
}
bool JobsList::rcompare(shared_ptr<JobEntry> first_entry,
                        shared_ptr<JobEntry> second_entry) {
    if (first_entry->jod_id > second_entry->jod_id) {
        return true;
    } else {
        return false;
    }
}

JobsList::JobEntry::JobEntry(std::shared_ptr<ExternalCommand> cmd,
                             bool isStopped, int job_id)
    : is_stopped(isStopped), jod_id(job_id) {
    if (cmd == nullptr) {
        throw NoJobProvided("On JobEntry ctor");
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
                it->cmd->getCommand() + " : " +
                to_string(it->cmd->getPid()) + " " +
                to_string(
                    (int) difftime(time(nullptr), it->time_inserted)) + " secs" +
                " (stopped)" << endl;
        } else {
            cout << "[" + to_string(it->jod_id) + "] " +
                it->cmd->getCommand() + " : " +
                to_string(it->cmd->getPid()) + " " +
                to_string(
                    (int) difftime(time(nullptr), it->time_inserted)) + " secs";
        }
        cout << endl;
    }
}
bool JobsList::isJobEntryExits(shared_ptr<ExternalCommand> parm_cmd) {
    if (getJobEntryExits(parm_cmd)) {
        return true;
    }
    return false;
}
shared_ptr<JobsList::JobEntry> JobsList::getJobEntryExits(std::shared_ptr<ExternalCommand> parm_cmd) {
    for (auto &it :jobs) {
        if (it->cmd == parm_cmd) {
            return it;
        }
    }
    return nullptr;
}
JobsList::~JobsList() = default;
