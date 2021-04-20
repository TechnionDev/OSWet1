#ifndef OSWET1__EXCEPTIONS_H_
#define OSWET1__EXCEPTIONS_H_

#define EXCEPTION(name)                                  \
    class name : public CommandException {               \
       public:                                           \
        name(std::string str) : CommandException(str){}; \
    }

class CommandException : public std::runtime_error {
 public:
  CommandException(std::string str) : std::runtime_error(ERR_PREFIX + str) {};
};

EXCEPTION(CommandNotFoundException);
EXCEPTION(MissingRequiredArgumentsException);
EXCEPTION(FailedToOpenFileException);
EXCEPTION(ItemDoesNotExist);
EXCEPTION(ListIsEmpty);
EXCEPTION(AlreadyRunningInBackGround)


#endif //OSWET1__EXCEPTIONS_H_
