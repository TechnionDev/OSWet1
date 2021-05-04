#ifndef OSWET1__SMALLSHELL_H_
#define OSWET1__SMALLSHELL_H_

#include <set>
#include "Commands.h"
#include "Jobs.h"

class SmallShell {
private:
    std::string prompt;
    std::string last_dir;
    // Pairs of end-time and the pid of the process
    std::set<std::tuple<time_t, pid_t, std::string>> timers;

    SmallShell();

    JobsList smash_job_list;
    std::shared_ptr<ExternalCommand> cmd = nullptr;
public:
    std::shared_ptr<Command> createCommand(std::string cmd_line);

    SmallShell(SmallShell const &) = delete;      // disable copy ctor
    void operator=(SmallShell const &) = delete;  // disable = operator
    static SmallShell &getInstance()              // make SmallShell singleton
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

    void executeCommand(std::string cmd_line);

    std::shared_ptr<ExternalCommand> getExternalCommand() { return cmd; }

    void setExternalCommand(std::shared_ptr<ExternalCommand> parm_cmd);

    void registerTimeoutProcess(int timeout_pid, int timeout_seconds, std::string command);

    std::set<std::tuple<time_t, pid_t, std::string>> &getTimers();

    void removeFromTimers(pid_t timeout_pid);

};

#endif  // OSWET1__SMALLSHELL_H_