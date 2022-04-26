#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include "def.h"
#include "pathutils.h"

#define GITCMD "git clone git://git.suckless.org/sites "

int
run_sync() {
    int gitf;

    gitf = fork();

    if (gitf == -1) {
        error();
    }
}

int
parse_sync_args(int argc, char **argv) {
    return run_sync();
}