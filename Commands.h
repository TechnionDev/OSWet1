#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define DEF_PROMPT "smash> "
class Command {
public:
    int num_of_parms;
    bool is_bg;
    char **args{};
    Command(const char *cmd_line);
    virtual ~Command();
    virtual void execute() = 0;
    //virtual void prepare();
    //virtual void cleanup();
    // TODO: Add your extra methods if needed
};
class BuiltInCommand : public Command {
public:
    BuiltInCommand(const char *cmd_line);
    virtual ~BuiltInCommand() = default;
};
class ExternalCommand : public Command {
public:
    ExternalCommand(const char *cmd_line);
    virtual ~ExternalCommand() {}
    void execute() override;
};
class PipeCommand : public Command {
    // TODO: Add your data members
public:
    PipeCommand(const char *cmd_line);
    virtual ~PipeCommand() {}
    void execute() override;
};
class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    explicit RedirectionCommand(const char *cmd_line);
    virtual ~RedirectionCommand() {}
    void execute() override;
    //void prepare() override;
    //void cleanup() override;
};
class ChangePrompt : public BuiltInCommand {
private:
    std::string *prompt_line{};
public:
    ChangePrompt(const char *cmd_line, std::string *prompt_line);
    virtual ~ChangePrompt() = default;
    void execute() override;
};
class ChangeDirCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    ChangeDirCommand(const char *cmd_line, char **plastPwd);
    virtual ~ChangeDirCommand() {}
    void execute() override;
};
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);
    virtual ~GetCurrDirCommand() {}
    void execute() override;
};
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);
    virtual ~ShowPidCommand() {}
    void execute() override;
};
class JobsList;
class QuitCommand : public BuiltInCommand {
public:
    //TODO: Add your data members public:
    virtual ~QuitCommand() {}
    void execute() override;
    QuitCommand(const char *cmd_line, JobsList *jobs);
};
class JobsList {
public:
    class JobEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
public:
    JobsList();
    ~JobsList();
    void addJob(Command *cmd, bool isStopped = false);
    void printJobsList();
    void killAllJobs();
    void removeFinishedJobs();
    JobEntry *getJobById(int jobId);
    void removeJobById(int jobId);
    JobEntry *getLastJob(int *lastJobId);
    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};
class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);
    virtual ~KillCommand() {}
    void execute() override;
};
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};
class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmd_line, JobsList *jobs);
    virtual ~BackgroundCommand() {}
    void execute() override;
};
class CatCommand : public BuiltInCommand {
public:
    CatCommand(const char *cmd_line);
    virtual ~CatCommand() {}
    void execute() override;
};
class SmallShell {
private:
    // TODO: Add your data members
    SmallShell();
    JobsList *shell_jobs; //TODO: need to be instantiated in the constructor
    char **last_pwd; //To be used in change_dir
    std::string prompt_line = DEF_PROMPT;
public:
    Command *CreateCommand(const char *cmd_line);
    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~SmallShell();
    void executeCommand(const char *cmd_line);
    std::string *get_prompt() { return &prompt_line; }
};

#endif //SMASH_COMMAND_H_
