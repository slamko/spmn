#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "deftypes.h"

result 
openp(const char *toolname, const char *patch_name) {
    int patchn_len = strnlen(patch_name, ENTRYLEN);
    int toolname_len = strnlen(toolname, ENTRYLEN);
    
    if (entrname_valid(patch_name, patchn_len)) {
        HANDLE_ERR("Invalid patch name: '%s'", patch_name)
    }
    if (entrname_valid(toolname, toolname_len)) {
        HANDLE_ERR("Invalid tool name: '%s'", toolname)
    }

    return OK;
}

int
parse_open_args(int argc, char **argv, const char *basecacherepo) {
    result res;
    
    if (argc != 4)
        return ERR_INVARG;
    
    res = openp(argv[2], argv[3]);
    return res;
}