#include "def.h"

result 
loadp(const char *basecacherepo) {
    (void)basecacherepo;
    return OK;
}

int parse_load_args(int argc, char **argv, const char *basecacherepo) {
    (void)argv;
    (void)argc;
    return loadp(basecacherepo);
}