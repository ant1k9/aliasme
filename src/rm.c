#define _GNU_SOURCE

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "completion.h"
#include "const.h"
#include "help.h"

int unlink_cb(const char* fpath, const struct stat* sb, int typeflag,
              struct FTW* ftwbuf) {
    UNUSED(sb);
    UNUSED(typeflag);
    UNUSED(ftwbuf);

    int rv = remove(fpath);
    if (rv) handle_error("cannot remove cmd");
    return rv;
}

void remove_directory(char* dir) {
    nftw(dir, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void remove_fish_completion(char* cmd) {
    struct stat st = {0};
    char completion_path[MAX_PATH_LENGTH] = {0};

    snprintf(completion_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
             FISH_COMPLETION_DIRECTORY, cmd);

    if (stat(completion_path, &st) != -1) remove(completion_path);
}

void remove_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    struct stat st = {0};
    char cmd_path[MAX_PATH_LENGTH] = {0};

    for (int i = 0; i < argc; i++) {
        if (i) {
            snprintf(cmd_path + strlen(cmd_path),
                     MAX_PATH_LENGTH - strlen(cmd_path), "/%s", argv[i]);
            continue;
        }
        snprintf(cmd_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                 ALIASME_DIRECTORY, argv[i]);
    }

    if (stat(cmd_path, &st) != -1) remove_directory(cmd_path);

    if (argc == 1) {
        char exec_path[MAX_PATH_LENGTH] = {0};
        snprintf(exec_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                 ALIASME_BIN, argv[1]);
        remove(exec_path);
        remove_fish_completion(argv[0]);
    } else
        generate_fish_completion(argv[0]);
}
