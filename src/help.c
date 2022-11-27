#include <stdio.h>
#include <stdlib.h>

#include "const.h"

const char* USAGE =
    "Usage:\n"
    "\taliasme <command> ...\n";

void usage() {
    printf("%s", USAGE);
    exit(0);
}
