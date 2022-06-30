/*
Copyright 2022 Viacheslav Chepelyk-Kozhin.

This file is part of Suckless Patch Manager (spmn).
Spmn is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
Spmn is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
spmn. If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdbool.h>
#include <dirent.h>
#include <stddef.h>
#include "def.h"

result check_isdir(const struct dirent *dir);

result spappend(char **bufp, const char *base, const char *append);

result snpappend(char **bufp, const char *base, const char *append, size_t base_len);

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

bool check_baserepo_valid(const char *baserepocache);
