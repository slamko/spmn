#include "def.h"
#include "sys/sendfile.h"
#include "utils/entry-utils.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"
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
    ZIC_RESULT_INIT()

    if (argc < 4)
        ERROR(ERR_INVARG)

    TRY(applyp(argv[2], argv[3], basecacherepo), CATCH(ERR_SYS, HANDLE_SYS());

        CATCH(ERR_LOCAL, bug(strerror(errno)); FAIL()))
    ZIC_RETURN_RESULT()
}
