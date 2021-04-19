
#ifndef OSWET1__JOBS_H_
#define OSWET1__JOBS_H_
#include <unistd.h>
#include <list>
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
    JobEntry(const std::shared_ptr<ExternalCommand> &cmd, bool isStopped, int job_id);
  };
  // TODO: Add your data members
  int max_jod_id;
  std::list<JobEntry> job_list;
  static bool compare(const JobEntry &first_entry, const JobEntry &second_entry);
 public:
  JobsList();
  ~JobsList();
  void addJob(const std::shared_ptr<ExternalCommand> &cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry &getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry &getLastJob(int *lastJobId);
  JobEntry &getLastStoppedJob(int *jobId);
  int size() { return job_list.size(); }
  // TODO: Add extra methods or modify exisitng ones as needed
};

#endif //OSWET1__JOBS_H_
