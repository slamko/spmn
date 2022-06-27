/*
Copyright 2022 Viacheslav Chepelyk-Kozhin.

This file is part of Suckless Patch Manager (spm).
Spm is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
Spm is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
spm. If not, see <https://www.gnu.org/licenses/>.
*/

#include "def.h"
#include <bits/types.h>
#include <bsd/string.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "commands/apply.h"
#include "commands/download.h"
#include "commands/open.h"
#include "commands/runsearch.h"
#include "commands/sync.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"
#include "zic.h"

typedef int (*commandp)(int, char **, const char *);

#define CMD_CNT 7

result help(int argc, char **argv, const char *basecacherepo) {
    KINDA_USE_3ARG(argc, argv, basecacherepo);
    print_usage();
    RET_OK()
}

result version(int argc, char **argv, const char *basecacherepo) {
    KINDA_USE_3ARG(argc, argv, basecacherepo);
    print_version();
    RET_OK()
}

static const commandp commands[CMD_CNT] = {
    &parse_sync_args, &parse_search_args, &parse_open_args,
    &parse_load_args, &parse_apply_args,  &help,
    &version};

static const char *const command_names[CMD_CNT] = {
    "sync", SEARCH_CMD, "open", "load", "apply", "help", "version"};

enum command {
    SYNC = 0,
    SEARCH = 1,
    OPEN = 2,
    DOWNLOAD = 3,
    APPLY = 4,
    HELP = 5,
    VERSION = 6
};

static int local_repo_is_obsolete(struct tm *cttm, struct tm *lmttm) {
    return cttm->tm_mday - lmttm->tm_mday >= SYNC_INTERVAL_D ||
           (cttm->tm_mon > lmttm->tm_mon && cttm->tm_mday > SYNC_INTERVAL_D) ||
           (cttm->tm_year > lmttm->tm_year && cttm->tm_mday > SYNC_INTERVAL_D);
}

static result try_sync_caches(const char *basecacherepo) {
    struct stat cache_sb = {0};
    time_t lastmtime, curtime;
    struct tm *lmttm = NULL, *cttm = NULL;

    UNWRAP(stat(basecacherepo, &cache_sb))

    lastmtime = cache_sb.st_mtim.tv_sec;
    time(&curtime);
    cttm = gmtime(&curtime);
    lmttm = gmtime(&lastmtime);

    if (local_repo_is_obsolete(cttm, lmttm)) {
        return run_sync();
    }
    RET_OK();
}

static result parse_command(const int argc, char **argv,
                            enum command *commandarg) {
    int set_cmd = 0;

    if (argc <= 1)
        ERROR(ERR_INVARG);

    if (*argv[CMD_ARGPOS] == '-') {
        if (IS_OK(strcmp(argv[CMD_ARGPOS], "--help")) ||
            IS_OK(strcmp(argv[CMD_ARGPOS], "-h"))) {
            *commandarg = HELP;
            RET_OK();
        } else if (IS_OK(strcmp(argv[CMD_ARGPOS], "--version")) ||
                   IS_OK(strcmp(argv[CMD_ARGPOS], "-v"))) {
            *commandarg = VERSION;
            RET_OK();
        }
        ERROR(ERR_INVARG);
    }

    for (size_t cmdi = 0; cmdi < CMD_CNT; cmdi++) {
        if (IS_OK(strcmp(command_names[cmdi], argv[CMD_ARGPOS]))) {
            *commandarg = (enum command)cmdi;
            set_cmd = 1;
            break;
        }
    }

    if (!set_cmd)
        *commandarg = SEARCH;

    RET_OK();
}

int main(int argc, char **argv) {
    char *basecacherepo;
    enum command cmd;
    ZIC_RESULT_INIT();

    if (parse_command(argc, argv, &cmd)) {
        print_usage();
        FAIL();
    }

    if (get_repocache(&basecacherepo)) {
        FAIL();
    }

    if (cmd != SYNC) {
        if (!check_baserepo_valid(basecacherepo)) {
            PRINT_ERR("Could not find base suckless repo. Run '%s sync' to initialize mirror repository.", argv[0]);
            FAIL_DO_CLEAN_ALL();
        }

        if (try_sync_caches(basecacherepo)) {
            PRINT_ERR("Failed to autosync caches. Continuing without syncing...");
            FAIL_DO_CLEAN_ALL();
        }
    }

    TRY(commands[(int)cmd](argc, argv, basecacherepo),
        CATCH(ERR_INVARG, print_usage(); FAIL_DO_CLEAN_ALL()));

    CLEANUP_ALL(free(basecacherepo));
    ZIC_RETURN_RESULT();
}
