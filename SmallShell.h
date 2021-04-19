#ifndef OSWET1__SMALLSHELL_H_
#define OSWET1__SMALLSHELL_H_
#include "Jobs.h"
#include "Commands.h"
class SmallShell {
 private:
  std::string prompt;
  std::string last_dir;
  SmallShell();
  JobsList smash_job_list;
 public:
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;      // disable copy ctor
  void operator=(SmallShell const &) = delete;  // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
      static SmallShell instance;  // Guaranteed to be destroyed.
      // Instantiated on first use.
      return instance;
  }
  void setPrompt(std::string new_prompt);
  std::string getPrompt() const;
  void setLastDir(std::string new_dir);
  std::string getLastDir() const;
  JobsList &getJobList() { return smash_job_list; }
  ~SmallShell();
  void executeCommand(const char *cmd_line);

  // TODO: add extra methods as needed
};


#endif //OSWET1__SMALLSHELL_H_
