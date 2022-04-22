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

#define DWM_PATCHES "/usr/local/src/sites/dwm.suckless.org/patches/"
#define INDEXMD "/index.md"
#define LINEBUF 4096
#define DESCRIPTION_SECTION "Description" 
#define DESCRIPTION_SECTION_LENGTH 11
#define GREP_BIN "/bin/grep"
#define DESCFILE "descfile.XXXXXX"
#define RESULTCACHE "result.XXXXXX"
#define DEVNULL "/dev/null"
#define ASCNULL '\0'
#define OPTTHREAD_COUNT 4
#define OPTWORK_AMOUNT 80
#define MIN_WORKAMOUNT 40
#define ERRPREFIX_LEN 7

char *searchstr;

char *
concat(char *base, char *append) {
    int pdirlen = strlen(base);
    int pnamelen = strlen(append);
    char *buf = (char *)calloc(pdirlen + pnamelen + 1, sizeof(char));

    if (!buf)
        exit(0);

    strcpy(buf, base);
    strcpy(buf + pdirlen, append);
    return buf; 
}

int
append_patchmd(char **buf, char *patch) {
    char *patchmd = concat(patch, INDEXMD);
    *buf = concat(DWM_PATCHES, patchmd);
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
    err = calloc(ERRPREFIX_LEN + errlen + 1, sizeof(char));
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

void
eperror() {
    error(strerror(errno));
}

void
usage() {
    printf("usage\n");
}

int 
is_line_separator(char *line) {
    return (line[0] & line[1] & line[2]) == '-';
}

struct threadargs {
    char *descfname;
    int descffd;
    int outfd;
    int startpoint;
    int endpoint;
    int result;
    pthread_mutex_t *mutex;
};

typedef struct threadargs lookupthread_args;

int
read_description(char *indexmd, FILE *descfile) {
    FILE *index;
    int res = 1;
    int descrlen = 0;
    char linebuf[LINEBUF];
    bool description_exists = false;

    index = fopen(indexmd, "r");
    if (!index) {
        return 1;
    }

    memset(linebuf, ASCNULL, LINEBUF);

    while(fgets(linebuf, LINEBUF, index) != NULL) {
        if (description_exists) {
            descrlen++;
            if (is_line_separator(linebuf)) {
                if (descrlen > 1)
                    break;
            } else {
                fwrite(linebuf, sizeof(char), strlen(linebuf), descfile);
            }
        } else {
            char tittle[DESCRIPTION_SECTION_LENGTH + 1];
            memcpy(tittle, linebuf, DESCRIPTION_SECTION_LENGTH);
            tittle[DESCRIPTION_SECTION_LENGTH] = ASCNULL;
            if (strcmp(tittle, DESCRIPTION_SECTION) == 0) {
                description_exists = true;
            } 
        }
        memset(linebuf, ASCNULL, LINEBUF);
    }

    if (description_exists) {
        fclose(descfile);
        res = 0;
    }

    fclose(index);
    return res;
}

int isdir(struct dirent *dir) {
    struct stat dst;
    if (!dir || !dir->d_name)
        return 0;

    if (dir->d_name[0] == '.') 
        return 0;

    switch (dir->d_type)
    {
    case DT_DIR:
        return 1;
    case DT_UNKNOWN: 
        if (stat(dir->d_name, &dst) == 0)
            return S_ISDIR(dst.st_mode);
        
        return 0;  
    default:
        return 0;
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
lookup_entries_args(char *descfname, int descffd, int startpoint, int endpoint, 
                        int outfd, pthread_mutex_t *fmutex) {
    DIR *pd;
    FILE *descfile, *rescache;
    struct dirent *pdir;

    printf("\nStart point: %d", startpoint);
    printf("\nEnd point: %d", endpoint);

    TRY(rescache = fdopen(outfd, "w")) 
        WITH error("Failed to open result cache file");
    TRY(descfile = fdopen(descffd, "w+")) 
        WITH error("Failed to open descfile");
    TRY(pd = opendir(DWM_PATCHES)) 
        WITH error("Failed to open patch dir");

    while ((pdir = readdir(pd)) != NULL) {
        if (isdir(pdir)) {
            char *indexmd = NULL; 
            int grep, grepst;
            char dch;

            append_patchmd(&indexmd, pdir->d_name);
            truncate(descfname, 0);

            if (read_description(indexmd, descfile) == 0) {
                fseek(descfile, 0, SEEK_SET);
                grep = fork();
                if (grep == 0) {
                    int devnull = open(DEVNULL, O_WRONLY);
                    dup2(devnull, STDOUT_FILENO);
                    execl(GREP_BIN, GREP_BIN, searchstr, descfname, NULL);
                }
                wait(&grepst);
                lock_if_multithreaded(fmutex);

                if (grepst == 0) {
                    fprintf(rescache, "\n%s:\n", pdir->d_name);
                    while((dch = fgetc(descfile)) != EOF) {
                        fputc(dch, rescache);
                    }
                }

                unlock_if_multithreaded(fmutex);
            }
            free(indexmd);
        }
    }

    fclose(rescache);
    fclose(descfile);
    closedir(pd);
    remove(descfname);
    return 0;
}

int 
lookup_entries(lookupthread_args *args) {
    return lookup_entries_args(args->descfname, args->descffd, args->startpoint, 
        args->endpoint, args->outfd, args->mutex);
}

void *
search_entry(void *thread_args) {
    lookupthread_args *args = (lookupthread_args *)thread_args;
    args->result = lookup_entries(args);
    return NULL;
}

int
worth_multithread(int entrycount) {
    printf("\nEntrycount: %d\n", entrycount);
    //return entrycount >= ((OPTWORK_AMOUNT * 2) - (OPTWORK_AMOUNT - (OPTWORK_AMOUNT / 4)));
    return 0;
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
                    int thoutfd, pthread_mutex_t *fmutex) {
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
        eperror();
        return 1;
    }

    thargs->descfname = strdup(descfname);
    thargs->descffd = descffd;
    thargs->outfd = thoutfd;
    thargs->mutex = fmutex;

    if (thcount == 1) {
        thargs->startpoint = 0;
        thargs->endpoint = entrycnt;
        return 0;
    }

    if (tid == 0) {
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
cleanup_threadargs(lookupthread_args *thargs) {
    cleanup_descfname(thargs);
    free(thargs);
}

int
main(int argc, char **argv) {
    DIR *pd;
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
            eperror();
            return res;
        }
        
        thcount = calc_threadcount(tentrycnt);
        thpoolsize = sizeof(pthread_t) * thcount;
        threadpool = malloc(thpoolsize);
        thargs = malloc(sizeof(lookupthread_args) * thcount);
        memset(threadpool, 0, thpoolsize);
        pthread_mutex_init(&fmutex, NULL);

        for (int tid = 0; tid < thcount; tid++) {
            if (setup_threadargs(thargs, tid, thcount, tentrycnt, rescachefd, &fmutex)) {
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

        if (setup_threadargs(&thargs, 0, 1, tentrycnt, STDOUT_FILENO, NULL)) {
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