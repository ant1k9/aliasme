#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const char* USAGE =
    "Usage:\n"
    "\taliasme <command> ...\n";

const char* EXEC_TEMPLATE =
    "#!/usr/bin/env bash\n"
    "aliasme run %s";

const char* MAIN_TEMPLATE =
    "#!/usr/bin/env bash\n"
    "# Description: \n"
    "# Args: 0\n"
    "\n"
    "echo %s";

const char* ADD = "add";
const char* RUN = "run";

const char* ALIASME_DIRECTORY = ".local/share/aliasme";
const char* ALIASME_BIN = "bin";
const char* MAIN = "_main";

#define handle_error(msg)     \
    do {                      \
        fprintf(stderr, msg); \
        exit(EXIT_FAILURE);   \
    } while (0)

void usage() {
    printf("%s", USAGE);
    exit(0);
}

void ensure_aliasme_directory_exists() {
    struct stat st = {0};
    char aliasme_path[1024] = {0};

    snprintf(aliasme_path, 1024, "%s/%s", getenv("HOME"), ALIASME_DIRECTORY);

    if (stat(aliasme_path, &st) == -1) {
        if (mkdir(aliasme_path, 0755))
            perror("cannot create aliasme directory");
    }
}

void create_command_directory(char* cmd) {
    struct stat st = {0};
    char cmd_path[1024] = {0};

    snprintf(cmd_path, 1024, "%s/%s/%s", getenv("HOME"), ALIASME_DIRECTORY,
             cmd);

    if (stat(cmd_path, &st) == -1) {
        mkdir(cmd_path, 0755);
    }
}

void create_main(char* cmd) {
    char main_path[1012] = {0};

    snprintf(main_path, 1024, "%s/%s/%s/%s", getenv("HOME"), ALIASME_DIRECTORY,
             cmd, MAIN);
    FILE* file = fopen(main_path, "w");
    fprintf(file, MAIN_TEMPLATE, cmd);
    fclose(file);

    char editor_cmd[1024] = {0};
    snprintf(editor_cmd, 1024, "$EDITOR %s", main_path);
    if (system(editor_cmd)) perror("cannot open file in editor");

    chmod(main_path, 0755);
}

void create_executable(char* cmd) {
    char exec_path[1024] = {0};

    snprintf(exec_path, 1024, "%s/%s/%s", getenv("HOME"), ALIASME_BIN, cmd);
    FILE* file = fopen(exec_path, "w");
    fprintf(file, EXEC_TEMPLATE, cmd);

    chmod(exec_path, 0755);
}

void add_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    char* cmd = argv[0];
    ensure_aliasme_directory_exists();
    create_command_directory(cmd);
    create_main(cmd);
    if (argc == 1) create_executable(cmd);
    printf("Successfully added %s\n", cmd);
}

void run_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    char exec_path[1024] = {0};
    char* cmd = argv[0];
    snprintf(exec_path, 1024, "%s/%s/%s/%s", getenv("HOME"), ALIASME_DIRECTORY,
             cmd, MAIN);
    if (system(exec_path)) perror("cannot run command");
}

int main(int argc, char* argv[]) {
    if (argc == 1) usage();

    if (!strcmp(argv[1], ADD)) {
        add_command(argc - 2, argv + 2);
        return 0;
    }
    if (!strcmp(argv[1], RUN)) {
        run_command(argc - 2, argv + 2);
        return 0;
    }
}
