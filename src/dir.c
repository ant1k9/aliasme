#include <stdio.h>
#include <sys/stat.h>

#include "const.h"

void ensure_aliasme_directory_exists() {
    struct stat st = {0};
    char aliasme_path[MAX_PATH_LENGTH] = {0};

    snprintf(aliasme_path, MAX_PATH_LENGTH, "%s/%s", getenv("HOME"),
             ALIASME_DIRECTORY);

    if (stat(aliasme_path, &st) == -1)
        if (mkdir(aliasme_path, 0755))
            handle_error("cannot create aliasme directory");
}
