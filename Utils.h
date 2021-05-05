#ifndef OSWET1__UTILS_H_
#define OSWET1__UTILS_H_

#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <tuple>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

#include "Constants.h"

// TODO: Remove debug flag
#define _DEBUG_

#define VEC_END (-1)
#ifdef _DEBUG_
#define log(msg) cout << msg << endl
#else
#define log(msg)
#endif

typedef enum {
    PIPE = 1,
    PIPE_ERR = 2,
    OUT_RD = 3,
    OUT_RD_APPEND = 4,
    IN_RD = 5,
    NORMAL = 6
} CommandType;

std::tuple<CommandType, std::string, std::string> splitPipeRedirect(const std::string &str);

std::vector<std::string> split(const std::string &str);

std::string trim(const std::string &s);

int parseCommandLine(const char *cmd_line, char **args);

bool isBackgroundCommand(const std::string &cmd_line);

std::string removeBackgroundSign(std::string cmd_line);

bool can_exec(const char *file);

template<class T>
std::vector<T> subvector(const std::vector<T> &vec, int start, int end = -1) {
    return std::vector<std::string>(
            vec.begin() + start, end == -1 ? vec.end() : (vec.begin() + end));
}

#endif  // OSWET1__UTILS_H_
