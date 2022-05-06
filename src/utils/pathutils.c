#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <bsd/string.h>
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
    char *buf = NULL;
    size_t baselen;
    size_t pnamelen;

    if (!base || !append)
        return ERR_LOCAL;

    baselen = strlen(base);
    pnamelen = strlen(append);
    *bufp = calloc(baselen + pnamelen + 1, sizeof(**bufp));
    buf = *bufp;

    if (!buf)
        return ERR_SYS;

    strncpy(buf, base, baselen);
    strncat(buf, append, pnamelen);

    return OK;
}

char *
bufappend(char *buf, const char *append) {
    if (!buf)
        return buf;

    return strncat(buf, append, PATHBUF);
}

result
bufpappend(char *buf, const char *append) {
    if (!buf)
        return ERR_LOCAL;

    P_UNWRAP (strncat(buf, append, PATHBUF))
    return OK;
}

result 
append_patch_path(char **pbuf, const char *toolpath, const char *patchname) {
    size_t patchp_mlen;

    patchp_mlen = strlen(toolpath) + sizeof(PATCHESP) + strlen(patchname);
    *pbuf = calloc(patchp_mlen, sizeof(**pbuf));
    P_UNWRAP ( *pbuf)

    if (snprintf(*pbuf, patchp_mlen, "%s%s%s", toolpath, PATCHESP, patchname) != 3) {
        UNWRAP (bufpappend(*pbuf, toolpath))
        UNWRAP (bufpappend(*pbuf, PATCHESP))
        UNWRAP (bufpappend(*pbuf, patchname))
    }

    return OK;
}

result
append_tooldir(char **buf, const char *basecacherepo, const char *tooldir) {
    UNWRAP (spappend(buf, basecacherepo, tooldir))
    return OK;
}

result
search_tooldir(char **buf, const char *basecacherepo, const char *toolname) {
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
                UNWRAP (spappend(buf, TOOLSDIR, tool->d_name))
            }
        } 
    }
    
    UNWRAP (closedir(toolsdir))
    return !!*buf;
}

result
get_tool_path(char **patchdir, const char *basecacherepo, const char *toolname) {
    if (OK(strncmp(toolname, DWM, ENTRYLEN))) {
        size_t psize = sizeof(DWM_PATCHESDIR);
        *patchdir = calloc(psize, sizeof(**patchdir)); 
        strncpy(*patchdir, DWM_PATCHESDIR, psize - 1);
    } else if (OK(strncmp(toolname, ST, ENTRYLEN))) {
        size_t psize = sizeof(ST_PATCHESDIR);
        *patchdir = calloc(psize, sizeof(**patchdir)); 
        strncpy(*patchdir, ST_PATCHESDIR, psize - 1);
    } else if (OK(strncmp(toolname, SURF, ENTRYLEN))) {
        size_t psize = sizeof(SURF_PATCHESDIR);
        *patchdir = calloc(psize, sizeof(**patchdir)); 
        strncpy(*patchdir, SURF_PATCHESDIR, psize - 1);
    } else {
        UNWRAP (search_tooldir(patchdir, basecacherepo, toolname))
    }

    return OK;
}

result 
append_toolpath(char **buf, const char *basecacherepo, const char *toolname) {
    char *patchdir = NULL;
    UNWRAP (get_tool_path(&patchdir, basecacherepo, toolname))
    UNWRAP (append_tooldir(buf, basecacherepo, patchdir)) 
    return OK;
}

result
get_repocache(char **cachedirbuf) {
    char *homedir = NULL;
    struct passwd *pd = NULL;
    size_t homedirplen;

    homedir = getenv("HOME");
    if (!homedir) {
        pd = getpwuid(getuid());
        P_UNWRAP (pd)

        homedir = pd->pw_dir;
    }

    homedirplen = strnlen(homedir, PATHBUF);

    *cachedirbuf = calloc(PATHBUF + homedirplen + 1, sizeof(**cachedirbuf));
    P_UNWRAP(*cachedirbuf)
    
    strlcpy(*cachedirbuf, homedir, PATHBUF);
    strlcpy(*cachedirbuf + homedirplen, BASEREPO, PATHBUF);
    return OK;
}

bool 
check_baserepo_exists(const char *basecacherepo) {
    struct stat brst;

    if (OK(stat(basecacherepo, &brst))) {
        return S_ISDIR(brst.st_mode);
    }

    return false;
}