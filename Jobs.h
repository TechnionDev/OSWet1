#include <unistd.h>
#include <list>
#ifndef OSWET1__JOBS_H_
#define OSWET1__JOBS_H_

class ExternalCommand;

class JobsList {
 private:
  class JobEntry {
   public:
    pid_t pid;
    ExternalCommand *command;
    time_t time_inserted;
    bool is_stopped;
    int jod_id;
    JobEntry(ExternalCommand *cmd, bool isStopped,int max_job_id);
  };
  // TODO: Add your data members
  int max_jod_id;
  std::list<JobEntry> job_list;
  static bool compare(const JobEntry &first_entry, const JobEntry &second_entry);
 public:
  JobsList();
  ~JobsList();
  void addJob(ExternalCommand *cmd, bool isStopped = false,int max_job_id);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry &getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry &getLastJob(int *lastJobId);
  JobEntry &getLastStoppedJob(int *jobId);
  // TODO: Add extra methods or modify exisitng ones as needed
};

#endif //OSWET1__JOBS_H_
