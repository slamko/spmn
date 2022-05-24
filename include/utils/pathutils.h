#include <stdbool.h>
#include <dirent.h>
#include "def.h"

result check_isdir(const struct dirent *dir);

result spappend(char **bufp, const char *base, const char *append);

result bufpappend(char *buf, const char *append);

result bufnpappend(char *buf, const char *append, size_t nmax);

result append_patch_path(char **pbuf, const char *toolpath, const char *patchname);

result check_patch_exists(const char *toolpath, const char *patchname);

result append_tooldir(char **buf, const char *basecacherepo, const char *tooldir);

result search_tooldir(char **buf, const char *basecacherepo, const char *toolname);

result get_tool_path(char **patchdir, const char *basecacherepo, const char *toolname);

result append_toolpath(char **buf, const char *basecacherepo, const char *toolname);

result get_repocache(char **cachedirbuf);

bool check_baserepo_exists(const char *baserepocache);