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
  std::shared_ptr<JobEntry> last_stopped_job = nullptr;
  int max_jod_id;
  std::list<std::shared_ptr<JobEntry>> jobs;
  static bool compare(std::shared_ptr<JobEntry> first_entry, std::shared_ptr<JobEntry> second_entry);
  static bool rcompare(std::shared_ptr<JobEntry> first_entry, std::shared_ptr<JobEntry> second_entry);
  bool isJobEntryExits(std::shared_ptr<ExternalCommand> parm_cmd);
  std::shared_ptr<JobEntry> getJobEntryExits(std::shared_ptr<ExternalCommand> parm_cmd);
 public:
  JobsList();
  ~JobsList();
  void addJob(std::shared_ptr<ExternalCommand> cmd,
              bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  std::shared_ptr<JobEntry> getJobById(int jobId);
  void setForegroundJob(int jobId);
  std::shared_ptr<JobEntry> getLastJob(int *lastJobPid, int *lastJobId);
  std::shared_ptr<JobEntry> getLastStoppedJob(int *jobId);
  int size() { return jobs.size(); }
};

#endif  // OSWET1__JOBS_H_
