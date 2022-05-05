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

result
append_tooldir(char **buf, const char *tooldir) {
    UNWRAP (spappend(buf, basecacherepo, tooldir))
    return OK;
}

result
search_tooldir(char **buf, const char *toolname) {
    char *toolsdir_path;
    DIR *toolsdir = NULL;
    struct dirent *tool = NULL;

    *buf = NULL;
    UNWRAP (spappend(&toolsdir_path, basecacherepo, TOOLSDIR))

    toolsdir = opendir(toolsdir_path);
    P_UNWRAP (toolsdir)

    while ((tool = readdir(toolsdir))) {
        if (OK(check_isdir(tool))) {
            if (OK(strncmp(toolname, tool->d_name, ENTRYLEN))) {
                *buf = tool->d_name;
            }
        } 
    }
    
    UNWRAP (closedir(toolsdir))
    return !!*buf;
}

result
get_patchdir(char **patchdir, const char *toolname) {
    if (OK(strncmp(toolname, DWM, ENTRYLEN))) {
        *patchdir = DWM; 
    } else if (OK(strncmp(toolname, ST, ENTRYLEN))) {
        *patchdir = ST;
    } else if (OK(strncmp(toolname, SURF, ENTRYLEN))) {
        *patchdir = SURF;
    } else {
        UNWRAP (search_tooldir(patchdir, toolname))
    }

    return OK;
}

result 
append_patchdir(char **buf, const char *toolname) {
    char *patchdir = NULL;
    UNWRAP (get_patchdir(&patchdir, toolname))
    UNWRAP (spappend(buf, basecacherepo, patchdir)) 
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