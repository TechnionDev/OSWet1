
#include "Utils.h"

#include <sys/stat.h>
#include <wordexp.h>
#if 0
#define FUNC_ENTRY() cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

using namespace std;

bool can_exec(const char *file) {
    struct stat st;

    if (stat(file, &st) < 0) {
        return false;
    } else if ((st.st_mode & S_IEXEC) != 0) {
        return true;
    }
    return false;
}

vector<string> split(const string &str) {
    wordexp_t we;
    wordexp(str.c_str(), &we, 0);
    vector<string> argv(we.we_wordc);
    for (int i = 0; i < we.we_wordc; i++) {
        argv[i] = string(we.we_wordv[i]);
    }

    return argv;
}
string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}
string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
string trim(const std::string &s) { return _rtrim(_ltrim(s)); }

int parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *)malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}
bool isBackgroundComamnd(string cmd_line) {
    string trimmed = trim(cmd_line);
    return trimmed[trimmed.length() - 1] == '&';
}
string removeBackgroundSign(string cmd_line) {
    string trimmed = trim(cmd_line);
    if (trimmed[trimmed.length() - 1] == '&') {
        return trimmed.substr(0, trimmed.length() - 1);
    } else {
        return cmd_line;
    }
}
