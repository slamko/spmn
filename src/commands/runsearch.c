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


#include "def.h"
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "commands/runsearch.h"
#include "commands/search.h"
#include "utils/entry-utils.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"

static int getwords_count(char *searchstr, int searchlen) {
    int symbolscount = 0;
    bool prevcharisspace = true;

    for (int i = 0; i < searchlen; i++) {
        if (isspace(searchstr[i])) {
            prevcharisspace = true;
        } else {
            if (prevcharisspace)
                symbolscount++;

            prevcharisspace = false;
        }
    }
    return symbolscount;
}

static int parse_search_symbols(searchsyms *sargs, char **sstrings,
                                int scount) {
    char **words = NULL;
    char *token = NULL;
    size_t tstrcnt = 0;
    size_t lastalloc = 0, wid = 0;
    char delim[] = " ";

    ZIC_RESULT_INIT()

    sargs->words = NULL;
    sargs->wordcount = 0;

    for (int i = 0; i < scount; i++) {
        char *searchstr = sstrings[i];
        int sstrcnt = 0, sstrlen = 0;
        char *context = NULL, *parsedsstr = NULL;

        sstrlen = strnlen(searchstr, MAXSEARCH_LEN);
        UNWRAP(check_entrname_valid(searchstr, sstrlen));

        if (*searchstr == '-')
            continue;

        parsedsstr = strndup(searchstr, sstrlen);
        sstrcnt = getwords_count(searchstr, sstrlen);
        tstrcnt += sstrcnt;

        if (!words) {
            lastalloc = sstrcnt * scount;
            words = (char **)calloc(lastalloc, sizeof(*words));
        } else if (tstrcnt > lastalloc) {
            lastalloc = tstrcnt * 2;
            char **wrealloc = realloc(words, lastalloc * sizeof(*words));
            if (!wrealloc) {
                free(words);
                words = NULL;
                ERROR(ERR_SYS);
            }
            words = wrealloc;
        }

        if (!words) {
            DO_CLEAN_ALL();
		}
			
        for (token = strtok_r(parsedsstr, delim, &context);
             token && wid < tstrcnt; token = strtok_r(NULL, delim, &context)) {
            if (*token != '-') {
                if (*token == '\\') {
                    words[wid] = strndup(token + 1, sstrlen);
                } else {
                    words[wid] = strndup(token, sstrlen);
                }
                wid++;
            }
        }

        CLEANUP_ALL(context = NULL; free(parsedsstr));
    }

    sargs->words = words;
    sargs->wordcount = tstrcnt;
    ZIC_RETURN_RESULT();
}

int worth_multithread(int entrycount) {
    (void)entrycount;

#ifdef USE_MULTITHREADED
    return entrycount >=
           ((OPTWORK_AMOUNT * 2) - (OPTWORK_AMOUNT - (OPTWORK_AMOUNT / 4)));
#else
    return OK;
#endif
}

static void assign_thread_bounds(lookupthread_args *threadargpool,
                                 const int tid, const int thcount,
                                 const int entrycnt) {
    lookupthread_args *thargs = threadargpool + tid;

    if (tid == 0) {
        thargs->startpoint = 0;
        if (thcount < OPTTHREAD_COUNT) {
            thargs->endpoint = thargs->startpoint + OPTWORK_AMOUNT;
        } else {
            double actworkamount = (double)entrycnt / (double)OPTTHREAD_COUNT;
            int approxstartval = (int)(floor(actworkamount / 100.0)) + 2;
            thargs->endpoint = thargs->startpoint + approxstartval;
        }
    } else {
        lookupthread_args *prevthargs = threadargpool + (tid - 1);
        thargs->startpoint = prevthargs->endpoint;

        if (tid == thcount - 1) {
            thargs->endpoint = entrycnt;
        } else {
            thargs->endpoint = (prevthargs->endpoint - prevthargs->startpoint) +
                               thargs->startpoint;
        }
    }
}

static int setup_threadargs(lookupthread_args *threadargpool, const int tid,
                            const int thcount, const int entrycnt,
                            const int thoutfd, searchsyms *searchargs,
                            char *patchdir, pthread_mutex_t *fmutex) {
    lookupthread_args *thargs;
    int descffd;
    char descfname[] = DESCFILE;

    if (thcount < 1 || entrycnt < 1 || tid < 0 || tid >= thcount) {
        ERROR(ERR_LOCAL)
    }

    thargs = threadargpool + tid;
    descffd = mkstemp(descfname);
    UNWRAP_NEG(descffd)

    thargs->descfname = strndup(descfname, DESCFILE_LEN);
    UNWRAP_PTR(thargs->descfname)

    thargs->descffd = descffd;
    thargs->outfd = thoutfd;
    thargs->mutex = fmutex;
    thargs->patchdir = patchdir;
    thargs->searchargs = searchargs;

    if (thcount == 1) {
        thargs->startpoint = 0;
        thargs->endpoint = entrycnt;
        RET_OK()
    }

    assign_thread_bounds(threadargpool, tid, thcount, entrycnt);
    RET_OK()
}

static void cleanup_descfname(lookupthread_args *thargs) {
    free(thargs->descfname);
}

static void cleanup_searchargs(searchsyms *sargs) {
    for (size_t i = 0; i < sargs->wordcount; i++) {
        free(sargs->words[i]);
    }
    free(sargs->words);
}

int run_search(char *patchdir, searchsyms *searchargs) {
    DIR *pd = NULL;
    struct dirent *pdir = NULL;
    int tentrycnt = 0;

    ZIC_RESULT_INIT()

    UNWRAP_PTR(pd = opendir(patchdir))

    while ((pdir = readdir(pd))) {
        if (pdir->d_type == DT_DIR)
            tentrycnt++;
    }
    UNWRAP(closedir(pd))

    if (worth_multithread(tentrycnt)) {
        ZIC_RESULT = /*run_multithreaded(patchdir, searchargs, tentrycnt);*/ 0;
    } else {
        lookupthread_args thargs = {0};
        lookupthread_args *thargsp = NULL;

        UNWRAP(setup_threadargs(&thargs, 0, 1, tentrycnt, STDOUT_FILENO,
                                searchargs, patchdir, NULL))

        thargsp = &thargs;

        lookup_entries(thargsp);
        ZIC_RESULT = thargsp->result;
        cleanup_descfname(thargsp);
        cleanup_searchargs(thargsp->searchargs);
    }
    ZIC_RETURN_RESULT()
}

int parse_search_args(int argc, char **argv, const char *basecacherepo) {
    char *patchdir = NULL;
    searchsyms *searchargs = NULL;
    size_t startp = 2, toolname_argpos;
    int option;

    ZIC_RESULT_INIT();

    if (IS_OK(strncmp(argv[CMD_ARGPOS], SEARCH_CMD, CMD_LEN))) {
        startp++;
    }

    toolname_argpos = startp - TOOLNAME_ARGPOS + 1;

    if (toolname_argpos >= (size_t)argc) {
        ERROR(ERR_INVARG)
    }

    TRY(append_toolpath(&patchdir, basecacherepo, argv[toolname_argpos]),
        HANDLE_PRINT_ERR("Suckless tool with name: '%s' not found",
               argv[toolname_argpos]););

    searchargs = calloc(1, sizeof(*searchargs));
    TRY_PTR(searchargs, DO_CLEAN(cl_pdir));

    TRY(parse_search_symbols(searchargs, argv + startp, argc - startp),
        HANDLE_PRINT_ERR_DO_CLEAN_ALL("Invalid search string"));

    while ((option = getopt(argc, argv, "f")) != -1) {
        switch (option) {
        case 'f':
            searchargs->s_flags.print_full_patch = true;
            break;
        case '?':
            ERROR_DO_CLEAN(ERR_INVARG, DO_CLEAN_ALL());
            break;
        }
    }

    TRY(run_search(patchdir, searchargs),
        CATCH(ERR_SYS, HANDLE_SYS_DO_CLEAN_ALL()));

    CLEANUP_ALL(free(searchargs));
    CLEANUP(cl_pdir, free(patchdir))
    ZIC_RETURN_RESULT();
}
