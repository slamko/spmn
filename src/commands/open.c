#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "def.h"

result 
openp(const char *toolname, const char *entryname) {
    int entrnlen = strnlen(entryname, ENTRYLEN);
    if (entrname_valid(entryname, entrnlen))
        return ERR_INVARG;

    
}

int
parse_open_args(int argc, char **argv) {
    result res;
    
    if (argc != 4)
        return ERR_INVARG;
    
    res = openp(argv[2], argv[3]);
    return res;
}