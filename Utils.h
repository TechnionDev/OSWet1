#ifndef OSWET1__UTILS_H_
#define OSWET1__UTILS_H_

#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#define WHITESPACE " "
using namespace std;
vector<string> split(const string &str, const string &sep);
string _ltrim(const std::string &s);
string _rtrim(const std::string &s);
string _trim(const std::string &s);
int _parseCommandLine(const char *cmd_line, char **args);
bool _isBackgroundComamnd(const char *cmd_line);
void _removeBackgroundSign(char *cmd_line);
bool can_exec(const char *file);
#endif //OSWET1__UTILS_H_
