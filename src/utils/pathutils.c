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
#include <time.h>
#include "def.h"
#include "search.h"
#include "sync.h"

int 
check_isdir(const struct dirent *dir) {
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
sappend(const char *base, const char *append) {
    size_t baselen = strlen(base);
    size_t pnamelen = strlen(append);
    char *buf = calloc(baselen + pnamelen + 1, sizeof(*buf));

    strncpy(buf, base, baselen);
    return strncat(buf, append, pnamelen);
}

char *
bufappend(char *buf, const char *append) {
    return strncat(buf, append, PATHBUF);
}

char *
searchtool(char *baserepodir, const char *toolname) {
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
get_patchdir(char *basecacherepo, char **patchdir, const char *toolname) {
    if (OK(strncmp(toolname, DWM, ENTRYLEN))) {
        *patchdir = bufappend(basecacherepo, DWM_PATCHESDIR);
    } else if (OK(strncmp(toolname, ST, ENTRYLEN))) {
        *patchdir = bufappend(basecacherepo, ST_PATCHESDIR);
    } else if (OK(strncmp(toolname, SURF, ENTRYLEN))) {
        *patchdir = bufappend(basecacherepo, SURF_PATCHESDIR);
    } else {
        *patchdir = searchtool(basecacherepo, toolname);
        if (!*patchdir) {
            return 1;
        }
    }   

    return !*patchdir;
}