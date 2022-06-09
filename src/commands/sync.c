#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "def.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

static const char *const GIT_CMD        = "/bin/git";
static const char *const CLONE_CMD      = "clone";
static const char *const PULL_CMD       =  "pull";
static const char *const QUITE_ARG      =  "-q";
static const char *const CHANGE_DIR_OPT =  "-C";
static const char *const SUCKLESS_REPO  =  "git://git.suckless.org/sites";

static int 
git_pull(const char *base_cache_repo) {
    return execl(GIT_CMD, 
        GIT_CMD, 
        CHANGE_DIR_OPT, 
        base_cache_repo, 
        PULL_CMD, 
        QUITE_ARG,
        SUCKLESS_REPO, 
        (char *)NULL);
}

static int 
git_clone(const char *base_cache_repo) {
    return execl(GIT_CMD, 
        GIT_CMD, 
        CLONE_CMD, 
        SUCKLESS_REPO, 
        base_cache_repo,
        (char *)NULL);
}

result
run_sync(const char *basecacherepo, int *gitclone_st) {
    pid_t gitpid;
    ZIC_RESULT_INIT()

    UNWRAP_NEG (gitpid = fork())

    if (gitpid == 0) {
        TRY (check_baserepo_exists(basecacherepo), 
            UNWRAP_NEG (
                git_pull(basecacherepo)
            ) 
        )

        UNWRAP_NEG (git_clone(basecacherepo)) 
    } 

    UNWRAP_NEG (waitpid(gitpid, gitclone_st, 0))
    RET_OK()
}

int 
sync_repo(const char *basecacherepo) {
    ZIC_RESULT_INIT()
    int sync_stat;

    TRY (run_sync(basecacherepo, &sync_stat),
        HANDLE_SYS()
    )

    return !!sync_stat;
}

int
parse_sync_args(int argc, char **argv, const char *basecacherepo) {
    (void)argv;
    (void)argc;

    return sync_repo(basecacherepo);
}
