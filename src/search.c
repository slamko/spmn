#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <stdarg.h> 
#include <ctype.h>
#include <pwd.h>
#include "deftypes.h"

int
append_patchmd(char **buf, char *patchdir, char *patch) {
    char *patchmd = sappend(patch, INDEXMD);
    *buf = sappend(patchdir, patchmd);
    free(patchmd);
    return !*buf;
}


int 
is_line_separator(char *line) {
    return (line[0] & line[1] & line[2]) == '-';
}

typedef struct searchargs {
    char **words; 
    char *searchstr;
    int wordcount;
} searchsyms;

struct threadargs {
    char *descfname;
    int descffd;
    int outfd;
    int startpoint;
    int endpoint;
    int result;
    char *patchdir;
    pthread_mutex_t *mutex;
    searchsyms *searchargs;
};

typedef struct threadargs lookupthread_args;

int 
matched_all(bool *is_matched, int wordcount) {
    for (int i = 0; i < wordcount; i++) {
        if (!is_matched[i])
            return 1;
    }
    return 0;
}

int 
iter_search_words(char *searchbuf, bool *matched, const searchsyms *sargs) {
    for (int i = 0; i < sargs->wordcount; i++) {
        if (!matched[i]) {
            if (strstr(searchbuf, sargs->words[i])) {
                matched[i] = true;

                if (OK(matched_all(matched, sargs->wordcount)))
                    return 1;
            }
        }
    }
    return 0;
}

int
searchdescr(FILE *descfile, char *toolname, const searchsyms *sargs) {
    char searchbuf[LINEBUF];
    int res = 0;
    bool *matched = NULL;

    matched = (bool *)calloc(sargs->wordcount, sizeof(*matched));
    if (iter_search_words(toolname, matched, sargs))
        goto cleanup;

    memset(searchbuf, ASCNULL, LINEBUF);

    while (fgets(searchbuf, LINEBUF, descfile)) {
        if (iter_search_words(searchbuf, matched, sargs))
            goto cleanup;

        memset(searchbuf, ASCNULL, LINEBUF);
    }

cleanup:
    res = matched_all(matched, sargs->wordcount);
    free(matched);
    return res;
}

char *
tryread_desc(FILE *index, char *buf, bool descrexists) {
    if (descrexists) {
        return fgets(buf, LINEBUF, index);
    }
    return fgets(buf, DESCRIPTION_SECTION_LENGTH, index);
}

int
read_description(char *indexmd, FILE *descfile) {
    FILE *index;
    int res = 1;
    char tempbuf[LINEBUF];
    char linebuf[LINEBUF];
    bool description_exists = false;

    index = fopen(indexmd, "r");
    if (!index)
        return 1;

    memset(linebuf, ASCNULL, LINEBUF);
    memset(tempbuf, ASCNULL, LINEBUF);

    for(int descrlen = 0; 
        tryread_desc(index, linebuf, description_exists) != NULL;) {
        if (description_exists) {
            if (is_line_separator(linebuf)) {
                if (descrlen > 1)
                    break;
            } else {
                if (descrlen > 0)
                    fputs(tempbuf, descfile);
                
                memcpy(tempbuf, linebuf, LINEBUF);
            }
            descrlen++;
        } else {
            description_exists = !strcmp(linebuf, DESCRIPTION_SECTION);
        }
        memset(linebuf, ASCNULL, LINEBUF);
    }

    if (description_exists) {
        fflush(descfile);
        res = 0;
    }

    fclose(index);
    return res;
}

void 
lock_if_multithreaded(pthread_mutex_t *mutex) {
    if (mutex)
        pthread_mutex_lock(mutex);
}

void 
unlock_if_multithreaded(pthread_mutex_t *mutex) {
    if (mutex)
        pthread_mutex_unlock(mutex);
}

void 
print_matched_entry(FILE *descfile, FILE *targetf, char *entryname) {
    char dch;
    static int matchedc;

    matchedc++;
    fprintf(targetf, "\n--------------------------------------------------");
    fprintf(targetf, "\n%d) %s:\n", matchedc, entryname);

    while((dch = fgetc(descfile)) != EOF) {
        fputc(dch, targetf);
    }
}

int
lookup_entries_args(const char *descfname, int startpoint, int endpoint, int outfd, 
                       char *patchdir, const searchsyms *sargs, pthread_mutex_t *fmutex) {
    DIR *pd;
    FILE *descfile, *rescache;
    struct dirent *pdir;

    printf("\nStart point: %d", startpoint);
    printf("\nEnd point: %d", endpoint);

    TRY(rescache = fdopen(outfd, "w")) 
        WITH(error("Failed to open result cache file"))
    TRY(pd = opendir(patchdir)) 
        WITH(error("Failed to open patch dir"))
    
    while ((pdir = readdir(pd)) != NULL) {
        if (OK(check_isdir(pdir))) {
            char *indexmd = NULL; 

            if (append_patchmd(&indexmd, patchdir, pdir->d_name)) {
                error("Internal exception");
                return 1;
            }

            TRY(descfile = fopen(descfname, "w+")) 
                WITH(error("Failed to open descfile"))
            
            if (OK(read_description(indexmd, descfile))) {
                fseek(descfile, 0, SEEK_SET);
                int searchres = searchdescr(descfile, pdir->d_name, sargs);

                lock_if_multithreaded(fmutex);
                fseek(descfile, 0, SEEK_SET);
                
                if (OK(searchres)) {
                    print_matched_entry(descfile, rescache, pdir->d_name);
                }

                unlock_if_multithreaded(fmutex);
            }
            free(indexmd);
            fclose(descfile);
        }
    }

    fclose(rescache);
    closedir(pd);
    remove(descfname);
    return 0;
}

int 
lookup_entries(lookupthread_args *args) {
    return lookup_entries_args(args->descfname, args->startpoint, args->endpoint, 
        args->outfd, args->patchdir, args->searchargs, args->mutex);
}

void *
search_entry(void *thread_args) {
    lookupthread_args *args = (lookupthread_args *)thread_args;
    args->result = lookup_entries(args);
    return NULL;
}

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
parse_searchargs(searchsyms *sargs, char *searchstr){
    char **words;
    char *pubsearchstr, *parsesearchstr;
    int sstrcnt;
    char *token;
    int sstrlen;
    char *delim = " ", *context = NULL;

    sstrlen = strnlen(searchstr, MAXSEARCH_LEN);
    sargs->words = NULL;
    sargs->searchstr = NULL;
    sargs->wordcount = 0;

    if (searchstr_invalid(searchstr, sstrlen))
        return 1;

    pubsearchstr = strndup(searchstr, sstrlen);
    parsesearchstr = strndup(searchstr, sstrlen);
    sstrcnt = getwords_count(searchstr, sstrlen);

    words = (char **)calloc(sstrcnt, sizeof(token));
    token = strtok_r(parsesearchstr, delim, &context);

    for (int i = 0; token && i < sstrcnt; i++) {
        words[i] = strndup(token, sstrlen);
        token = strtok_r(NULL, delim, &context);
    }

    sargs->words = words;
    sargs->searchstr = pubsearchstr;
    sargs->wordcount = sstrcnt;
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
assign_thread_bounds(lookupthread_args *threadargpool, int tid, int thcount, int entrycnt) {
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
            thargs->endpoint = (prevthargs->endpoint - prevthargs->startpoint) + thargs->startpoint; 
        }
    }
}

int
setup_threadargs(lookupthread_args *threadargpool, int tid, int thcount, int entrycnt,
                    int thoutfd, searchsyms *searchargs, char *patchdir, pthread_mutex_t *fmutex) {
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

    thargs->descfname = strndup(descfname, DESKFILE_LEN);
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

    for (int i = 0; i < sargs->wordcount; i++) {
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

int run_search(int argc, char **argv) {
    DIR *pd = NULL;
    char *searchstr = NULL;
    char *patchdir = NULL;
    searchsyms *searchargs = NULL;
    struct dirent *pdir = NULL;
    int tentrycnt = 0;

    if (get_patchdir(basecacherepo, &patchdir, argv[TOOLNAME_ARGPOS])) {
        return EXIT_FAILURE;
    }

    searchargs = (searchsyms *)malloc(sizeof(*searchargs));
    searchstr = argv[KEYWORD_ARGPOS];

    if (parse_searchargs(searchargs, searchstr)) {
        error("Invalid search string");
        return EXIT_FAILURE;
    }

    TRY(pd = opendir(patchdir))
        WITH(error("Cache directory not found"))

    while ((pdir = readdir(pd)) != NULL) {
        if (pdir->d_type == DT_DIR) 
            tentrycnt++;
    }
    closedir(pd);

    if (worth_multithread(tentrycnt)) {
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
        
        thcount = calc_threadcount(tentrycnt);
        thpoolsize = sizeof(*threadpool) * thcount;
        threadpool = malloc(thpoolsize);
        thargs = malloc(sizeof(*thargs) * thcount);
        memset(threadpool, 0, thpoolsize);
        pthread_mutex_init(&fmutex, NULL);

        for (int tid = 0; tid < thcount; tid++) {
            if (setup_threadargs(thargs, tid, thcount, tentrycnt, rescachefd, searchargs, patchdir, &fmutex)) {
                return res;
            }
            pthread_create(threadpool + tid, NULL, *search_entry, thargs + tid);
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
        return res;
    }
}