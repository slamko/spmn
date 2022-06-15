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

DEFINE_ERROR(ERR_NO_DIFF_FILE, 15)
DEFINE_ERROR(ERR_LOAD_CANCELED, 16)
	
#define DIFF_FILE_EXT "diff"
#define ENTER_NUMBER_PROMPT "Enter a number"

result diff_f_iter(DIR *ddir, struct dirent **ddirent, char **diff_f) {
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

result get_diff_files_cnt(const char *patch_p, size_t *fcnt) {
    DIR *pdir = NULL;
    struct dirent *pdirent = NULL;
    size_t diff_cnt = 0;

    pdir = opendir(patch_p);
    UNWRAP_PTR(pdir)

    while (IS_OK(diff_f_iter(pdir, &pdirent, NULL))) {
        diff_cnt++;
    }

    closedir(pdir);
    *fcnt = diff_cnt;
    RET_OK()
}

result get_diff_file_list(char ***diff_table, size_t *diff_table_len,
                          const char *patch_p) {
    DIR *pdir = NULL;
    struct dirent *pdirent = NULL;
    char *diff_f_name;
    size_t diff_total_cnt = 0;

    UNWRAP(get_diff_files_cnt(patch_p, &diff_total_cnt))

    if (diff_total_cnt == 0) {
        ERROR(ERR_NO_DIFF_FILE)
    }

    *diff_table = calloc(diff_total_cnt, sizeof(**diff_table));
    UNWRAP_PTR(*diff_table)

    pdir = opendir(patch_p);
    UNWRAP_PTR(pdir)

    for (size_t diff_counter = 0;
         IS_OK(diff_f_iter(pdir, &pdirent, &diff_f_name)); diff_counter++) {

        (*diff_table)[diff_counter] = diff_f_name;
    }

    *diff_table_len = diff_total_cnt;
    UNWRAP(closedir(pdir))
    RET_OK()
}

result copy_diff_file(const char *diff_f, const char *patch_path) {
    char *sdiff_path = NULL, *copy_buf = NULL;
    int dest_diff, source_diff, read_buf_c;
    struct stat diff_st = {0};
    size_t ppath_len, tot_buf_len;
    ZIC_RESULT_INIT()

    ppath_len = strlen(patch_path);
    tot_buf_len = ppath_len + 1 + strnlen(diff_f, ENTRYLEN) + 1;
    sdiff_path = calloc(tot_buf_len, sizeof(*sdiff_path));

    snprintf(sdiff_path, tot_buf_len, "%s/%s", patch_path, diff_f);

    source_diff = open(sdiff_path, O_RDONLY);
    TRY_UNWRAP_NEG(source_diff, DO_CLEAN(cl_diff_path));
    dest_diff = open(diff_f, O_CREAT | O_WRONLY, 0640);
    TRY_UNWRAP_NEG(dest_diff, DO_CLEAN(cl_source_fd));

    TRY_UNWRAP_NEG(stat(sdiff_path, &diff_st), DO_CLEAN(cl_dest_fd));

    TRY_PTR(copy_buf = calloc(diff_st.st_size, sizeof(*copy_buf)),
            DO_CLEAN(cl_dest_fd));

    read_buf_c = read(source_diff, copy_buf, diff_st.st_size);
    TRY_UNWRAP_NEG(read_buf_c, DO_CLEAN_ALL());
    TRY_UNWRAP_NEG(write(dest_diff, copy_buf, read_buf_c), DO_CLEAN_ALL());

    CLEANUP_ALL(free(copy_buf));
    CLEANUP(cl_dest_fd, close(dest_diff));
    CLEANUP(cl_source_fd, close(source_diff));
    CLEANUP(cl_diff_path, free(sdiff_path));
    ZIC_RETURN_RESULT()
}

result read_prompt_diff_file(size_t *input_val, size_t diff_t_len) {
    char read_buf[ENTRYLEN] = {0};
    size_t tmp_input = 0;

    putc('\n', stdout);

    for (char *prompt_msg = ENTER_NUMBER_PROMPT; true;) {
        printf("\r%s: ", prompt_msg);
        UNWRAP_PTR(fgets(read_buf, ENTRYLEN - 1, stdin))

        if (sscanf(read_buf, "%zu", &tmp_input) != EOF) {
            if (tmp_input > diff_t_len) {
                prompt_msg = "Please, enter a number from the listed range";
                continue;
            }
			if (tmp_input == 0) {
				input_val = NULL;
				ERROR(ERR_LOAD_CANCELED)
			}
            break;
        }
        prompt_msg = ENTER_NUMBER_PROMPT;
    }
    *input_val = tmp_input;
    RET_OK()
}

result prompt_diff_file(char **diff_table, char **chosen_diff,
                        const char *patch_name, size_t diff_f_cnt) {
    size_t usr_input = 0;

    printf(
        "Multiple diff files(%zu) found for patch '%s'. Please, choose one:\n",
        diff_f_cnt, patch_name);

    for (size_t diff_i = 0; diff_i < diff_f_cnt; diff_i++) {
        printf("(%zu) %s\n", diff_i + 1, diff_table[diff_i]);
    }

	puts("\n(0) Cancel");

    UNWRAP(read_prompt_diff_file(&usr_input, diff_f_cnt))
    *chosen_diff = diff_table[usr_input - 1];
    RET_OK()
}

static void free_diff_f_table(char **diff_table, size_t diff_t_len) {
    for (size_t diff_f_i = 0; diff_f_i < diff_t_len; diff_f_i++) {
        free(diff_table[diff_f_i]);
    }
	free(diff_table);
}

static result prompt_for_file_and_load(char **diff_table, char *ppath,
                                       const char *patch_name,
                                       size_t diff_t_len) {
    char *chosen_diff_f = NULL;

    UNWRAP(prompt_diff_file(diff_table, &chosen_diff_f, patch_name, diff_t_len))
    UNWRAP(copy_diff_file(chosen_diff_f, ppath))

    RET_OK()
}

result loadp(const char *toolname, const char *patchname,
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

int parse_load_args(int argc, char **argv, const char *basecacherepo) {
    ZIC_RESULT_INIT()

    if (argc < 4)
        ERROR(ERR_INVARG)

    TRY(loadp(argv[2], argv[3], basecacherepo), CATCH(ERR_SYS, HANDLE_SYS());

        CATCH(ERR_LOCAL, bug(strerror(errno)); FAIL()))
    ZIC_RETURN_RESULT()
}
