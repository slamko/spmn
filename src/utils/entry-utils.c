#include <ctype.h>
#include "def.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

int
append_patchmd(char **buf, const char *patchdir, char *patch) {
    char *patchmd = NULL;

    UNWRAP (spappend(&patchmd, patch, INDEXMD))
    UNWRAP (spappend(buf, patchdir, patchmd))
    free(patchmd);

    return OK;
}

int 
entrname_valid(char *entryname, int enamelen) {
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