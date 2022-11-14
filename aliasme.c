#define _GNU_SOURCE

#define UNUSED(x) (void)(x)

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
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
const char* REMOVE = "rm";

const char* ALIASME_DIRECTORY = ".local/share/aliasme";
const char* ALIASME_BIN = "bin";
const char* MAIN = "_main";

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
            handle_error("cannot create aliasme directory");
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
    if (system(editor_cmd)) handle_error("cannot open file in editor");

    chmod(main_path, 0755);
}

void create_executable(char* cmd) {
    char exec_path[1024] = {0};

    snprintf(exec_path, 1024, "%s/%s/%s", getenv("HOME"), ALIASME_BIN, cmd);
    FILE* file = fopen(exec_path, "w");
    fprintf(file, EXEC_TEMPLATE, cmd);
    fclose(file);

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
    if (system(exec_path)) handle_error("cannot run command");
}

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

void remove_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    struct stat st = {0};
    char cmd_path[1024] = {0};

    char* cmd = argv[0];
    snprintf(cmd_path, 1024, "%s/%s/%s", getenv("HOME"), ALIASME_DIRECTORY,
             cmd);

    if (stat(cmd_path, &st) != -1) {
        remove_directory(cmd_path);
    }

    char exec_path[1024] = {0};
    snprintf(exec_path, 1024, "%s/%s/%s", getenv("HOME"), ALIASME_BIN, cmd);
    remove(exec_path);
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
    if (!strcmp(argv[1], REMOVE)) {
        remove_command(argc - 2, argv + 2);
        return 0;
    }
}
