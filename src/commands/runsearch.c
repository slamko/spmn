#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <math.h>
#include <errno.h>
#include <stdarg.h> 
#include <ctype.h>
#include <pwd.h>
#include "commands/search.h"
#include "deftypes.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

int searchstr_invalid(char *searchstr, int sstrlen) {
    if (!searchstr || 
        *searchstr == '\0' || 
        sstrlen == 0 || 
        sstrlen > MAXSEARCH_LEN ||
        (sstrlen == 1 && isspace(*searchstr)))
        return 1;

    for (int i = 0; i < sstrlen; i++) {
        if (!isspace(searchstr[i]))
            return 0;
    }
    return 1;
}

int 
getwords_count(char *searchstr, int searchlen) {
    int symbolscount = !!searchlen;
    bool prevcharisspace = true;

    for (int i = 0; i < searchlen; i++) {
        if (isspace(searchstr[i])) {
            if (!prevcharisspace)
                symbolscount++;
        } else {
            prevcharisspace = false;
        }
    }
    return symbolscount;
}

int
parse_search_symbols(searchsyms *sargs, char **sstrings, int scount){
    char **words = NULL;
    char *pubsearchstr = NULL, *parsesearchstr = NULL, *token = NULL;
    int tstrcnt = 0;
    size_t lastalloc = 0;
    char delim[] = " ";
    char *context = NULL;

    sargs->words = NULL;
    sargs->searchstr = NULL;
    sargs->wordcount = 0;

    for (char *searchstr = *sstrings; (uintptr_t)searchstr - (uintptr_t)(*sstrings) < (uintptr_t)scount; searchstr++) {
        int sstrcnt, sstrlen;

        sstrlen = strnlen(searchstr, MAXSEARCH_LEN);
        if (searchstr_invalid(searchstr, sstrlen)) {
            return 1;
        }

        pubsearchstr = strndup(searchstr, sstrlen);
        parsesearchstr = strndup(searchstr, sstrlen);
        sstrcnt = getwords_count(searchstr, sstrlen);
        tstrcnt += sstrcnt;

        if (!words) {
            lastalloc = sstrcnt * scount;
            words = (char **)calloc(lastalloc, sizeof(*token));
        } else if ((size_t)tstrcnt > lastalloc) {
            words = realloc(words, tstrcnt * 2);
        }

        if (!words) {
            return 1;
        }

        token = strtok_r(parsesearchstr, delim, &context);

        for (int i = 0; token && i < sstrcnt; i++) {
            words[i] = strndup(token, sstrlen);
            token = strtok_r(NULL, delim, &context);
        }
    }

    sargs->words = words;
    sargs->searchstr = pubsearchstr;
    sargs->wordcount = tstrcnt;
    return 0;
}

int
worth_multithread(int entrycount) {
    printf("\nEntrycount: %d\n", entrycount);

    #ifdef USE_MULTITHREADED
    return entrycount >= ((OPTWORK_AMOUNT * 2) - (OPTWORK_AMOUNT - (OPTWORK_AMOUNT / 4)));
    #else
    return 0;
    #endif
}

int
calc_threadcount(int entrycnt) {
    int optentrycnt = OPTWORK_AMOUNT * OPTTHREAD_COUNT;
    if (entrycnt > optentrycnt) {
        return OPTTHREAD_COUNT;
    } else {
        float actualthrcount = entrycnt / OPTWORK_AMOUNT;
        int nearest_thrcount = floor((double)actualthrcount);
        int workdiff = (actualthrcount * 100) - (nearest_thrcount * 100);
        
        if (workdiff < MIN_WORKAMOUNT)
            return nearest_thrcount;

        return nearest_thrcount + 1;
    }
}

void 
assign_thread_bounds(lookupthread_args *threadargpool, const int tid, 
                    const int thcount, const int entrycnt) {
    lookupthread_args *thargs = threadargpool + tid;

    if (tid == 0) {
        thargs->startpoint = 0;
        if (thcount < OPTTHREAD_COUNT) {
            thargs->endpoint = thargs->startpoint + OPTWORK_AMOUNT;
        } else {
            double actworkamount = entrycnt / OPTTHREAD_COUNT;
            int approxstartval = (int)(floor(actworkamount / 100.0)) + 2;
            thargs->endpoint = thargs->startpoint + approxstartval;
        }
    } else {
        lookupthread_args *prevthargs = threadargpool + (tid - 1);
        thargs->startpoint = prevthargs->endpoint;

        if (tid == thcount - 1) {
            thargs->endpoint = entrycnt;
        } else {
            thargs->endpoint = (prevthargs->endpoint - prevthargs->startpoint)
                 + thargs->startpoint; 
        }
    }
}

int
setup_threadargs(lookupthread_args *threadargpool, const int tid, 
                const int thcount, const int entrycnt, const int thoutfd, 
                searchsyms *searchargs, char *patchdir, pthread_mutex_t *fmutex) {
    lookupthread_args *thargs;
    int descffd;
    char descfname[] = DESCFILE;

    if (thcount < 1 || entrycnt < 1 || tid < 0 || tid >= thcount) {
        error("Internal exception");
        return EXIT_FAILURE;
    }

    thargs = threadargpool + tid;
    descffd = mkstemp(descfname);
    
    if (descffd == -1) {
        EPERROR();
        return EXIT_FAILURE;
    }

    thargs->descfname = strndup(descfname, DESCFILE_LEN);
    thargs->descffd = descffd;
    thargs->outfd = thoutfd;
    thargs->mutex = fmutex;
    thargs->patchdir = patchdir;
    thargs->searchargs = searchargs;

    if (thcount == 1) {
        thargs->startpoint = 0;
        thargs->endpoint = entrycnt;
        return EXIT_SUCCESS;
    }

    assign_thread_bounds(threadargpool, tid, thcount, entrycnt);   
    return EXIT_SUCCESS;
}

void
cleanup_descfname(lookupthread_args *thargs) {
    free(thargs->descfname);
    free(thargs->patchdir);
}

void 
cleanup_searchargs(searchsyms *sargs) {
    free(sargs->searchstr);

    for (size_t i = 0; i < sargs->wordcount; i++) {
        free(sargs->words[i]);
    }
    free(sargs->words);
}

void
cleanup_threadargs(lookupthread_args *thargs) {
    cleanup_descfname(thargs);
    cleanup_searchargs(thargs->searchargs);
    free(thargs);
}

int 
run_multithreaded(char *patchdir, searchsyms *searchargs, const int entrycnt) {
    pthread_t *threadpool;
    pthread_mutex_t fmutex;
    lookupthread_args *thargs;
    FILE *rescache;
    int thcount, thpoolsize, rescachefd;
    char rescachename[] = RESULTCACHE, resc;
    int res = EXIT_FAILURE;

    rescachefd = mkstemp(rescachename);
    if (rescachefd == -1) {
        EPERROR();
        return res;
    }
    
    thcount = calc_threadcount(entrycnt);
    thpoolsize = sizeof(*threadpool) * thcount;
    threadpool = malloc(thpoolsize);
    thargs = malloc(sizeof(*thargs) * thcount);
    memset(threadpool, 0, thpoolsize);
    pthread_mutex_init(&fmutex, NULL);

    for (int tid = 0; tid < thcount; tid++) {
        if (setup_threadargs(thargs, tid, thcount, entrycnt, rescachefd, 
                            searchargs, patchdir, &fmutex)) {
            return res;
        }
        pthread_create(threadpool + tid, NULL, &search_entry, thargs + tid);
    }
    
    for (int tid = 0; tid < thcount; tid++) {
        pthread_join(threadpool[tid], NULL);
        res |= thargs[tid].result;
        cleanup_threadargs(thargs + tid);
    }
    
    pthread_mutex_destroy(&fmutex);
    free(threadpool);
    TRY(rescache = fdopen(rescachefd, "r")) 
    WITH(
        error("Failed to copy rescache"); 
        res = 1;
        goto cleanup)

    while ((resc = fgetc(rescache)) != EOF) {
        fputc(resc, stdout);
    }

cleanup:
    fclose(rescache);
    remove(rescachename);
    return !!res;
}

int run_search(char *patchdir, searchsyms *searchargs) {
    DIR *pd = NULL;
    struct dirent *pdir = NULL;
    int tentrycnt = 0;
    int res;

    TRY(pd = opendir(patchdir))
        WITH(error("Cache directory not found"))

    while ((pdir = readdir(pd)) != NULL) {
        if (pdir->d_type == DT_DIR) 
            tentrycnt++;
    }
    closedir(pd);

    if (worth_multithread(tentrycnt)) {
        res = run_multithreaded(patchdir, searchargs, tentrycnt);
    } else {
        lookupthread_args thargs;
        lookupthread_args *thargsp;
        int res = EXIT_FAILURE;

        if (setup_threadargs(&thargs, 0, 1, tentrycnt, STDOUT_FILENO, searchargs, patchdir, NULL)) {
            return res;
        }
        thargsp = &thargs;

        lookup_entries(thargsp);
        res = thargsp->result;
        cleanup_descfname(thargsp);
        cleanup_searchargs(thargsp->searchargs);
    }
    return res;
}

int parse_search_args(int argc, char **argv) {
    char *patchdir = NULL;
    searchsyms *searchargs = NULL;
    size_t startp = 0; 

    if (OK(strncmp(argv[CMD_ARGPOS], SEARCH_CMD, CMD_LEN))) {
        startp = 3;
    } else {
        startp = 2;
    }

    if (get_patchdir(basecacherepo, &patchdir, argv[TOOLNAME_ARGPOS])) {
        error("Suckless tool with name: '%s' not found", argv[TOOLNAME_ARGPOS]);
        return EXIT_FAILURE;
    }

    searchargs = malloc(sizeof(*searchargs));

    if (parse_search_symbols(searchargs, argv + startp, argc - startp)) {
        error("Invalid search string");
        return EXIT_FAILURE;
    }

    return run_search(patchdir, searchargs);
}