#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include "def.h"
#include "utils/entry-utils.h"

#define DIFF_FILE_EXT "diff"

result
diff_f_iter(DIR *ddir, struct dirent **ddirent, char **diff_f) {
    while ((*ddirent = readdir(ddir))) {
        char *diff_ext;
        diff_ext = strrchr((*ddirent)->d_name, '.');

        if (IS_OK(strcmp(diff_ext + 1, DIFF_FILE_EXT))) {
            if (diff_f)
                *diff_f = strdup((*ddirent)->d_name);

            RET_OK()
        }
    }

    FAIL()
}

result
get_diff_files_cnt(const char *patch_p, int *fcnt) {
    DIR *pdir = NULL;
    struct dirent *pdirent = NULL;
    int diff_cnt = 0;

    pdir = opendir(patch_p);
    UNWRAP_PTR (pdir)

    while (IS_OK(diff_f_iter(pdir, &pdirent, NULL))) {
        diff_cnt++;
    }

    closedir(pdir);
    *fcnt = diff_cnt;
    RET_OK()
}

result
get_diff_file_list(char ***diff_table, const char *patch_p) {
    DIR *pdir = NULL;
    struct dirent *pdirent = NULL;
    int diff_f_cnt = 0;

    UNWRAP (get_diff_files_cnt(patch_p, &diff_f_cnt))
    *diff_table = calloc(diff_f_cnt, sizeof(**diff_table));
    UNWRAP_PTR (*diff_table)



}

result 
loadp(const char *toolname, const  char *patchname, const char *basecacherepo) {
    char *ppath = NULL;
    size_t patchn_len;
    ZIC_RESULT_INIT()

    patchn_len = strnlen(patchname, ENTRYLEN);
    build_patch_dir(&ppath, toolname, patchname, patchn_len, basecacherepo)


}

int parse_load_args(int argc, char **argv, const char *basecacherepo) {
    (void)argv;
    (void)argc;
    return loadp(basecacherepo);
}
