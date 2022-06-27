#include <stdbool.h>
#include "zic.h"

struct load_args {
	bool apply;
};

result loadp(const char *toolname, const char *patchname,
             const char *basecacherepo, struct load_args flags);

int parse_load_args(int argc, char **argv, const char *basecacherepo);
