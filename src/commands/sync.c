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
    int gitclone_st = 0;

    gitpid = fork();
    UNWRAP_N (gitpid)

    if (gitpid == 0) {
        if (!check_baserepo_exists()) {
            UNWRAP (execl(GITCMD, GITCMD, CLONE, SUCKLESSREPO, basecacherepo,
                        (char *)NULL)) 
        }

        UNWRAP (execl(GITCMD, GITCMD, CHANGEDIR, basecacherepo, PULL, QUITEARG,
                    SUCKLESSREPO, (char *)NULL))
    } 

    UNWRAP_N (wait(&gitclone_st))
    return gitclone_st;
}

int
parse_sync_args(int argc, char **argv) {
    return run_sync();
}