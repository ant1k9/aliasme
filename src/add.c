#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "completion.h"
#include "const.h"
#include "dir.h"
#include "help.h"

const char* EXEC_TEMPLATE =
    "#!/usr/bin/env bash\n"
    "aliasme run %s $*";

const char* MAIN_TEMPLATE =
    "#!/usr/bin/env bash\n"
    "# Description: \n"
    "# Help: \n"
    "# Args: false\n"
    "\n"
    "echo %s";

void create_command_directory(char* cmd) {
    struct stat st = {0};
    char cmd_path[MAX_PATH_LENGTH] = {0};

    snprintf(cmd_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
             ALIASME_DIRECTORY, cmd);

    if (stat(cmd_path, &st) == -1) mkdir(cmd_path, 0755);
}

void create_main(char* cmd) {
    char main_path[MAX_PATH_LENGTH] = {0};

    snprintf(main_path, MAX_PATH_LENGTH, "%s/%s/%s/%s", getenv("HOME"),
             ALIASME_DIRECTORY, cmd, MAIN);

    struct stat st = {0};
    if (stat(main_path, &st) != -1) return;

    FILE* file = fopen(main_path, "w");
    fprintf(file, MAIN_TEMPLATE, cmd);
    fclose(file);

    char editor_cmd[MAX_PATH_LENGTH] = {0};
    snprintf(editor_cmd, MAX_PATH_LENGTH - strlen(main_path), "$EDITOR %s",
             main_path);
    if (system(editor_cmd)) handle_error("cannot open file in editor");

    chmod(main_path, 0755);
}

void create_executable(char* cmd) {
    char exec_path[MAX_PATH_LENGTH] = {0};

    snprintf(exec_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
             ALIASME_BIN, cmd);
    struct stat st = {0};
    if (stat(exec_path, &st) != -1) return;

    FILE* file = fopen(exec_path, "w");
    fprintf(file, EXEC_TEMPLATE, cmd);
    fclose(file);

    chmod(exec_path, 0755);
}

void add_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    ensure_aliasme_directory_exists();

    char cmd_path[MAX_PATH_LENGTH] = {0};
    for (int i = 0; i < argc; i++) {
        char* separator = strlen(cmd_path) > 0 ? "/" : "";
        snprintf(cmd_path + strlen(cmd_path),
                 MAX_PATH_LENGTH - strlen(cmd_path), "%s%s", separator,
                 argv[i]);

        create_command_directory(cmd_path);
        create_main(cmd_path);
    }

    create_executable(argv[0]);
    generate_fish_completion(argv[0]);
    printf("Successfully added %s\n", argv[argc - 1]);
}

