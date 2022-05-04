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
#include "deftypes.h"
#include "utils/logutils.h" 

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
    if (!buf)
        DIE_F()

    strncpy(buf, base, baselen);
    return strncat(buf, append, pnamelen);
}

result
spappend(char **bufp, const char *base, const char *append) {
    size_t baselen = strlen(base);
    size_t pnamelen = strlen(append);
    *bufp = calloc(baselen + pnamelen + 1, sizeof(**bufp));
    if (!bufp)
        return ERR_SYS;

    strncpy(bufp, base, baselen);
    strncat(bufp, append, pnamelen);
    if (!bufp)
        return ERR_SYS;

    return OK;
}

char *
bufappend(char *buf, const char *append) {
    if (!buf)
        return buf;

    return strncat(buf, append, PATHBUF);
}

int
check_tool_exists(const char *toolsdir_path, const char *toolname) {
    DIR *toolsdir = NULL;
    struct dirent *tdir = NULL;

    toolsdir = opendir(toolsdir_path);
    P_UNWRAP (toolsdir)

    while ((tdir = readdir(toolsdir))) {
        if (OK(check_isdir(tdir))) {
            if (OK(strncmp(toolname, tdir->d_name, ENTRYLEN))) {
                return OK;
            }
        } 
    }
    
    closedir(toolsdir);
}

result
append_tooldir(char **buf, const char *baserepodir, const char *toolname) {
   
    char *toolsdirpath;
    
    *buf = NULL;
    UNWRAP (spappend(&toolsdirpath, baserepodir, TOOLSDIR))
    
    return OK;
}

result
get_patchdir(char **patchdir, const char *toolname) {
    if (OK(strncmp(toolname, DWM, ENTRYLEN))) {
        UNWRAP (spappend(patchdir, basecacherepo, DWM_PATCHESDIR))
    } else if (OK(strncmp(toolname, ST, ENTRYLEN))) {
        UNWRAP (spappend(patchdir, basecacherepo, ST_PATCHESDIR))
    } else if (OK(strncmp(toolname, SURF, ENTRYLEN))) {
        UNWRAP (spappend(patchdir, basecacherepo, SURF_PATCHESDIR))
    } else {
        UNWRAP (append_tooldir(patchdir, basecacherepo, toolname))
    }   

    return OK;
}

bool 
check_baserepo_exists(void) {
    struct stat brst;

    if (OK(stat(basecacherepo, &brst))) {
        return S_ISDIR(brst.st_mode);
    }

    return false;
}