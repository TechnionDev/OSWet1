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
    // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
   public:
    BuiltInCommand() = default;
    virtual ~BuiltInCommand() = default;
};

class ExternalCommand : public Command {
    std::string command_name;
    pid_t pid;
    std::vector<std::string> argv;

   public:
    ExternalCommand(std::vector<std::string> &argv);
    virtual ~ExternalCommand() = default;
    void execute() override;
    std::string getCommand() const;
    std::string getCommandName() const;
    pid_t getPid() const;
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

class QuitCommand : public BuiltInCommand {
  // TODO: Add your data members public:
  bool kill_all;
 public:
  QuitCommand(std::vector<std::string> &argv);
  virtual ~QuitCommand() = default;
  void execute() override;
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
    int sig_num;
    int jod_id;

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

#endif  // SMASH_COMMAND_H_
