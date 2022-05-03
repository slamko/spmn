#ifndef LOGUTILS_H
#define LOGUTILS_H

#include <stdio.h>

void error(const char* err_format, ...);

void print_usage(void);

void error_nolocalrepo(void);

void fatalerr(const char *err);

void perrfatal(void);

void fcache_error(void);

#define DIE_M(CLEANUP) { \
    perror(FATALERR_PREFIX); \
    CLEANUP; \
    return EXIT_FAILURE; \
}

#define DIE_E(CLEANUP) { \
    perror(FATALERR_PREFIX); \
    CLEANUP; \
    exit(EXIT_FAILURE); \
}

#define DIE_F() { \
    perror(FATALERR_PREFIX); \
    exit(EXIT_FAILURE); \
}

#define DIE_C(ERR) { \
    ERR; \
    return EXIT_FAILURE; \
}

#define UNWRAP(EXP) { \
    int res = (EXP); \
    if (res) return res; }

typedef enum {
    OK = 0,
    FAIL = 1,
    ERR_LOCAL = 2,
    ERR_SYS = 3,
    ERR_INVARG = 4
} result;

#endif