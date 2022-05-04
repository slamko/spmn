#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "def.h"

result 
openp(const char *toolname, const char *patch_name) {
    int patchn_len = strnlen(patch_name, ENTRYLEN);
    int toolname_len = strnlen(toolname, ENTRYLEN);
    
    if (entrname_valid(patch_name, patchn_len))
        HANDLE_ERR("Invalid patch name: '%s'", patch_name)

    if (entrname_valid(toolname, toolname_len))
        HANDLE_ERR("Invalid tool name: '%s'", toolname)

    
}

int
parse_open_args(int argc, char **argv) {
    result res;
    
    if (argc != 4)
        return ERR_INVARG;
    
    res = openp(argv[2], argv[3]);
    return res;
}