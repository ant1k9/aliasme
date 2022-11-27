#include <string.h>

#include "./src/add.h"
#include "./src/const.h"
#include "./src/edit.h"
#include "./src/help.h"
#include "./src/rm.h"
#include "./src/run.h"

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
