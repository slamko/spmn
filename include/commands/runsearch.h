#ifndef RUN_SEARCH_DEF
#define RUN_SEARCH_DEF

#define SEARCH_CMD "search"

#include "commands/search.h"

int run_search(char *patchdir, searchsyms *searchsyms);

int parse_search_args(int argc, char **argv, const char *basecacherepo);
#endif
