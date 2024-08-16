#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "const.h"
#include "dir.h"
#include "help.h"

void show_main(char* cmd) {
    FILE* file;
    char ch;

    file = fopen(cmd, "r");
    if (file == NULL) handle_error("Error! File cannot be opened.\n");

    while ((ch = fgetc(file)) != EOF) printf("%c", ch);

    fclose(file);
}

void show_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    ensure_aliasme_directory_exists();

    char cmd_path[MAX_PATH_LENGTH] = {0};
    for (int i = 0; i < argc; i++) {
        if (i)
            snprintf(cmd_path + strlen(cmd_path),
                     MAX_PATH_LENGTH - strlen(cmd_path), "/%s", argv[i]);
        else
            snprintf(cmd_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                     ALIASME_DIRECTORY, argv[i]);

        struct stat st = {0};
        if (stat(cmd_path, &st) == -1) handle_error("command does not exist");
    }

    snprintf(cmd_path + strlen(cmd_path), MAX_PATH_LENGTH - strlen(cmd_path),
             "/%s", MAIN);

    show_main(cmd_path);
}
