#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "utils/pathutils.h"

#define MINI_ZIC
#include <zic/zic.h>

#include "def.h"

#define XDG_OPEN "/bin/xdg-open"
#define HTTPS_PREF "https://"
#define HTTPS_PLEN sizeof(HTTPS_PREF)

result 
xdg_open(const char *url) {
    int openst;
    
    if (!url)
        return ERR_LOCAL;

    if (fork() == 0) {
        if (execl(XDG_OPEN, XDG_OPEN, url, (char *)NULL)) {
            EPERROR()
            exit(1);
        }
    } 

    wait(&openst);
    return !!openst;
}

result 
build_url(char **url, const char *toolpath, const char *patch_name, size_t patchn_len) {
    size_t toolpath_len, url_len;

    toolpath_len = strnlen(toolpath, LINEBUF);
    url_len = HTTPS_PLEN + toolpath_len + patchn_len;
    *url = calloc(url_len, sizeof(**url));
    UNWRAP_PTR(*url)

    if (snprintf(*url, url_len, HTTPS_PREF "%s%s", toolpath, patch_name) != 2) {
        ERROR(ERR_LOCAL)
    }

    RET_OK()
}

result 
openp(const char *toolname, const char *patch_name, const char *basecacherepo) {
    size_t patchn_len, toolname_len;
    char *toolpath = NULL, *tooldir = NULL, *url = NULL;

    ZIC_RESULT_INIT()

    patchn_len = strnlen(patch_name, ENTRYLEN);
    toolname_len = strnlen(toolname, ENTRYLEN);

    TRY (check_entrname_valid(patch_name, patchn_len),
        HANDLE_CLENUP("Invalid patch name: '%s'", patch_name))

    TRY (check_entrname_valid(toolname, toolname_len),
        HANDLE_CLENUP("Invalid tool name: '%s'", toolname))

    TRY (get_tool_path(&toolpath, basecacherepo, toolname),
        HANDLE_CLENUP("Suckless tool with name: '%s' not found", toolname))

    TRY (append_tooldir(&tooldir, basecacherepo, toolpath), 
        ERROR(ERR_SYS, free_toolpath))

    TRY (check_patch_exists(tooldir, patch_name),
        HANDLE_CLENUP("A patch with name: '%s' not found", patch_name)
    )

    UNWRAP_CLEANUP (build_url(&url, toolpath, patch_name, patchn_len))
    
    UNWRAP_CLEANUP (xdg_open(url))

    CLEANUP(
        free_tooldir: free(tooldir);
        free_toolpath: free(toolpath))
}

int
parse_open_args(int argc, char **argv, const char *basecacherepo) {
    result res;
    
    if (argc != 4)
        return ERR_INVARG;
    
    res = openp(argv[2], argv[3], basecacherepo);
    return res;
}