#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include <errno.h>
#include <stdarg.h> 
#include <ctype.h>

#define DWM_PATCHES "/usr/local/src/sites/dwm.suckless.org/patches/"
#define INDEXMD "/index.md"
#define LINEBUF 4096
#define DESCRIPTION_SECTION "Description" 
#define DESCRIPTION_SECTION_LENGTH 12
#define GREP_BIN "/bin/grep"
#define DESCFILE "descfile.XXXXXX"
#define DESKFILE_LEN 15
#define RESULTCACHE "result.XXXXXX"
#define DEVNULL "/dev/null"
#define ASCNULL '\0'
#define OPTTHREAD_COUNT 4
#define OPTWORK_AMOUNT 80
#define MIN_WORKAMOUNT 40
#define ERRPREFIX_LEN 7
#define AVSEARCH_WORD_LEN 5
#define MAXSEARCH_LEN 512

//#define USE_MULTITHREADED

char *
sappend(char *base, char *append) {
    int baselen = strlen(base);
    int pnamelen = strlen(append);
    char *buf = (char *)calloc(baselen + pnamelen + 1, sizeof(*buf));

    strncpy(buf, base, baselen);
    return strncat(buf, append, pnamelen);
}

int
append_patchmd(char **buf, char *patch) {
    char *patchmd = sappend(patch, INDEXMD);
    *buf = sappend(DWM_PATCHES, patchmd);
    free(patchmd);
    return 0;
}

void 
error(const char* err_format, ...) {
    va_list args;
    int errlen;
    char *err;

    va_start(args, err_format);
    errlen = strlen(err_format);
    err = calloc(ERRPREFIX_LEN + errlen + 1, sizeof(*err));
    sprintf(err, "error: %s", err_format);

    vfprintf(stderr, err, args);
    vfprintf(stderr, "\n", args);
    free(err);
    va_end(args);
}

void 
empty() {}

#define TRY(EXP) (EXP)
#define WITH ? empty() : 
#define EPERROR() error(strerror(errno));
#define OK(RES) RES == 0

void
usage() {
    printf("usage\n");
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
searchdescr(FILE *descfile, const searchsyms *sargs) {
    char searchbuf[LINEBUF];
    char **swords = sargs->words;
    int matched_toks = 0;
    int res = 0;
    bool *matched;

    matched = (bool *)calloc(sargs->wordcount, sizeof(*matched));
    memset(searchbuf, ASCNULL, LINEBUF);

    while (fgets(searchbuf, LINEBUF, descfile)) {
        for (int i = 0; i < sargs->wordcount; i++) {
            if (!matched[i]) {
                if (strstr(searchbuf, swords[i])) {
                    matched_toks++;
                    matched[i] = true;

                    if (matched_toks >= sargs->wordcount)
                        goto cleanup;
                }
            }
        }
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

int check_isdir(struct dirent *dir) {
    struct stat dst;

    if (!dir || !dir->d_name)
        return 1;

    if (dir->d_name[0] == '.') 
        return 1;

    switch (dir->d_type)
    {
    case DT_DIR:
        return 0;
    case DT_UNKNOWN: 
        if (OK(stat(dir->d_name, &dst)))
            return S_ISDIR(dst.st_mode);  
        
        return 1;
    default: return 1;
    }
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

int
lookup_entries_args(const char *descfname, int startpoint, int endpoint, int outfd, 
                       const searchsyms *sargs, pthread_mutex_t *fmutex) {
    DIR *pd;
    FILE *descfile, *rescache;
    struct dirent *pdir;

    printf("\nStart point: %d", startpoint);
    printf("\nEnd point: %d", endpoint);

    TRY(rescache = fdopen(outfd, "w")) 
        WITH error("Failed to open result cache file");
    TRY(pd = opendir(DWM_PATCHES)) 
        WITH error("Failed to open patch dir");
    
    while ((pdir = readdir(pd)) != NULL) {
        if (OK(check_isdir(pdir))) {
            char *indexmd = NULL; 
            char dch;

            append_patchmd(&indexmd, pdir->d_name);
            TRY(descfile = fopen(descfname, "w+")) 
                WITH error("Failed to open descfile");
            
            if (OK(read_description(indexmd, descfile))) {
                fseek(descfile, 0, SEEK_SET);
                int searchres = searchdescr(descfile, sargs);

                lock_if_multithreaded(fmutex);
                fseek(descfile, 0, SEEK_SET);
                
                if (OK(searchres)) {
                    fprintf(rescache, "\n%s:\n", pdir->d_name);
                    while((dch = fgetc(descfile)) != EOF) {
                        fputc(dch, rescache);
                    }
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
        args->outfd, args->searchargs, args->mutex);
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

int
setup_threadargs(lookupthread_args *threadargpool, int tid, int thcount, int entrycnt,
                    int thoutfd, char *searchstr, pthread_mutex_t *fmutex) {
    lookupthread_args *thargs;
    int descffd;
    char descfname[] = DESCFILE;

    if (thcount < 1 || entrycnt < 1 || tid < 0 || tid >= thcount) {
        error("Internal exception");
        return 1;
    }

    thargs = threadargpool + tid;
    descffd = mkstemp(descfname);
    
    if (descffd == -1) {
        EPERROR();
        return 1;
    }

    thargs->descfname = strndup(descfname, DESKFILE_LEN);
    thargs->descffd = descffd;
    thargs->outfd = thoutfd;
    thargs->mutex = fmutex;

    thargs->searchargs = (searchsyms *)malloc(sizeof(*thargs->searchargs));
    if (parse_searchargs(thargs->searchargs, searchstr)) {
        error("Invalid search string");
        return 1;
    }

    if (thcount == 1) {
        thargs->startpoint = 0;
        thargs->endpoint = entrycnt;
        return 0;
    }

    if (OK(tid)) {
        thargs->startpoint = 0;
        if (thcount < OPTTHREAD_COUNT) {
            thargs->endpoint = thargs->startpoint + OPTWORK_AMOUNT;
        } else {
            double actworkamount = entrycnt / OPTTHREAD_COUNT;
            int approxstartval = (int)(floor(actworkamount / 100)) + 2;
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
    return 0;
}

void
cleanup_descfname(lookupthread_args *thargs) {
    free(thargs->descfname);
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

int
main(int argc, char **argv) {
    DIR *pd;
    char *searchstr;
    struct dirent *pdir;
    int tentrycnt = 0;

    if (argc < 2) {
        usage();
        return 1;
    }

    searchstr = argv[1];
    pd = opendir(DWM_PATCHES);

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
        int res = 1;

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
            if (setup_threadargs(thargs, tid, thcount, tentrycnt, rescachefd, searchstr, &fmutex)) {
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
            WITH error("Failed to copy rescache");

        while ((resc = fgetc(rescache)) != EOF) {
            fputc(resc, stdout);
        }
        fclose(rescache);
        remove(rescachename);
        return !!res;
    } else {
        lookupthread_args thargs;
        lookupthread_args *thargsp;
        int res = 1;

        if (setup_threadargs(&thargs, 0, 1, tentrycnt, STDOUT_FILENO, searchstr, NULL)) {
            return res;
        }
        thargsp = &thargs;

        lookup_entries(thargsp);
        res = thargsp->result;
        cleanup_descfname(thargsp);
        return res;
    }

    return 0;
}