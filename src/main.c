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
#include "def.h"

char *basecacherepo = NULL;

char *
sappend(char *base, char *append) {
    size_t baselen = strlen(base);
    size_t pnamelen = strlen(append);
    char *buf = (char *)calloc(baselen + pnamelen + 1, sizeof(*buf));

    strncpy(buf, base, baselen);
    return strncat(buf, append, pnamelen);
}

char *
bufappend(char *buf, char *append) {
    return strncat(buf, append, PATHBUF);
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
print_usage() {
    printf("usage: sise <tool> <keywords>\n");
}


int
get_repocache(char **cachedirbuf) {
    char *homedir;
    struct passwd *pd;
    size_t homedirplen;

    homedir = getenv("HOME");
    if (!homedir) {
        pd = getpwuid(getuid());

        if (!pd)
            return 1;
        homedir = pd->pw_dir;
    }

    homedirplen = strnlen(homedir, PATHBUF);

    *cachedirbuf = (char *)malloc(sizeof(**cachedirbuf) * (PATHBUF + homedirplen));
    if (!*cachedirbuf)
        return 1;

    if (snprintf(*cachedirbuf, PATHBUF, "%s"BASEREPO, homedir) != 1) {
        memset(*cachedirbuf, ASCNULL, PATHBUF);
        strncpy(*cachedirbuf, homedir, PATHBUF);
        strncpy(*cachedirbuf + homedirplen, BASEREPO, PATHBUF);
    }
    return !*cachedirbuf;
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

char *
searchtool(char *baserepodir, char *toolname) {
    DIR *toolsdir = NULL;
    struct dirent *tdir = NULL;
    char *toolsdirpath;

    toolsdirpath = bufappend(baserepodir, TOOLSDIR);
    toolsdir = opendir(toolsdirpath);
    while ((tdir = readdir(toolsdir))) {
        if (OK(check_isdir(tdir))) {
            if (OK(strncmp(toolname, tdir->d_name, ENTRYLEN))) {
                return bufappend(toolsdirpath, toolname);
            }
        } 
    }
    
    closedir(toolsdir);
    return NULL;
}

int
get_patchdir(char *basecacherepo, char **patchdir, char *toolname) {
    if (OK(strncmp(toolname, DWM, ENTRYLEN))) {
        *patchdir = bufappend(basecacherepo, DWM_PATCHESDIR);
    } else if (OK(strncmp(toolname, ST, ENTRYLEN))) {
        *patchdir = bufappend(basecacherepo, ST_PATCHESDIR);
    } else if (OK(strncmp(toolname, SURF, ENTRYLEN))) {
        *patchdir = bufappend(basecacherepo, SURF_PATCHESDIR);
    } else {
        *patchdir = searchtool(basecacherepo, toolname);
        if (!*patchdir) {
            error("Suckless tool not found");
            return 1;
        }
    }   

    return !*patchdir;
}

int
parse_args(int argc, char **argv) {
    if (argc != 3) {
        print_usage();
        return EXIT_FAILURE;
    }
}

int
main(int argc, char **argv) {
    if (parse_args(argc, argv))
        return 1;

    if (get_repocache(&basecacherepo))
        return 1;
    
    return EXIT_SUCCESS;
}