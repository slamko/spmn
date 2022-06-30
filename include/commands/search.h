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


#ifndef SEARCH_COMMAND_DEF
#define SEARCH_COMMAND_DEF

#include <pthread.h>
#include "def.h"
#include "stdbool.h"

struct search_flags {
	bool print_full_patch;
};

typedef struct searchargs {
    char **words; 
    size_t wordcount;
	struct search_flags s_flags;
} searchsyms;

struct threadargs {
    char *descfname;
    char *patchdir;
    int descffd;
    int outfd;
    int startpoint;
    int endpoint;
    result result;
    pthread_mutex_t *mutex;
    searchsyms *searchargs;
};

typedef struct threadargs lookupthread_args;

void *search_entry(void *thread_args);

result lookup_entries(const lookupthread_args *args);
#endif
