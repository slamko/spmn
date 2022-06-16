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

result applyp(const char *toolname, const char *patchname,
             const char *basecacherepo) {
    char *ppath = NULL;
    char **diff_table = NULL;
    size_t patchn_len, diff_t_len = 0;
    ZIC_RESULT_INIT()

    patchn_len = strnlen(patchname, ENTRYLEN);
    TRY(
        build_patch_dir(&ppath, toolname, patchname, patchn_len, basecacherepo), DO_CLEAN(cl_ppath))

    TRY(get_diff_file_list(&diff_table, &diff_t_len, ppath),
        CATCH(ERR_NO_DIFF_FILE,
              HANDLE("No diff files found for patch '%s'", patchname));
        DO_CLEAN_ALL());

    if (diff_t_len == 1) {
        TRY(copy_diff_file(diff_table[0], ppath), DO_CLEAN_ALL())
    } else {
        TRY(prompt_for_file_and_load(diff_table, ppath, patchname, diff_t_len),
			CATCH(ERR_LOAD_CANCELED,
				  HANDLE_DO_CLEAN_ALL("Canceled")
				)
			DO_CLEAN_ALL())
    }

    CLEANUP_ALL(free_diff_f_table(diff_table, diff_t_len));
    CLEANUP(cl_ppath, free(ppath));
    ZIC_RETURN_RESULT()
}

int parse_apply_args(int argc, char **argv, const char *basecacherepo) {
    ZIC_RESULT_INIT()

    if (argc < 4)
        ERROR(ERR_INVARG)

    TRY(applyp(argv[2], argv[3], basecacherepo), CATCH(ERR_SYS, HANDLE_SYS());

        CATCH(ERR_LOCAL, bug(strerror(errno)); FAIL()))
    ZIC_RETURN_RESULT()
}
