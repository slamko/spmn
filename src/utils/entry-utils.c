#include "utils/entry-utils.h"
#include "def.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// static const size_t HTTPS_PLEN      = sizeof(HTTPS_PREF);

result append_patchmd(char **buf, const char *patchdir, char *patch) {
    char *patchmd = NULL;

    UNWRAP(spappend(&patchmd, patch, INDEXMD))
    UNWRAP(spappend(buf, patchdir, patchmd))
    free(patchmd);

    RET_OK();
}

result check_patch_path_exists(const char *ppath) {
    struct stat st = {0};
    return stat(ppath, &st);
}

result check_patch_exists(const char *toolpath, const char *patchname) {
    char *full_patch_path = NULL;
    ZIC_RESULT_INIT()

    UNWRAP(spappend(&full_patch_path, toolpath, patchname))

    TRY(check_patch_path_exists(full_patch_path), FAIL())

    free(full_patch_path);
    ZIC_RETURN_RESULT()
}

result check_entrname_valid(const char *entryname, const int enamelen) {
    if (!entryname || *entryname == '\0' || enamelen == 0 ||
        enamelen > MAXSEARCH_LEN || (enamelen == 1 && isspace(*entryname)))
        FAIL();

    for (int i = 0; i < enamelen; i++) {
        if (!isspace(entryname[i]))
            RET_OK();
    }
    FAIL();
}

result build_url(char **url, const char *patch_path) {
    return spappend(url, HTTPS_PREF, patch_path);
}

result build_patch_path(char **path, const char *toolname,
                        const char *patch_name, size_t patchn_len,
                        const char *basecacherepo) {
    size_t toolname_len;
    char *tool_path = NULL;

    ZIC_RESULT_INIT();

    toolname_len = strnlen(toolname, ENTRYLEN);

    TRY(check_entrname_valid(patch_name, patchn_len),
        HANDLE("Invalid patch name: '%s'", patch_name));

    TRY(check_entrname_valid(toolname, toolname_len),
        HANDLE("Invalid tool name: '%s'", toolname));

    TRY(get_tool_path(&tool_path, basecacherepo, toolname),
        CATCH(ERR_ENTRY_NOT_FOUND,
              HANDLE_DO_CLEAN_ALL("Suckless tool with name: '%s' not found",
                                  toolname));

        CATCH(ERR_SYS, ERROR_DO_CLEAN_ALL(ERR_SYS)));

    UNWRAP_DO_CLEAN_ALL(spappend(path, tool_path, patch_name));

    CLEANUP_ALL(free(tool_path));
    ZIC_RETURN_RESULT();
}

result append_patch_dir(char **full_pdir, char *ppath, const char *patch_name,
                        const char *basecacherepo) {
    ZIC_RESULT_INIT();

    UNWRAP(snpappend(full_pdir, basecacherepo, ppath, PATHBUF));

    TRY(check_patch_path_exists(*full_pdir),
        HANDLE("A patch with name: '%s' not found", patch_name));
    ZIC_RETURN_RESULT();
}

result check_patch_dir(char *ppath, const char *patch_name,
                       const char *basecacherepo) {
    char *full_pdir = NULL;
    ZIC_RESULT_INIT();

    TRY(append_patch_dir(&full_pdir, ppath, patch_name, basecacherepo),
        DO_CLEAN_ALL());

    CLEANUP_ALL(free(full_pdir));
    ZIC_RETURN_RESULT()
}

result build_patch_dir(char **pdir, const char *toolname,
                       const char *patch_name, size_t patchn_len,
                       const char *basecacherepo) {
    char *ppath = NULL;
    ZIC_RESULT_INIT();

    TRY(build_patch_path(&ppath, toolname, patch_name, patchn_len,
                         basecacherepo),
        DO_CLEAN_ALL());
    TRY(append_patch_dir(pdir, ppath, patch_name, basecacherepo),
        DO_CLEAN_ALL());
    CLEANUP_ALL(free(ppath));
    ZIC_RETURN_RESULT()
}

result build_patch_url(char **url, const char *toolname, const char *patch_name,
                       const char *basecacherepo) {
    size_t patchn_len;
    char *patch_path = NULL;
    ZIC_RESULT_INIT()

    patchn_len = strnlen(patch_name, ENTRYLEN);

    TRY(build_patch_path(&patch_path, toolname, patch_name, patchn_len,
                         basecacherepo),
        DO_CLEAN_ALL());
    TRY(check_patch_dir(patch_path, patch_name, basecacherepo), DO_CLEAN_ALL());

    TRY(build_url(url, patch_path), ERROR_DO_CLEAN(ERR_LOCAL, DO_CLEAN_ALL()));

    CLEANUP_ALL(free(patch_path));
    ZIC_RETURN_RESULT()
}

result parse_tool_and_patch_name(int argc, char **argv, char **toolname,
                                 char **patchname, size_t init_search_pos) {
    UNWRAP_PTR(toolname);
    UNWRAP_PTR(patchname);
    size_t covered_entries_cnt = 0;

    for (size_t eid = 0; eid < (size_t)argc; eid++) {
        if (*(argv[eid]) != '-') {
            if (covered_entries_cnt >= init_search_pos) {
                if (!*toolname) {
                    *toolname = strdup(argv[eid]);
                } else {
                    if (!*patchname) {
                        *patchname = strdup(argv[eid]);
                    }
                }
            }

            covered_entries_cnt++;
        }
    }
    RET_OK()
}
