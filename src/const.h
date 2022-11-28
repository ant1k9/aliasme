#pragma once
#include <stdlib.h>

#define MAX_PATH_LENGTH 1024
#define MAX_ARGS_BUFFER 1024
#define MAX_CONDITION_BUFFER 1024
#define MAX_COMPLETION_DEPTH 64
#define HEADER_SIZE 256

#define ADD "add"
#define EDIT "edit"
#define REMOVE "rm"
#define RUN "run"
#define GENERATE "generate"

#ifndef ALIASME_DIRECTORY
#define ALIASME_DIRECTORY ".local/share/aliasme"
#endif
#define ALIASME_BIN "bin"
#define MAIN "_main"
#define FISH_COMPLETION_DIRECTORY ".config/fish/completions"

#define UNUSED(x) (void)(x)

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)
