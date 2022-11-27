#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "const.h"
#include "help.h"

void print_help(char* exec_path) {
    snprintf(exec_path + strlen(exec_path), MAX_PATH_LENGTH - strlen(exec_path),
             "/%s", MAIN);

    char header_buf[HEADER_SIZE] = {0};
    FILE* file = fopen(exec_path, "r");
    if (!file) handle_error("cannot check main content");

    fread(header_buf, sizeof(char), HEADER_SIZE, file);

    char* help_ptr;
    if ((help_ptr = strstr(header_buf, "# Help:")) != NULL) {
        help_ptr += 7;
        while (*help_ptr && *help_ptr == ' ') ++help_ptr;
        while (*help_ptr != '\n') printf("%c", *help_ptr++);
        puts("");
    }

    fclose(file);
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
        if (i)
            snprintf(exec_path + strlen(exec_path),
                     MAX_PATH_LENGTH - strlen(exec_path), "/%s", argv[i]);
        else
            snprintf(exec_path, MAX_PATH_LENGTH, "%s/%s/%s", getenv("HOME"),
                     ALIASME_DIRECTORY, argv[i]);
        if (get_args(exec_path)) break;
        if (i + 1 < argc &&
            (!strcmp(argv[i + 1], "-h") || !strcmp(argv[i + 1], "--help"))) {
            print_help(exec_path);
            return;
        }

        struct stat st = {0};
        if (stat(exec_path, &st) == -1) handle_error("command does not exist");
    }

    snprintf(exec_path + strlen(exec_path), MAX_PATH_LENGTH - strlen(exec_path),
             "/%s", MAIN);
    for (++i; i < argc; i++)
        snprintf(exec_path + strlen(exec_path),
                 MAX_PATH_LENGTH - strlen(exec_path), " %s", argv[i]);

    if (system(exec_path)) handle_error("cannot run command");
}
