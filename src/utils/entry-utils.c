#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "utils/pathutils.h"
#include "utils/logutils.h"
#include "deftypes.h"

result
append_patchmd(char **buf, const char *patchdir, char *patch) {
    char *patchmd = NULL;

    UNWRAP (spappend(&patchmd, patch, INDEXMD))
    UNWRAP (spappend(buf, patchdir, patchmd))
    free(patchmd);

    return OK;
}

result
check_patch_exists(const char *toolpath, const char *patchname) {
    char *full_patch_path = NULL;
    struct stat st = {0};
    result res;

    UNWRAP (spappend(&full_patch_path, toolpath, patchname))

    puts(full_patch_path);
    res = stat(full_patch_path, &st);
    free(full_patch_path);
    return !!res;
}

result 
entrname_valid(const char *entryname, const int enamelen) {
    if (!entryname || 
        *entryname == '\0' || 
        enamelen == 0 || 
        enamelen > MAXSEARCH_LEN ||
        (enamelen == 1 && isspace(*entryname)))
        return FAIL;

    for (int i = 0; i < enamelen; i++) {
        if (!isspace(entryname[i]))
            return OK;
    }
    return FAIL;
}