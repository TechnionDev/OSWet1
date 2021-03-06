#ifndef SMASH_CONSTANTS_H_
#define SMASH_CONSTANTS_H_

// Metrics
#define MAX_LINE_LEN 80
#define MAX_ARG_COUNT 20
#define MAX_JOBS_COUNT 100
#define MAX_PROC_NAME 50

// String constants
#define BASH_PATH "/bin/bash"
#define SHELL_NAME "smash"
#define MSG_PREFIX SHELL_NAME ": "
#define ERR_PREFIX_NOSPACE SHELL_NAME " error:"
#define ERR_PREFIX ERR_PREFIX_NOSPACE " "
#define PROMPT_SIGN "> "
#define ERROR(x) (ERR_PREFIX_NOSPACE PROMPT_SIGN "\"" x "\"")
#define WHITESPACE " \n\r\t\f\v"
//
#define SIG_KILL 9
// Aliasing macros
#define self this

#endif
