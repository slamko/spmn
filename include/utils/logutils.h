#ifndef LOGUTILS_H
#define LOGUTILS_H

#include <stdio.h>

void error(const char* err_format, ...);

void bug(const char *bug_msg, ...);

void print_usage(void);

void error_nolocalrepo(void);

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
#endif
