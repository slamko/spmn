#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "def.h"
#include "utils/entry-utils.h"
#include "utils/pathutils.h"
#include "sys/sendfile.h"

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
get_diff_files_cnt(const char *patch_p, size_t *fcnt) {
    DIR *pdir = NULL;
    struct dirent *pdirent = NULL;
    size_t diff_cnt = 0;

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
get_diff_file_list(char ***diff_table, size_t *diff_table_len, const char *patch_p) {
    DIR *pdir = NULL;
    struct dirent *pdirent = NULL;
    char *diff_f_name;
    size_t diff_total_cnt = 0;

    UNWRAP (get_diff_files_cnt(patch_p, &diff_total_cnt))
    *diff_table = calloc(diff_total_cnt, sizeof(**diff_table));
    UNWRAP_PTR (*diff_table)

    pdir = opendir(patch_p);
    UNWRAP_PTR (pdir)
    
    for (size_t diff_counter = 0;
	 IS_OK(diff_f_iter(pdir, &pdirent, &diff_f_name));
	 diff_counter++) {

      (*diff_table)[diff_counter] = strdup(pdirent->d_name);
    }

    *diff_table_len = diff_total_cnt;
    UNWRAP (closedir(pdir))
    RET_OK()
}

result 
loadp(const char *toolname, const  char *patchname, const char *basecacherepo) {
    char *ppath = NULL;
    char **diff_table = NULL;
    size_t patchn_len, diff_t_len = 0;
    ZIC_RESULT_INIT()

    patchn_len = strnlen(patchname, ENTRYLEN);
    build_patch_dir(&ppath, toolname, patchname, patchn_len, basecacherepo);

    get_diff_file_list(&diff_table, &diff_t_len, ppath);

    if (diff_t_len == 1) {
      int dest_diff, source_diff;
      char *sdiff_path;
      struct stat diff_st = {0};

      spappend(&sdiff_path, ppath, diff_table[0]);
      source_diff = open(sdiff_path, O_RDONLY);
      dest_diff = open(diff_table[0], O_CREAT | O_WRONLY);
      stat(sdiff_path, &diff_st);
      
      sendfile(dest_diff, source_diff, NULL, diff_st.st_size);
    }

    RET_OK()
}

int parse_load_args(int argc, char **argv, const char *basecacherepo) {
    (void)argv;
    (void)argc;
    return loadp(basecacherepo);
}
