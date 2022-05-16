#ifndef DEF_TYPES
#define DEF_TYPES
#include "def.h"


#define HANDLE_ERR(ERR, ...) { error(ERR, ##__VA_ARGS__); return FAIL; }

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
    const int res = (EXP); \
    if (res < 0) return ERR; \
    else if (res) return res; }

#define P_UNWRAP(EXP) { \
    const void *res = (EXP); \
    if (!res) return ERR_SYS; }

typedef enum {
    OK = 0,
    FAIL = 1,
    ERR_LOCAL = 2,
    ERR_SYS = 3,
    ERR_INVARG = 4,
    ERR_USER = 5
} result;

#endif