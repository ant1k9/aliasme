#define _GNU_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "add.h"
#include "const.h"

const char* COMPLETION_TEMPLATE =
    "complete -f -c %s \\\n"
    "\t-n \"%s\" \\\n"
    "\t-a \"%s\" \\\n"
    "\t-d \"%s\"\n\n";

const char* COMPLETION_HELP_TEMPLATE =
    "complete -f -c %s \\\n"
    "\t-s h \\\n"
    "\t-l help\n\n";

void read_description(char* command_path, char* description) {
    char main_path[MAX_PATH_LENGTH];
    snprintf(main_path, 1024 - strlen(command_path), "%s/%s", command_path,
             MAIN);

    FILE* file = fopen(main_path, "r");

    char buf[MAX_DESCRIPTION_LENGTH * 2];
    fread(buf, sizeof(char), MAX_DESCRIPTION_LENGTH * 2, file);

    char* description_ptr;
    if ((description_ptr = strstr(buf, "# Description:")) != NULL) {
        description_ptr += 14;
        while (*description_ptr && *description_ptr == ' ') ++description_ptr;
        while (*description_ptr != '\n')
            sprintf(description + strlen(description), "%c",
                    *description_ptr++);
    }

    fclose(file);
}

void generate_completions_for_command(FILE* file, char* cmd, char* subcommand,
                                      char* command_path,
                                      char** root_cmd_list) {
    struct dirent* entry = NULL;

    if (!*root_cmd_list) {
        DIR* dp = NULL;
        fprintf(file, "set -l _subcommands \"");

        if ((dp = opendir(command_path)))
            while ((entry = readdir(dp))) {
                if (entry->d_type != DT_DIR) continue;
                char* next_cmd = entry->d_name;
                if (!strcmp(next_cmd, ".") || !strcmp(next_cmd, "..")) continue;
                fprintf(file, " %s", next_cmd);
            }

        fprintf(file, "\"\n\n");
        closedir(dp);
    }

    DIR* dp = NULL;
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
                calloc(strlen(next_cmd) + 1, sizeof(char));
            strcpy(root_cmd_list[len_cmd_list], next_cmd);

            generate_completions_for_command(file, cmd, next_cmd, command_path,
                                             root_cmd_list);

            // clean the state for the next iteration
            memset(command_path + command_path_len, '\0',
                   MAX_PATH_LENGTH - command_path_len);
            free(root_cmd_list[len_cmd_list]);
            root_cmd_list[len_cmd_list] = 0;
        }

    char condition[MAX_CONDITION_BUFFER] = {0};
    if (*root_cmd_list && *(root_cmd_list + 1)) {
        for (int i = 0; i < MAX_COMPLETION_DEPTH; i++) {
            if (i + 1 < MAX_COMPLETION_DEPTH && !root_cmd_list[i + 1]) break;
            char* concat = i ? "; and " : "";
            snprintf(condition + strlen(condition),
                     MAX_CONDITION_BUFFER - strlen(condition),
                     "%s__fish_seen_subcommand_from %s", concat,
                     root_cmd_list[i]);
        }
    }

    int n_args = 0;
    while (*(root_cmd_list + n_args)) ++n_args;
    snprintf(condition + strlen(condition),
             MAX_CONDITION_BUFFER - strlen(condition),
             "; and test (commandline | wc -w) -le %d", n_args + 1);

    if (!*root_cmd_list)
        fprintf(file, COMPLETION_HELP_TEMPLATE, cmd);
    else {
        char description[MAX_DESCRIPTION_LENGTH];
        memset(description, '\0', MAX_DESCRIPTION_LENGTH);
        read_description(command_path, description);
        fprintf(file, COMPLETION_TEMPLATE, cmd, condition, subcommand,
                description);
    }

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
    generate_completions_for_command(file, cmd, cmd, command_path,
                                     root_cmd_list);

    fclose(file);
}

void generate_completions() {
    struct dirent* entry = NULL;
    DIR* dp = NULL;
    char aliasme_dir[MAX_PATH_LENGTH] = {0};

    snprintf(aliasme_dir, MAX_PATH_LENGTH, "%s/%s", getenv("HOME"),
             ALIASME_DIRECTORY);
    if ((dp = opendir(aliasme_dir)))
        while ((entry = readdir(dp))) {
            if (entry->d_type != DT_DIR) continue;
            char* next_cmd = entry->d_name;
            if (!strcmp(next_cmd, ".") || !strcmp(next_cmd, "..")) continue;
            generate_fish_completion(next_cmd);
            create_executable(next_cmd);
        }

    closedir(dp);
}
