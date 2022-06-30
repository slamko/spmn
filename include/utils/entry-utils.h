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


#include "def.h"
#include <stddef.h>

#define HTTPS_PREF "https://"

int append_patchmd(char **buf, const char *patchdir, char *patch);

int check_entrname_valid(const char *entryname, const int enamelen);

result build_patch_path(char **path, const char *toolname, 
                        const char *patch_name, size_t patchn_len, 
                        const char *basecacherepo);

result build_patch_dir(char **pdir, const char *toolname,
                        const char *patch_name, size_t patchn_len,
                        const char *basecacherepo);

result build_patch_url(char **url, const char *toolname, 
                        const char *patch_name, const char *basecacherepo);

result parse_tool_and_patch_name(int argc, char **argv, char **toolname, char **patchname, size_t init_search_pos);
