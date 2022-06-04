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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "commands/download.h"
#include "commands/open.h"
#include "commands/runsearch.h"
#include "commands/sync.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"

typedef int (*commandp)(int, char **, const char *);

static const commandp commands[] = {&parse_sync_args, &parse_search_args,
                                    &parse_open_args, &parse_load_args};

enum command { SYNC = 0, SEARCH = 1, OPEN = 2, DOWNLOAD = 3 };

int local_repo_is_obsolete(struct tm *cttm, struct tm *lmttm) {
  return cttm->tm_mday - lmttm->tm_mday >= SYNC_INTERVAL_D ||
         (cttm->tm_mon > lmttm->tm_mon && cttm->tm_mday > SYNC_INTERVAL_D) ||
         (cttm->tm_year > lmttm->tm_year && cttm->tm_mday > SYNC_INTERVAL_D);
}

result try_sync_caches(const char *basecacherepo) {
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

result parse_command(const int argc, char **argv, enum command *commandarg) {
  if (argc <= 1)
    ERROR(ERR_INVARG);

  if (IS_OK(strncmp(argv[CMD_ARGPOS], SYNC_CMD, sizeof(SYNC_CMD) - 1))) {
    *commandarg = SYNC;
  } else if (IS_OK(strncmp(argv[CMD_ARGPOS], DOWNLOAD_CMD,
                           sizeof(DOWNLOAD_CMD) - 1))) {
    *commandarg = DOWNLOAD;
  } else if (IS_OK(strncmp(argv[CMD_ARGPOS], OPEN_CMD, sizeof(OPEN_CMD) - 1))) {
    *commandarg = OPEN;
  } else {
    *commandarg = SEARCH;
  }

  RET_OK();
}

int main(int argc, char **argv) {
  char *basecacherepo;
  enum command cmd;
  ZIC_RESULT_INIT()

  if (parse_command(argc, argv, &cmd)) {
    print_usage();
    return EXIT_FAILURE;
  }

  if (get_repocache(&basecacherepo)) {
    return EXIT_FAILURE;
  }

  if (cmd != SYNC) {
    if (!check_baserepo_exists(basecacherepo)) {
      error_nolocalrepo();
      return EXIT_FAILURE;
    }

    if (try_sync_caches(basecacherepo)) {
      error("Failed to autosync caches. Continuing without syncing...");
    }
  }

  TRY(commands[(int)cmd](argc, argv, basecacherepo), )

  free(basecacherepo);
}
