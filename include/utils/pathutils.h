#include <stdbool.h>
#include <dirent.h>
#include "deftypes.h"

int check_isdir(const struct dirent *dir);

char *sappend(const char *base, const char *append);

int spappend(char **bufp, const char *base, const char *append);

char *bufappend(char *buf, const char *append);

result bufpappend(char *buf, const char *append);

result append_patch_path(char **pbuf, const char *toolpath, const char *patchname);

result check_patch_exists(const char *toolpath, const char *patchname);

result append_tooldir(char **buf, const char *basecacherepo, const char *tooldir);

result search_tooldir(char **buf, const char *basecacherepo, const char *toolname);

result get_tool_path(char **patchdir, const char *basecacherepo, const char *toolname);

result append_toolpath(char **buf, const char *basecacherepo, const char *toolname);

result get_repocache(char **cachedirbuf);

bool check_baserepo_exists(const char *baserepocache);