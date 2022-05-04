#ifndef LOGUTILS_H
#define LOGUTILS_H

#include <stdio.h>

void error(const char* err_format, ...);

void bug(const char *bug_msg, ...);

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

#define HANDLE_ERR(ERR, ...) \
    error(ERR, ##__VA_ARGS__); \
    return FAIL;

#define UNWRAP(EXP) { \
    int res = (EXP); \
    if (res < 0) return ERR_SYS; \
    else if (res) return res; }

#define UNWRAP_N(EXP) { \
    int res = (EXP); \
    if (res < 0) return ERR_SYS; }

#define UNWRAP_L(EXP) { \
    int res = (EXP); \
    if (res < 0) return ERR_LOCAL; \
    else if (res) return res; }

#define EUNWRAP(EXP, ERR) { \
    int res = (EXP); \
    if (res < 0) return ERR; \
    else if (res) return res; }

#define P_UNWRAP(EXP) { \
    void *res = (EXP); \
    if (!res) return ERR_SYS; }

extern void *_pres;

#define UNWRAP_P(EXP) (_pres = EXP) ? _pres : NULL; if (!_pres) return ERR_SYS;

typedef enum {
    OK = 0,
    FAIL = 1,
    ERR_LOCAL = 2,
    ERR_SYS = 3,
    ERR_INVARG = 4,
    ERR_USER = 5
} result;

#endif