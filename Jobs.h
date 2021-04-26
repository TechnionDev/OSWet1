
#ifndef OSWET1__JOBS_H_
#define OSWET1__JOBS_H_
#include <unistd.h>

#include <list>
#include <memory>
#include <string>

#include "Commands.h"
class ExternalCommand;

class JobsList {
   private:
    class JobEntry {
       public:
        std::shared_ptr<ExternalCommand> cmd;
        time_t time_inserted{};
        bool is_stopped;
        int jod_id;
        JobEntry(std::shared_ptr<ExternalCommand> cmd = nullptr,
                 bool isStopped = false, int job_id = 0);
    };

    // TODO: Add your data members
    int max_jod_id;
    std::list<std::shared_ptr<JobEntry>> jobs;
    std::shared_ptr<JobEntry> foreground_job;
    static bool compare(const std::shared_ptr<JobEntry> first_entry,
                        const std::shared_ptr<JobEntry> second_entry);

   public:
    JobsList();
    ~JobsList();
    void addJob(const std::shared_ptr<ExternalCommand> &cmd,
                bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    std::shared_ptr<JobEntry> getJobById(int jobId);
    void setForegroundJob(int jobId);
    std::shared_ptr<JobEntry> getLastJob(int *lastJobPid);
    std::shared_ptr<JobEntry> getLastStoppedJob(int *jobId);
    int size() { return jobs.size(); }
    // TODO: Add extra methods or modify exisitng ones as needed
};

#endif  // OSWET1__JOBS_H_
