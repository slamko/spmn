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
#include "def.h"
#include "utils/entry-utils.h"
#include "utils/pathutils.h"
#include "utils/logutils.h"

static int 
is_line_separator(const char *line) {
    return (line[0] & line[1] & line[2]) == '-';
}

static result 
check_matched_all(const bool *is_matched, const size_t wordcount) {
    for (size_t i = 0; i < wordcount; i++) {
        if (!is_matched[i])
            FAIL()
    }
    RET_OK()
}

static int 
iter_search_words(const char *searchbuf, bool *matched, const searchsyms *sargs) {
    for (size_t i = 0; i < sargs->wordcount; i++) {
        if (!matched[i]) {
            if (strstr(searchbuf, sargs->words[i])) {
                matched[i] = true;

                if (IS_OK(check_matched_all(matched, sargs->wordcount)))
                    return 1;
            }
        }
    }
    return 0;
}

static int
toolname_contains_searchword(const char *toolname, bool *matched, const searchsyms *sargs) {
    return iter_search_words(toolname, matched, sargs);
}

static result
searchdescr(FILE *descfile, const char *toolname, const searchsyms *sargs) {
    char searchbuf[LINEBUF] = {0};
    bool *matched = NULL;

    ZIC_RESULT_INIT()

    matched = calloc(sargs->wordcount, sizeof(*matched));
    UNWRAP_PTR (matched)

    if (toolname_contains_searchword(toolname, matched, sargs))
        RET_OK_DO_CLEAN_ALL()

    while (fgets(searchbuf, sizeof(searchbuf), descfile)) {
        if (iter_search_words(searchbuf, matched, sargs))
            RET_OK_DO_CLEAN_ALL()

        memset(searchbuf, ASCNULL, sizeof(searchbuf));
    }

    ZIC_RESULT = check_matched_all(matched, sargs->wordcount);
    
    CLEANUP_ALL(
        free(matched);
        fseek(descfile, 0, SEEK_SET);
	);
	ZIC_RETURN_RESULT()
}

static char *
tryread_desc(FILE *index, char *buf, const bool descrexists) {
    if (descrexists) {
        return fgets(buf, LINEBUF, index);
    }
    return fgets(buf, DESCRIPTION_SECTION_LENGTH, index);
}

static int
read_description(FILE *descfile, const char *indexmd) {
    FILE *index = NULL;
    char tempbuf[LINEBUF] = {0};
    char linebuf[LINEBUF] = {0};
    bool description_exists = false;

    ZIC_RESULT_INIT()

    UNWRAP_PTR (index = fopen(indexmd, "r"))

    for(int descrlen = 0; 
        tryread_desc(index, linebuf, description_exists);
        ) {
        if (description_exists) {
            if (is_line_separator(linebuf)) {
                if (descrlen > 1)
                    break;
            } else {
                if (descrlen > 0)
                    TRY_UNWRAP_NEG (fputs(tempbuf, descfile), DO_CLEAN_ALL());
                
                memcpy(tempbuf, linebuf, sizeof(linebuf));
            }
            descrlen++;
        } else {
            description_exists = !strcmp(linebuf, DESCRIPTION_SECTION);
        }
        memset(linebuf, ASCNULL, sizeof(linebuf));
    }

    if (description_exists) {
        TRY_UNWRAP_NEG (fflush(descfile), DO_CLEAN_ALL());
    }

	ZIC_RESULT = OK;
	CLEANUP_ALL (fclose(index));
	ZIC_RETURN_RESULT()
}

static result 
lock_if_multithreaded(pthread_mutex_t *mutex) {
    if (mutex)
        return pthread_mutex_lock(mutex);

    RET_OK()
}

static result 
unlock_if_multithreaded(pthread_mutex_t *mutex) {
    if (mutex)
        return pthread_mutex_unlock(mutex);
    
    RET_OK()
}

static result
get_descfile_size(FILE *descfile, size_t *size_p) {
    UNWRAP_NEG (fseek(descfile, 0, SEEK_END));
    UNWRAP_NEG (*size_p = ftell(descfile));
    UNWRAP_NEG (fseek(descfile, 0, SEEK_SET));
	RET_OK();
}

static result
print_full_patch(FILE *descfile, int matchedc, const char *entryname, FILE *targetf) {
	char *print_buf = NULL;
    size_t descf_size;
	ZIC_RESULT_INIT();

	fputs( "--------------------------------------------------", targetf);
    fprintf(targetf, "\n%d) %s:\n", matchedc, entryname);

    UNWRAP (get_descfile_size(descfile, &descf_size));

    print_buf = calloc(descf_size + 1, sizeof(*print_buf));
    UNWRAP_PTR (print_buf)

    if (fread(print_buf, descf_size, sizeof(*print_buf), descfile) == 0) {
        TRY (ferror(descfile), DO_CLEAN_ALL())
    }

    if (fwrite(print_buf, descf_size, sizeof(*print_buf), targetf) == 0) {
        TRY (ferror(descfile), DO_CLEAN_ALL())
    }
	
    CLEANUP_ALL(free(print_buf));
	ZIC_RETURN_RESULT()
}

static result 
print_matched_entry(FILE *descfile, FILE *targetf, const char *entryname, bool print_full_patch_description) {
    static int matchedc;

    matchedc++;
	if (print_full_patch_description) {
		UNWRAP(print_full_patch(descfile, matchedc, entryname, targetf));
	} else {
		fprintf(targetf, "%d) %s\n", matchedc, entryname);
	}
	
	RET_OK()
}

static result
lookup_entries_args(const char *descfname, 
                    const int startpoint, const int endpoint, 
                    const int outfd, 
                    const char *patchdir, 
                    const searchsyms *sargs, 
                    pthread_mutex_t *fmutex) {
    DIR *pd = NULL;
    FILE *rescache = NULL;
    struct dirent *pdir = NULL;

    ZIC_RESULT_INIT()

    rescache = fdopen(outfd, "w");
    UNWRAP_PTR (rescache);

	TRY_PTR (pd = opendir(patchdir), DO_CLEAN(cl_rescache));
    
    for (int entrid = 1;
        (pdir = readdir(pd)) && 
            (entrid > startpoint && entrid <= endpoint); 
        entrid++) 
        {
        if (IS_OK(check_isdir(pdir))) {
            FILE *descfile = NULL;
            char *indexmd = NULL; 

            TRY_PTR (descfile = fopen(descfname, "w+"), DO_CLEAN(cl_pdir));

			TRY (append_patchmd(&indexmd, patchdir, pdir->d_name), DO_CLEAN_ALL());

            if (IS_OK(read_description(descfile, indexmd))) {
                result search_res;

                TRY (fseek(descfile, 0, SEEK_SET), DO_CLEAN_ALL())

                search_res = searchdescr(descfile, pdir->d_name, sargs);
                lock_if_multithreaded(fmutex);
                
                if (IS_OK(search_res)) {
                    TRY (print_matched_entry(descfile, rescache, pdir->d_name, sargs->s_flags.print_full_patch),
						 DO_CLEAN_ALL()
                    )
                }

                unlock_if_multithreaded(fmutex);
            }
            free(indexmd);
            fclose(descfile);
        }
    }

	ZIC_RESULT = OK;
    CLEANUP_ALL(remove(descfname));
	CLEANUP (cl_pdir, closedir(pd));
	CLEANUP (cl_rescache, fclose(rescache));
	ZIC_RETURN_RESULT()
}

result 
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
