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
#include "def.h"

#include "utils/pathutils.h"
#include "utils/logutils.h"
#include "utils/entry-utils.h"

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
    char *pubsearchstr = NULL, *token = NULL;
    size_t tstrcnt = 0;
    size_t lastalloc = 0, wid = 0;
    char delim[] = " ";

    ZIC_RESULT_INIT()

    sargs->words = NULL;
    sargs->searchstr = NULL;
    sargs->wordcount = 0;

    for (int i = 0; i < scount; i++) {
        char *searchstr = sstrings[i];
        int sstrcnt = 0, sstrlen = 0;
        char *context = NULL, *parsedsstr = NULL;
        
        sstrlen = strnlen(searchstr, MAXSEARCH_LEN) + 2;
        UNWRAP (check_entrname_valid(searchstr, sstrlen))

        pubsearchstr = strndup(searchstr, sstrlen);
        parsedsstr = strndup(searchstr, sstrlen);
        sstrcnt = getwords_count(searchstr, sstrlen);
        tstrcnt += sstrcnt;

        if (!words) {
            lastalloc = sstrcnt * scount;
            words = (char **)calloc(lastalloc, sizeof(*words));
        } else if (tstrcnt > lastalloc) {
            char **wrealloc = realloc(words, tstrcnt * 2);
            if (!wrealloc) {
                free(words);
                words = NULL;
            }
        }

        if (!words)
            goto cleanup;

        for (token = strtok_r(parsedsstr, delim, &context);
             token && wid < tstrcnt; wid++) {
            words[wid] = strndup(token, sstrlen);
            token = strtok_r(NULL, delim, &context);
        }

    cleanup:
        context = NULL;
        free(parsedsstr);
    }

    sargs->words = words;
    sargs->searchstr = pubsearchstr;
    sargs->wordcount = tstrcnt;
    return 0;
}

int
worth_multithread(int entrycount) {
    printf("\nEntrycount: %d\n", entrycount);
    (void)entrycount;

    #ifdef USE_MULTITHREADED
    return entrycount >= ((OPTWORK_AMOUNT * 2) - (OPTWORK_AMOUNT - (OPTWORK_AMOUNT / 4)));
    #else
    return OK;
    #endif
}

size_t
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
        ERROR(ERR_LOCAL)
    }

    thargs = threadargpool + tid;
    descffd = mkstemp(descfname);
    UNWRAP_NEG (descffd)

    thargs->descfname = strndup(descfname, DESCFILE_LEN);
    UNWRAP_PTR (thargs->descfname)

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
    free(sargs);
}

void
cleanup_threadargs(lookupthread_args *thargs) {
    cleanup_descfname(thargs);
    cleanup_searchargs(thargs->searchargs);
    free(thargs);
}

int 
run_multithreaded(char *patchdir, searchsyms *searchargs, const int entrycnt) {
    /*pthread_t *threadpool;
    pthread_mutex_t fmutex;
    lookupthread_args *thargs;
    FILE *rescache;
    int rescachefd;
    size_t thpoolsize, thcount;
    char rescachename[] = RESULTCACHE, resc;
    int res = EXIT_FAILURE;

    rescachefd = mkstemp(rescachename);
    if (rescachefd == -1) {
        EPERROR();
        return res;
    }
    
    thcount = calc_threadcount(entrycnt);
    thpoolsize = sizeof(*threadpool) * thcount;

    threadpool = calloc(1, thpoolsize);
    if (!threadpool) {
        perrfatal();
        goto cleanup;
    }

    thargs = malloc(sizeof(*thargs) * thcount);
    if (!thargs) {
        perrfatal();
        goto cleanup;
    }
    
    pthread_mutex_init(&fmutex, NULL);

    for (size_t tid = 0; tid < thcount; tid++) {
        if (setup_threadargs(thargs, tid, thcount, entrycnt, rescachefd, 
                            searchargs, patchdir, &fmutex)) {
            return res;
        }
        pthread_create(threadpool + tid, NULL, &search_entry, thargs + tid);
    }
    
    for (size_t tid = 0; tid < thcount; tid++) {
        pthread_join(threadpool[tid], NULL);
        res |= thargs[tid].result;
        cleanup_threadargs(thargs + tid);
    }
    
    UNWRAP_L (pthread_mutex_destroy(&fmutex))
    free(threadpool);
    TRY(rescache = fdopen(rescachefd, "r")) 
    WITH(
        fcache_error();
        res = 1;
        goto cleanup)

    while ((resc = fgetc(rescache)) != EOF) {
        fputc(resc, stdout);
    }

cleanup:
    fclose(rescache);
    remove(rescachename);
    return !!res;*/
    return 0;
}

int run_search(char *patchdir, searchsyms *searchargs) {
    DIR *pd = NULL;
    struct dirent *pdir = NULL;
    int tentrycnt = 0;

    ZIC_RESULT_INIT()

    UNWRAP_PTR (pd = opendir(patchdir))

    while ((pdir = readdir(pd))) {
        if (pdir->d_type == DT_DIR) 
            tentrycnt++;
    }
    UNWRAP (closedir(pd))
    
    if (worth_multithread(tentrycnt)) {
        ZIC_RESULT = run_multithreaded(patchdir, searchargs, tentrycnt);
    } else {
        lookupthread_args thargs = {0};
        lookupthread_args *thargsp = NULL;

        UNWRAP (
            setup_threadargs(&thargs, 0, 1, tentrycnt, STDOUT_FILENO, searchargs, patchdir, NULL))
        
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

    ZIC_RESULT_INIT()

    if (IS_OK(strncmp(argv[CMD_ARGPOS], SEARCH_CMD, CMD_LEN))) {
        startp++;
    }

    toolname_argpos = startp - TOOLNAME_ARGPOS;
    TRY (append_toolpath(&patchdir, basecacherepo, argv[toolname_argpos]),
        HANDLE ("Suckless tool with name: '%s' not found", argv[toolname_argpos])
    )

    searchargs = calloc(1, sizeof(*searchargs));
    UNWRAP_PTR (searchargs, cl_patchdir)

    TRY (parse_search_symbols(searchargs, argv + startp, argc - startp), 
        HANDLE_CLEANUP("Invalid search string")
    )

    ZIC_RESULT = run_search(patchdir, searchargs);

    CLEANUP(
        cl_searchargs: free(searchargs);
        cl_patchdir: free(patchdir)
    )
}