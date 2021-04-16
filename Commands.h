#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define DEF_PROMPT "smash> "
#define MAX_PARAMS_IN_LINE 20
class Command {
public:
    int num_of_parms;
    bool is_bg;
    char *args[MAX_PARAMS_IN_LINE];
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
    char *prompt_line;
public:
    ChangePrompt(const char *cmd_line, char *prompt_line);
    virtual ~ChangePrompt() = default;
    void execute() override;
};
class ChangeDirCommand : public BuiltInCommand {
private:
    char *last_pwd;
public:
    ChangeDirCommand(const char *cmd_line, char *last_pwd);
    virtual ~ChangeDirCommand() = default;
    void execute() override;
};
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line);
    virtual ~GetCurrDirCommand() = default;
    void execute() override;
    char *execute_with_return_val();
};
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};
class JobsList;
class QuitCommand : public BuiltInCommand {
public:
    //TODO: Add your data members public:
    virtual ~QuitCommand() {}
    void execute() override;
    QuitCommand(const char *cmd_line_1,
                const char *cmd_line,
                JobsList *jobs);
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
    JobsCommand(const char *cmd_line_1,
                const char *cmd_line,
                JobsList *jobs);
    virtual ~JobsCommand() {}
    void execute() override;
};
class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line_1,
                const char *cmd_line,
                JobsList *jobs);
    virtual ~KillCommand() {}
    void execute() override;
};
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line_1,
                      const char *cmd_line,
                      JobsList *jobs);
    virtual ~ForegroundCommand() {}
    void execute() override;
};
class BackgroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    BackgroundCommand(const char *cmd_line_1,
                      const char *cmd_line,
                      JobsList *jobs);
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
    char *last_pwd = nullptr; //To be used in change_dir
    char *prompt_line;
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
    char *getLastPwd() { return last_pwd; }
    char *getPrompt() { return prompt_line; }
};

#endif //SMASH_COMMAND_H_
