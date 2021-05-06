#ifndef OSWET1__EXCEPTIONS_H_
#define OSWET1__EXCEPTIONS_H_
#include <iostream>
#define EXCEPTION(name)                                  \
    class name : public CommandException {               \
    public:                                              \
        name(std::string str) : CommandException(str){}; \
    }

class CommandException : public std::runtime_error {
public:
    CommandException(std::string str) : std::runtime_error(ERR_PREFIX + str){};
};
#define ERROR_M(name) std::cout << name << endl;
EXCEPTION(CommandNotFoundException);
EXCEPTION(MissingRequiredArgumentsException);
EXCEPTION(TooManyArgumentsException);
EXCEPTION(FailedToOpenFileException);
EXCEPTION(ItemDoesNotExist);
EXCEPTION(ListIsEmpty);
EXCEPTION(AlreadyRunningInBackGround);
EXCEPTION(FailedToWaitOnChild);
EXCEPTION(FailedToResumeChild);
EXCEPTION(NoJobProvided);
EXCEPTION(ImpossibleException);
EXCEPTION(TimeoutInvalidArguments);

class SyscallException : public CommandException {
public:
    SyscallException(std::string str) : CommandException("") {
        perror((ERR_PREFIX_NOSPACE + str).c_str());
    }
};


#endif// OSWET1__EXCEPTIONS_H_
