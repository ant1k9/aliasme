#include <stdio.h>

void generate_completions_for_command(FILE* file, char* cmd, char* subcommand,
                                      char* command_path, char** root_cmd_list);
void generate_fish_completion(char* cmd);
