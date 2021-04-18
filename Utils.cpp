
#include "Utils.h"
#include <sys/stat.h>

bool can_exec(const char *file) {
    struct stat st;

    if (stat(file, &st) < 0)
        return false;
    if ((st.st_mode & S_IEXEC) != 0)
        return true;
    return false;
}