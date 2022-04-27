#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deftypes.h"
#include "utils/logutils.h"

void 
error(const char* err_format, ...) {
    va_list args;
    int errlen;
    size_t errmsgsize;
    char *err = NULL;

    va_start(args, err_format);
    errlen = strlen(err_format);
    errmsgsize = ERR_PREFIX_LEN + errlen * sizeof(*err);
    err = malloc(errmsgsize);
    if (!err)
        DIE_F();

    memset(err, ASCNULL, errmsgsize);
    snprintf(err, errmsgsize,  ERR_PREFIX"%s", err_format);

    vfprintf(stderr, err, args);
    vfprintf(stderr, "\n", args);
    free(err);
    va_end(args);
}

void
print_usage(void) {
    printf("usage: sise <tool> <keywords>\n");
}

void 
error_nolocalrepo(void) {
    error("Unable to find base suckless repo. Try running 'sise sync'");
}

void fatalerr(const char *err) {
    fprintf(stderr, err);
    fprintf(stderr, "\n");
}

void perrfatal(void) {
    perror(FATALERR_PREFIX);
}

void fcache_error(void) {
    fatalerr("fatal: Cache access error");
}
