#include <stddef.h>
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
#include "def.h"
#include "utils/logutils.h" 

result 
check_isdir(const struct dirent *dir) {
    struct stat dst;
    ZIC_RESULT_INIT()

    if (!dir)
        FAIL();

    if (dir->d_name[0] == '.') 
        FAIL();

    switch (dir->d_type)
    {
    case DT_DIR:
        RET_OK();
    case DT_UNKNOWN: 
        TRY (stat(dir->d_name, &dst),
            HANDLE_SYS()
        )
        
        return S_ISDIR(dst.st_mode);  
    default: FAIL();
    }
}


result
snpappend(char **bufp, const char *base, const char *append, size_t base_len) {
    char *buf = NULL;
    size_t pnamelen;

    if (!base || !append)
        ERROR(ERR_LOCAL);

    pnamelen = strlen(append);

    *bufp = calloc(base_len + pnamelen + 1, sizeof(**bufp));
    UNWRAP_PTR(*bufp)

    buf = *bufp;

    strncpy(buf, base, base_len);
    strncat(buf, append, pnamelen);

    RET_OK();
}

result
spappend(char **bufp, const char *base, const char *append) {
    char *buf = NULL;
    size_t baselen;
    size_t pnamelen;

    if (!base || !append)
        ERROR(ERR_LOCAL);

    baselen = strlen(base);
    pnamelen = strlen(append);

    *bufp = calloc(baselen + pnamelen + 1, sizeof(**bufp));    
    UNWRAP_PTR(*bufp)
    
    buf = *bufp;

    strncpy(buf, base, baselen);
    strncat(buf, append, pnamelen);

    RET_OK();
}

result
bufpappend(char *buf, const char *append) {
    UNWRAP_PTR_ERR (buf, ERR_LOCAL)

    strncat(buf, append, PATHBUF);
    RET_OK();
}

result
bufnpappend(char *buf, const char *append, size_t nmax) {
    UNWRAP_PTR_ERR (buf, ERR_LOCAL)

    strncat(buf, append, nmax);
    RET_OK();
}

result 
append_patch_path(char **pbuf, const char *toolpath, const char *patchname) {
    size_t patchp_mlen;

    patchp_mlen = strlen(toolpath) + sizeof(PATCHESP) + strlen(patchname);
    
    *pbuf = calloc(patchp_mlen, sizeof(**pbuf));
    UNWRAP_PTR (*pbuf)

    UNWRAP (bufpappend(*pbuf, toolpath))
    UNWRAP (bufpappend(*pbuf, PATCHESP))
    UNWRAP (bufpappend(*pbuf, patchname))

    RET_OK();
}

result
append_tooldir(char **buf, const char *basecacherepo, const char *tooldir) {
    return spappend(buf, basecacherepo, tooldir);
}

result
search_tooldir(char **buf, const char *basecacherepo, const char *toolname) {
    char *toolsdir_path = NULL;
    DIR *toolsdir = NULL;
    struct dirent *tool = NULL;
    ZIC_RESULT_INIT()

    *buf = NULL;
    UNWRAP (spappend(&toolsdir_path, basecacherepo, TOOLSDIR));

	TRY_PTR (toolsdir = opendir(toolsdir_path), DO_CLEAN(cl_pbuf_free);

    while ((tool = readdir(toolsdir))) {
        if (IS_OK(check_isdir(tool))) {
            if (IS_OK(strncmp(toolname, tool->d_name, ENTRYLEN))) {
                TRY (
                    spappend(buf, TOOLSDIR, tool->d_name), DO_CLEAN_ALL()))
            }
        } 
    }

    ZIC_RESULT = !!*buf;

    CLEANUP_ALL (closedir(toolsdir));
	CLEANUP(cl_pbuf_free, free(toolsdir_path));
}

result
str_append_patch_dir(char **patchdir, const char *append, size_t psize) {
    *patchdir = calloc(psize, sizeof(**patchdir)); 
    UNWRAP_PTR (*patchdir)
    
    strncpy(*patchdir, append, psize - 1);
    RET_OK()
}

result
get_tool_path(char **patchdir, const char *basecacherepo, const char *toolname) {
    if (IS_OK(strncmp(toolname, DWM, ENTRYLEN))) {
        UNWRAP (
            str_append_patch_dir(patchdir, DWM_PATCHESDIR, sizeof(DWM_PATCHESDIR))
        )
    } else if (IS_OK(strncmp(toolname, ST, ENTRYLEN))) {
        UNWRAP (
            str_append_patch_dir(patchdir, ST_PATCHESDIR, sizeof(ST_PATCHESDIR))
        )
    } else if (IS_OK(strncmp(toolname, SURF, ENTRYLEN))) {
        UNWRAP (
            str_append_patch_dir(patchdir, SURF_PATCHESDIR, sizeof(SURF_PATCHESDIR))
        )
    } else {
        UNWRAP (search_tooldir(patchdir, basecacherepo, toolname))
    }

    RET_OK();
}

result 
append_toolpath(char **buf, const char *basecacherepo, const char *toolname) {
    char *patchdir = NULL;

    UNWRAP (get_tool_path(&patchdir, basecacherepo, toolname))
    UNWRAP (append_tooldir(buf, basecacherepo, patchdir)) 
    
    RET_OK();
}

result
get_repocache(char **cachedirbuf) {
    char *homedir = NULL;
    struct passwd *pd = NULL;
    size_t homedirplen;

    homedir = getenv("HOME");
    if (!homedir) {
        UNWRAP_PTR (pd = getpwuid(getuid()))

        homedir = pd->pw_dir;
    }

    homedirplen = strnlen(homedir, PATHBUF);

    *cachedirbuf = calloc(PATHBUF + homedirplen + 1, sizeof(**cachedirbuf));
    UNWRAP_PTR(*cachedirbuf)
    
    strlcpy(*cachedirbuf, homedir, PATHBUF);
    strlcpy(*cachedirbuf + homedirplen, BASEREPO, PATHBUF);
    
    RET_OK();
}

bool 
check_baserepo_exists(const char *basecacherepo) {
    struct stat brst;

    if (IS_OK(stat(basecacherepo, &brst))) {
        return S_ISDIR(brst.st_mode);
    }

    return false;
}
