#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "utils/pathutils.h"
#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "def.h"

//static const size_t HTTPS_PLEN      = sizeof(HTTPS_PREF);

result
append_patchmd(char **buf, const char *patchdir, char *patch) {
    char *patchmd = NULL;

    UNWRAP (spappend(&patchmd, patch, INDEXMD))
    UNWRAP (spappend(buf, patchdir, patchmd))
    free(patchmd);

    RET_OK();
}

result
check_patch_path_exists(const char *ppath) {
    struct stat st = {0};
    return stat(ppath, &st);
}

result
check_patch_exists(const char *toolpath, const char *patchname) {
    char *full_patch_path = NULL;
    ZIC_RESULT_INIT()

    UNWRAP (spappend(&full_patch_path, toolpath, patchname))

    TRY (check_patch_path_exists(full_patch_path), 
        FAIL()
    )
    
    free(full_patch_path);
    ZIC_RETURN_RESULT()
}

result 
check_entrname_valid(const char *entryname, const int enamelen) {
    if (!entryname ||
        *entryname == '\0' ||
        enamelen == 0 ||
        enamelen > MAXSEARCH_LEN ||
        (enamelen == 1 && isspace(*entryname)))
        FAIL();

    for (int i = 0; i < enamelen; i++) {
        if (!isspace(entryname[i]))
            RET_OK();
    }
    FAIL();
}

result 
build_url(char **url, const char *patch_path) {
    return spappend(url, HTTPS_PREF,patch_path);
}

result 
build_patch_path(char **path, const char *toolname, const char *patch_name, size_t patchn_len, const char *basecacherepo) {
    size_t toolname_len;

    ZIC_RESULT_INIT()

    toolname_len = strnlen(toolname, ENTRYLEN);
    *path = calloc(toolname_len + patchn_len, sizeof(**path)); 
    UNWRAP_PTR (*path)

    TRY (check_entrname_valid(patch_name, patchn_len),
        HANDLE("Invalid patch name: '%s'", patch_name))

    TRY (check_entrname_valid(toolname, toolname_len),
        HANDLE("Invalid tool name: '%s'", toolname))

    TRY (get_tool_path(path, basecacherepo, toolname),
        CATCH(ERR_ENTRY_NOT_FOUND, 
            HANDLE("Suckless tool with name: '%s' not found", toolname))

        CATCH(ERR_SYS, ERROR(ERR_SYS))
    )

    UNWRAP (bufnpappend(*path, patch_name, patchn_len))
    ZIC_RETURN_RESULT()
}

result
append_patch_dir(char **full_pdir, char *ppath, const char *patch_name, const char *basecacherepo) {
    ZIC_RESULT_INIT()

    UNWRAP (snpappend(full_pdir, basecacherepo, ppath, PATHBUF))

    TRY (check_patch_path_exists(*full_pdir),
        HANDLE("A patch with name: '%s' not found", patch_name)
    )
    ZIC_RETURN_RESULT()
}

result
check_patch_dir(char *ppath, const char *patch_name, const char *basecacherepo) {
    char *full_pdir = NULL;
    ZIC_RESULT_INIT()

    RES_UNWRAP_CLEANUP (append_patch_dir(&full_pdir, ppath, patch_name, basecacherepo))

    CLEANUP(free(full_pdir))
}

result
build_patch_dir(char **pdir, const char *toolname, const char *patch_name, size_t patchn_len, const char *basecacherepo) {
    char *ppath = NULL;
    ZIC_RESULT_INIT()

    RES_UNWRAP (build_patch_path(&ppath, toolname, patch_name, patchn_len, basecacherepo))
    RES_UNWRAP_CLEANUP(append_patch_dir(pdir, ppath, patch_name, basecacherepo))
    CLEANUP(free(ppath))
}

result 
build_patch_url(char **url, const char *toolname, const char *patch_name, const char *basecacherepo) {
    size_t patchn_len;
    char *patch_path = NULL;
    ZIC_RESULT_INIT()

    patchn_len = strnlen(patch_name, ENTRYLEN);

    UNWRAP_CLEANUP (build_patch_path(&patch_path, toolname, patch_name, patchn_len, basecacherepo))
    UNWRAP_CLEANUP (check_patch_dir(patch_path, patch_name, basecacherepo))

    TRY (build_url(url, patch_path), 
        ERROR_CLEANUP(ERR_LOCAL)
    )

    CLEANUP(free(patch_path))
}
