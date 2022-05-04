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
    size_t errmsg_size;
    char *err = NULL;

    va_start(args, err_format);
    errlen = strlen(err_format);
    errmsg_size = ERR_PREFIX_LEN + errlen;
    err = calloc(errmsg_size, sizeof(*err));
    if (!err)
        DIE_F();

    snprintf(err, errmsg_size,  ERR_PREFIX"%s", err_format);

    vfprintf(stderr, err, args);
    vfprintf(stderr, "\n", args);
    free(err);
    va_end(args);
}

void bug(const char *bug_msg, ...) {
    va_list args;
    int bugmsg_len;
    size_t bugmsg_size;
    char *err = NULL;

    va_start(args, bug_msg);
    bugmsg_len = strlen(bug_msg);
    bugmsg_size = BUG_PREFIX_LEN + bugmsg_len;
    err = calloc(bugmsg_size, sizeof(*err));
    if (!err)
        DIE_F();

    snprintf(err, bugmsg_size,  BUG_PREFIX, __FILE__, __LINE__, bug_msg);

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
