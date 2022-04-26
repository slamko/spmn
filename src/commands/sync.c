#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "deftypes.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

#define GITCMD "/bin/git"
#define CLONE "clone" 
#define PULL "pull"
#define QUITEARG "-q"
#define CHANGEDIR "-C"
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
        if (!check_baserepo_exists()) {
            if (execl(GITCMD, GITCMD, CLONE, SUCKLESSREPO, basecacherepo, (char *)NULL) == -1) {
                error("Failed to clone git repo");
                exit(1);
            }
        }

        if (execl(GITCMD, GITCMD, CHANGEDIR, basecacherepo, 
                    PULL, QUITEARG, SUCKLESSREPO, (char *)NULL) == -1) {
            error("Failed to sync git repositories");
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