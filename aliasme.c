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
#include <stdbool.h>
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
    "aliasme run %s $*";

const char* MAIN_TEMPLATE =
    "#!/usr/bin/env bash\n"
    "# Description: \n"
    "# Help: \n"
    "# Args: false\n"
    "\n"
    "echo %s";

const char* COMPLETION_TEMPLATE =
    "complete -f -c %s \\\n"
    "\t-n \"%s\" \\\n"
    "\t-a \"%s\" \\\n"
    "\t-d \"%s\"\n\n";

const char* ALIASME_ROOT_COMPLETION =
    "complete -f -c aliasme \\\n"
    "\t-n \"not __fish_seen_subcommand_from add edit rm run\" \\\n"
    "\t-a \"add edit rm run\" \\\n";

const char* ALIASME_ADD_COMPLETION_TEMPLATE =
    "complete -f -c aliasme \\\n"
    "\t-n \"__fish_seen_subcommand_from add\" \\\n"
    "\t-a \"(/bin/ls $HOME/.local/share/aliasme)\" \\\n"
    "\t-d \"add new command\"\n\n";

const char* ADD = "add";
const char* EDIT = "edit";
const char* REMOVE = "rm";
const char* RUN = "run";

const char* ALIASME_DIRECTORY = ".local/share/aliasme";
const char* ALIASME_BIN = "bin";
const char* MAIN = "_main";
const char* FISH_COMPLETION_DIRECTORY = ".config/fish/completions";

#define MAX_PATH_LENGTH 1024
#define MAX_ARGS_BUFFER 1024
#define MAX_CONDITION_BUFFER 1024
#define MAX_COMPLETION_DEPTH 64
#define HEADER_SIZE 256

void usage() {
    printf("%s", USAGE);
    exit(0);
}

void ensure_aliasme_directory_exists() {
    struct stat st = {0};
    char aliasme_path[MAX_PATH_LENGTH] = {0};

    snprintf(aliasme_path, MAX_PATH_LENGTH, "%s/%s", getenv("HOME"),
             ALIASME_DIRECTORY);

    if (stat(aliasme_path, &st) == -1) {
        if (mkdir(aliasme_path, 0755))
            handle_error("cannot create aliasme directory");
    }
}

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

void generate_completions_for_command(FILE* file, char* cmd, char* command_path,
                                      char** root_cmd_list) {
    struct dirent* entry = 0;
    DIR* dp = 0;

    char* subcommand_list[MAX_COMPLETION_DEPTH] = {0};
    int len_subcommand_list = 0;

    if ((dp = opendir(command_path)))
        while ((entry = readdir(dp))) {
            if (entry->d_type != DT_DIR) continue;

            char* next_cmd = entry->d_name;
            if (!strcmp(next_cmd, ".") || !strcmp(next_cmd, "..")) continue;

            int command_path_len = strlen(command_path);
            snprintf(command_path + command_path_len,
                     MAX_PATH_LENGTH - command_path_len, "/%s", next_cmd);

            int len_cmd_list = 0;
            while (root_cmd_list[len_cmd_list]) ++len_cmd_list;
            root_cmd_list[len_cmd_list] =
                calloc(sizeof(char), strlen(next_cmd) + 1);
            strcpy(root_cmd_list[len_cmd_list], next_cmd);

            subcommand_list[len_subcommand_list] =
                calloc(sizeof(char), strlen(next_cmd) + 1);
            strcpy(subcommand_list[len_subcommand_list], next_cmd);
            ++len_subcommand_list;

            generate_completions_for_command(file, next_cmd, command_path,
                                             root_cmd_list);

            // clean the state for the next iteration
            memset(command_path + command_path_len, '\0',
                   MAX_PATH_LENGTH - command_path_len);
            free(root_cmd_list[len_cmd_list]);
            root_cmd_list[len_cmd_list] = 0;
        }

    char condition[MAX_CONDITION_BUFFER] = {0};
    char args[MAX_ARGS_BUFFER] = {0};

    for (int i = 0; i < MAX_COMPLETION_DEPTH; i++) {
        if (root_cmd_list[i] == 0) break;
        char* concat = i ? "; and " : "";
        snprintf(condition + strlen(condition),
                 MAX_CONDITION_BUFFER - strlen(condition),
                 "%s__fish_seen_subcommand_from %s", concat, root_cmd_list[i]);
    }

    for (int i = 0; i < len_subcommand_list; i++) {
        char* concat = i ? " " : "";
        snprintf(args + strlen(args), MAX_ARGS_BUFFER - strlen(args), "%s%s",
                 concat, subcommand_list[i]);
        free(subcommand_list[i]);
    }

    fprintf(file, COMPLETION_TEMPLATE, cmd, condition, args, "");

    closedir(dp);
}

void generate_fish_completion(char* cmd) {
    char completion_path[MAX_PATH_LENGTH] = {0};

    snprintf(completion_path, MAX_PATH_LENGTH, "%s/%s/%s.fish", getenv("HOME"),
             FISH_COMPLETION_DIRECTORY, cmd);

    FILE* file = fopen(completion_path, "w");

    char command_path[MAX_PATH_LENGTH] = {0};
    snprintf(command_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
             ALIASME_DIRECTORY, cmd);

    char* root_cmd_list[MAX_COMPLETION_DEPTH] = {0};
    generate_completions_for_command(file, cmd, command_path, root_cmd_list);

    fclose(file);
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

void edit_main(char* cmd) {
    char editor_cmd[MAX_PATH_LENGTH] = {0};

    snprintf(editor_cmd, MAX_PATH_LENGTH, "$EDITOR %s/%s", cmd, MAIN);

    if (system(editor_cmd)) handle_error("cannot open file in editor");
}

void edit_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    ensure_aliasme_directory_exists();

    char cmd_path[MAX_PATH_LENGTH] = {0};
    for (int i = 0; i < argc; i++) {
        if (i) {
            snprintf(cmd_path + strlen(cmd_path),
                     MAX_PATH_LENGTH - strlen(cmd_path), "/%s", argv[i]);
        } else {
            snprintf(cmd_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                     ALIASME_DIRECTORY, argv[i]);
        }

        struct stat st = {0};
        if (stat(cmd_path, &st) == -1) {
            handle_error("command does not exist");
        }
    }

    edit_main(cmd_path);
}

bool get_args(char* path) {
    char main_path[MAX_PATH_LENGTH] = {0};
    strcpy(main_path, path);

    snprintf(main_path + strlen(main_path), MAX_PATH_LENGTH - strlen(main_path),
             "/%s", MAIN);

    char header_buf[HEADER_SIZE] = {0};
    FILE* file = fopen(main_path, "r");
    if (!file) handle_error("cannot check main content");

    fread(header_buf, sizeof(char), HEADER_SIZE, file);
    bool has_args = strstr(header_buf, "# Args: true") != NULL;

    fclose(file);

    return has_args;
}

void run_command(int argc, char* argv[]) {
    if (argc == 0) usage();

    char exec_path[MAX_PATH_LENGTH] = {0};
    int i = 0;
    for (; i < argc; i++) {
        if (i) {
            snprintf(exec_path + strlen(exec_path),
                     MAX_PATH_LENGTH - strlen(exec_path), "/%s", argv[i]);
        } else {
            snprintf(exec_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                     ALIASME_DIRECTORY, argv[i]);
        }
        if (get_args(exec_path)) break;

        struct stat st = {0};
        if (stat(exec_path, &st) == -1) {
            handle_error("command does not exist");
        }
    }

    snprintf(exec_path + strlen(exec_path), MAX_PATH_LENGTH - strlen(exec_path),
             "/%s", MAIN);
    for (++i; i < argc; i++)
        snprintf(exec_path + strlen(exec_path),
                 MAX_PATH_LENGTH - strlen(exec_path), " %s", argv[i]);

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

void remove_fish_completion(char* cmd) {
    struct stat st = {0};
    char completion_path[MAX_PATH_LENGTH] = {0};

    snprintf(completion_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
             FISH_COMPLETION_DIRECTORY, cmd);

    if (stat(completion_path, &st) != -1) {
        remove(completion_path);
    }
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

    if (stat(cmd_path, &st) != -1) {
        remove_directory(cmd_path);
    }

    if (argc == 1) {
        char exec_path[MAX_PATH_LENGTH] = {0};
        snprintf(exec_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                 ALIASME_BIN, argv[1]);
        remove(exec_path);
        remove_fish_completion(argv[0]);
    } else {
        generate_fish_completion(argv[0]);
    }
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
    if (!strcmp(argv[1], EDIT)) {
        edit_command(argc - 2, argv + 2);
        return 0;
    }

    usage();
}
