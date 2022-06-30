/*
Copyright 2022 Viacheslav Chepelyk-Kozhin.

This file is part of Suckless Patch Manager (spmn).
Spmn is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
Spmn is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
spmn. If not, see <https://www.gnu.org/licenses/>.
*/


#ifndef LOGUTILS_H
#define LOGUTILS_H

#include <stdio.h>

void error(const char* err_format, ...);

void bug(const char *filename, int linenum, const char *bug_msg, ...);

void print_usage(void);

void print_version(void);

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
