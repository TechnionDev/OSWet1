#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <string>
#include <vector>
#include <list>
#include "Constants.h"

#define EXCEPTION(name)                                  \
    class name : public CommandException {               \
       public:                                           \
        name(std::string str) : CommandException(str){}; \
    }

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

class CommandException : public std::runtime_error {
public:
    CommandException(std::string str) : std::runtime_error(ERR_PREFIX + str) {};
};

EXCEPTION(CommandNotFoundException);
EXCEPTION(MissingRequiredArgumentsException);
EXCEPTION(FailedToOpenFileException);

class Command {
public:
    Command() = default;
    virtual ~Command() = default;
    virtual void execute() = 0;
    // virtual void prepare();
    // virtual void cleanup();
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
public:
    BuiltInCommand() = default;
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
    std::string command_name;
public:
    ExternalCommand(std::vector<std::string> &argv);
    virtual ~ExternalCommand() = default;
    void execute() override;
    void setCommandName(std::string name) { command_name = name; }
    std::string getCommandName() { return command_name; }
};

class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(std::vector<std::string> &argv);
    virtual ~PipeCommand() = default;
    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(std::vector<std::string> &argv);
    virtual ~RedirectionCommand() = default;
    void execute() override;
    // void prepare() override;
    // void cleanup() override;
};

class NopCommand : public BuiltInCommand {
public:
    NopCommand() = default;
    virtual ~NopCommand() = default;
    void execute() override {}
};

class ChangePromptCommand : public BuiltInCommand {
    std::string new_prompt;

public:
    ChangePromptCommand(std::vector<std::string> &argv);
    virtual ~ChangePromptCommand() = default;
    void execute() override;
};

class ChangeDirCommand : public BuiltInCommand {
    std::string new_dir;
public:
    // TODO: Add your data members public:
    ChangeDirCommand(std::vector<std::string> &argv);
    virtual ~ChangeDirCommand() = default;
    void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(std::vector<std::string> &argv);
    virtual ~GetCurrDirCommand() = default;
    void execute() override;
};

class ShowPidCommand : public BuiltInCommand {
private:
    int pwd;
public:
    ShowPidCommand(std::vector<std::string> &argv);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
    // TODO: Add your data members public:
    QuitCommand(std::vector<std::string> &argv);
    virtual ~QuitCommand() = default;
    void execute() override;
};

class JobsList {
private:
    class JobEntry {
        // TODO: Add your data members
    public:
        pid_t pid;
        ExternalCommand *command;
        time_t time_inserted;
        bool is_stopped;
        int jod_id;
        JobEntry(ExternalCommand *cmd, bool isStopped);
    };
    // TODO: Add your data members
    static int max_jod_id;
    std::list<JobEntry> job_list;
    static bool compare(const JobEntry &first_entry, const JobEntry &second_entry);
public:
    JobsList();
    ~JobsList();
    void addJob(ExternalCommand *cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry &getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry &getLastJob(int *lastJobId);
    JobEntry &getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};

class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(std::vector<std::string> &argv);
    virtual ~JobsCommand() = default;
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(std::vector<std::string> &argv);
    virtual ~KillCommand() = default;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(std::vector<std::string> &argv);
    virtual ~ForegroundCommand() = default;
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(std::vector<std::string> &argv);
    virtual ~BackgroundCommand() = default;
    void execute() override;
};

class CatCommand : public BuiltInCommand {
    std::vector<std::string> argv;

public:
    CatCommand(std::vector<std::string> &argv);
    virtual ~CatCommand() = default;
    void execute() override;
};

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
    JobsList& getJobList() {return smash_job_list;}
    ~SmallShell();
    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed
};

#endif  // SMASH_COMMAND_H_
