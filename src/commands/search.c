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
#include <math.h>
#include <errno.h>
#include <stdarg.h> 
#include <ctype.h>
#include <pwd.h>
#include "commands/search.h"
#include "deftypes.h"
#include "utils/entry-utils.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

int 
is_line_separator(const char *line) {
    return (line[0] & line[1] & line[2]) == '-';
}

int 
matched_all(const bool *is_matched, const size_t wordcount) {
    for (size_t i = 0; i < wordcount; i++) {
        if (!is_matched[i])
            return 1;
    }
    return 0;
}

int 
iter_search_words(const char *searchbuf, bool *matched, const searchsyms *sargs) {
    for (size_t i = 0; i < sargs->wordcount; i++) {
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
searchdescr(FILE *descfile, const char *toolname, const searchsyms *sargs) {
    char searchbuf[LINEBUF] = {0};
    int res = 0;
    bool *matched = NULL;

    matched = calloc(sargs->wordcount, sizeof(*matched));
    if (!matched)
        DIE_M()

    if (iter_search_words(toolname, matched, sargs))
        goto cleanup;

    while (fgets(searchbuf, sizeof(searchbuf), descfile)) {
        if (iter_search_words(searchbuf, matched, sargs))
            goto cleanup;

        memset(searchbuf, ASCNULL, sizeof(searchbuf));
    }

cleanup:
    res = matched_all(matched, sargs->wordcount);
    free(matched);
    return res;
}

char *
tryread_desc(FILE *index, char *buf, const bool descrexists) {
    if (descrexists) {
        return fgets(buf, LINEBUF, index);
    }
    return fgets(buf, DESCRIPTION_SECTION_LENGTH, index);
}

int
read_description(FILE *descfile, const char *indexmd) {
    FILE *index;
    int res = 1;
    char tempbuf[LINEBUF] = {0};
    char linebuf[LINEBUF] = {0};
    bool description_exists = false;

    index = fopen(indexmd, "r");
    if (!index)
        return res;

    for(int descrlen = 0; 
        tryread_desc(index, linebuf, description_exists) != NULL;) {
        if (description_exists) {
            if (is_line_separator(linebuf)) {
                if (descrlen > 1)
                    break;
            } else {
                if (descrlen > 0)
                    fputs(tempbuf, descfile);
                
                memcpy(tempbuf, linebuf, sizeof(linebuf));
            }
            descrlen++;
        } else {
            description_exists = !strcmp(linebuf, DESCRIPTION_SECTION);
        }
        memset(linebuf, ASCNULL, sizeof(linebuf));
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
print_matched_entry(FILE *descfile, FILE *targetf, const char *entryname) {
    char dch;
    static int matchedc;

    matchedc++;
    fprintf(targetf, "\n------------------------------------------------------------");
    fprintf(targetf, "\n%d) %s:\n", matchedc, entryname);

    while((dch = fgetc(descfile)) != EOF) {
        fputc(dch, targetf);
    }
}

int
lookup_entries_args(const char *descfname, const int startpoint, 
                    const int endpoint, const int outfd, const char *patchdir, 
                    const searchsyms *sargs, pthread_mutex_t *fmutex) {
    DIR *pd;
    FILE *descfile, *rescache;
    struct dirent *pdir;
    int res = 1;

    printf("\nStart point: %d", startpoint);
    printf("\nEnd point: %d", endpoint);

    TRY(rescache = fdopen(outfd, "w")) 
        WITH(fcache_error())

    TRY(pd = opendir(patchdir)) 
    WITH(
        fcache_error(); 
        goto cleanuprescache)
    
    while ((pdir = readdir(pd)) != NULL) {
        if (OK(check_isdir(pdir))) {
            char *indexmd = NULL; 

            TRY(descfile = fopen(descfname, "w+")) 
            WITH(
                fcache_error(); 
                goto cleanuppdir)

            if (append_patchmd(&indexmd, patchdir, pdir->d_name)) {
                perrfatal();
                goto cleandescfile;
            }
            
            if (OK(read_description(descfile, indexmd))) {
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

    res = 0;

cleandescfile:
    remove(descfname);
cleanuppdir:
    closedir(pd);
cleanuprescache:
    fclose(rescache);
    return res;
}

int 
lookup_entries(const lookupthread_args *args) {
    return lookup_entries_args(args->descfname, args->startpoint, args->endpoint, 
        args->outfd, args->patchdir, args->searchargs, args->mutex);
}

void *
search_entry(void *thread_args) {
    lookupthread_args *args = (lookupthread_args *)thread_args;
    args->result = lookup_entries(args);
    return NULL;
}