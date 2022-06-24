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
#include "sys/sendfile.h"
#include "utils/entry-utils.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"
#include <bits/types/__FILE.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "commands/download.h"

static const char *const apply_cmd = "patch < "; 

result do_apply(const char *diff_file) {
	char *cmd = NULL;
	UNWRAP (spappend(&cmd, apply_cmd, diff_file));
	
	system(cmd);

	free(cmd);
	RET_OK();
}

static result applyp(const char *toolname, const char *patchname,
             const char *basecacherepo) {
	return loadp(toolname, patchname, basecacherepo, (struct load_args){.apply = true});
}

int parse_apply_args(int argc, char **argv, const char *basecacherepo) {
	int option;
	ZIC_RESULT_INIT()

    if (argc < 4)
        ERROR(ERR_INVARG);

	while ((option = getopt(argc, argv, ":f")) != -1) {
		switch (option) {
		case 'f':
			UNWRAP_ERR(do_apply(argv[3]), ERR_SYS);
			RET_OK();
		case ':':
			ERROR(ERR_INVARG);
		case '?':
			ERROR(ERR_INVARG);
		}
	}
			
    TRY(applyp(argv[2], argv[3], basecacherepo), CATCH(ERR_SYS, HANDLE_SYS());

        CATCH(ERR_LOCAL, bug(__FILE__, __LINE__, strerror(errno)); FAIL()))
    ZIC_RETURN_RESULT()
}
