#define _XOPEN_SOURCE 500
#include "def.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"
#include <dirent.h>
#include <ftw.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *const GIT_CMD = "/bin/git";
static const char *const CLONE_CMD = "clone";
static const char *const PULL_CMD = "pull";
static const char *const QUITE_ARG = "-q";
static const char *const CHANGE_DIR_OPT = "-C";
static const char *const SUCKLESS_REPO = "git://git.suckless.org/sites";

static int git_pull(const char *base_cache_repo) {
    return execl(GIT_CMD, GIT_CMD, CHANGE_DIR_OPT, base_cache_repo, PULL_CMD,
                 QUITE_ARG, SUCKLESS_REPO, (char *)NULL);
}

static int git_clone(const char *base_cache_repo) {
    return execl(GIT_CMD, GIT_CMD, CLONE_CMD, QUITE_ARG, SUCKLESS_REPO,
                 base_cache_repo, (char *)NULL);
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag,
              struct FTW *ftwbuf) {
    KINDA_USE_3ARG(sb, typeflag, ftwbuf);
    if (remove(fpath))
        perror(fpath);

    return OK;
}

int rm_repo(const char *path) {
    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

result run_sync(const char *basecacherepo, int *gitclone_st) {
    pid_t gitpid;

    puts("Synchronizing git repositories...");
    UNWRAP_NEG(gitpid = fork());

    if (gitpid == 0) {
        if (check_baserepo_exists(basecacherepo)) {
            if (check_baserepo_valid(basecacherepo)) {
                if (git_pull(basecacherepo)) {
                    perror(ERROR_PREFIX);
                    exit(FAIL);
                }
            }
            rm_repo(basecacherepo);
        }

        if (git_clone(basecacherepo)) {
            perror(ERROR_PREFIX);
            exit(FAIL);
        }
    }

    UNWRAP_NEG(waitpid(gitpid, gitclone_st, 0));

    puts("Done.");
    RET_OK();
}

int sync_repo(const char *basecacherepo) {
    ZIC_RESULT_INIT();
    int sync_stat;

    TRY(run_sync(basecacherepo, &sync_stat), HANDLE_SYS(););

    return !!sync_stat;
}

int parse_sync_args(int argc, char **argv, const char *basecacherepo) {
    (void)argv;
    (void)argc;

    return sync_repo(basecacherepo);
}
