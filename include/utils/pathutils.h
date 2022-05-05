#include <stdbool.h>
#include <dirent.h>
#include "utils/logutils.h"

int check_isdir(const struct dirent *dir);

char *sappend(const char *base, const char *append);

int spappend(char **bufp, const char *base, const char *append);

char *bufappend(char *buf, const char *append);

result append_tooldir(char **buf, const char *tooldir);

result search_tooldir(char **buf, const char *toolname);

result get_patchdir(char **patchdir, const char *toolname);

result append_patchdir(char **buf, const char *toolname);

bool check_baserepo_exists(void);