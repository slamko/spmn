#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include "deftypes.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

#define GITCMD "/bin/git"
#define CLONEARGS "clone" 
#define SUCKLESSREPO "git://git.suckless.org/sites"

int
run_sync(void) {
    int gitpid;
    int gitclonest = 0;

    gitpid = fork();

    if (gitpid == -1) {
        return 1;
    }

    if (gitpid == 0) {
        if (execl(GITCMD, CLONEARGS, SUCKLESSREPO, basecacherepo, NULL) == -1) {
            error("Failed to clone git repo");
            exit(1);
        }
    } 

    wait(&gitclonest);
    return gitclonest;
}

int
parse_sync_args(int argc, char **argv) {
    return run_sync();
}