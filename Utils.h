#ifndef OSWET1__UTILS_H_
#define OSWET1__UTILS_H_

#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#define WHITESPACE " "
#define VEC_END (-1)

std::vector<std::string> split(const std::string &str);

std::string trim(const std::string &s);

int parseCommandLine(const char *cmd_line, char **args);

bool isBackgroundComamnd(std::string cmd_line);

std::string removeBackgroundSign(std::string cmd_line);

bool can_exec(const char *file);

template <class T>
std::vector<T> subvector(const std::vector<T> &vec, int start, int end = -1) {
    return std::vector<std::string>(
        vec.begin() + start, end == -1 ? vec.end() : (vec.begin() + end));
}
#endif  // OSWET1__UTILS_H_
