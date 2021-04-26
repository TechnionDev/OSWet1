#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <list>
#include <string>
#include <vector>

#include "Constants.h"
#include "Exceptions.h"

class Command {
   public:
    Command() = default;
    virtual ~Command() = default;
    virtual void execute() = 0;
    // virtual void prepare();
    // virtual void cleanup();
};

class BuiltInCommand : public Command {
   public:
    BuiltInCommand() = default;
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
    pid_t pid;
    std::string command;
    bool isBackground;

   public:
    ExternalCommand(const std::string& command, bool isBackground);
    virtual ~ExternalCommand() = default;
    void execute() override;
    std::string getCommand() const;
    std::string getCommandName() const;
    pid_t getPid() const;
};

class PipeCommand : public Command {
   public:
    PipeCommand(std::vector<std::string> &argv);
    virtual ~PipeCommand() = default;
    void execute() override;
};

class RedirectionCommand : public Command {
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
    int pwd{};

   public:
    ShowPidCommand(std::vector<std::string> &argv);
    virtual ~ShowPidCommand() = default;
    void execute() override;
};

class QuitCommand : public BuiltInCommand {
  bool kill_all;
 public:
  QuitCommand(std::vector<std::string> &argv);
  virtual ~QuitCommand() = default;
  void execute() override;
};

class JobsCommand : public BuiltInCommand {
   public:
    JobsCommand(std::vector<std::string> &argv);
    virtual ~JobsCommand() = default;
    void execute() override;
};

class KillCommand : public BuiltInCommand {
    int sig_num;
    int jod_id;

   public:
    KillCommand(std::vector<std::string> &argv);
    virtual ~KillCommand() = default;
    void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
    int job_id = 0;
   public:
    ForegroundCommand(std::vector<std::string> &argv);
    virtual ~ForegroundCommand() = default;
    void execute() override;
};

class BackgroundCommand : public BuiltInCommand {
    int job_id = 0;
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

#endif  // SMASH_COMMAND_H_
