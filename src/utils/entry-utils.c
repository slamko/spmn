#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "utils/pathutils.h"
#include "utils/logutils.h"
#include "def.h"

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
    return stat(ppath, &st)
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
build_url(char **url, const char *toolpath, const char *patch_name, size_t patchn_len) {
    size_t toolpath_len, url_len;

    toolpath_len = strnlen(toolpath, LINEBUF);
    url_len = HTTPS_PLEN + toolpath_len + patchn_len;

    *url = calloc(url_len, sizeof(**url));
    UNWRAP_PTR(*url)
    UNWRAP_NEG(snprintf(*url, url_len, HTTPS_PREF "%s%s", toolpath, patch_name))
    
    RET_OK()
}

result 
build_patch_path(char **path, const char *toolname, const char *patch_name, size_t patchn_len, const char *basecacherepo) {
    size_t toolname_len;
    char *toolpath = NULL, *tooldir = NULL;

    ZIC_RESULT_INIT()

    toolname_len = strnlen(toolname, ENTRYLEN);

    TRY (check_entrname_valid(patch_name, patchn_len),
        HANDLE("Invalid patch name: '%s'", patch_name))

    TRY (check_entrname_valid(toolname, toolname_len),
        HANDLE("Invalid tool name: '%s'", toolname))

    TRY (get_tool_path(&toolpath, basecacherepo, toolname),
        CATCH(ERR_ENTRY_NOT_FOUND, 
            HANDLE("Suckless tool with name: '%s' not found", toolname))

        CATCH(ERR_SYS, ERROR(ERR_SYS))
    )

    UNWRAP (spappend(path, toolpath, patchname))

    TRY (check_patch_path_exists(*path),
        HANDLE_CLEANUP("A patch with name: '%s' not found", patch_name)
    )
    CLEANUP(free(tooldir))
}

result 
build_patch_url(char **url, const char *toolname, const char *patch_name, const char *basecacherepo) {
    size_t patchn_len;
    char *patch_path = NULL;
    ZIC_RESULT_INIT()

    patchn_len = strnlen(patch_name, ENTRYLEN)

    UNWRAP_CLEANUP (build_patch_path(&patch_path, toolname, patch_name, patchn_len, basecacherepo))

    TRY (build_url(url, patch_path, patch_name, patchn_len), 
        ERROR_CLEANUP(ERR_LOCAL)
    )

    CLEANUP(free(patch_path))
}