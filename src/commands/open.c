#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "utils/pathutils.h"
#include "def.h"

static const char *const XDG_OPEN   = "/bin/xdg-open";
static const char *const HTTPS_PREF = "https://";
static const HTTPS_PLEN             = sizeof(HTTPS_PREF);

result 
xdg_open(const char *url) {
    int openst;
    
    if (!url)
        ERROR (ERR_LOCAL)

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

    if (snprintf(*url, url_len, "%s%s%s", HTTPS_PREF, toolpath, patch_name) != 3) {
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
        HANDLE("Invalid patch name: '%s'", patch_name))

    TRY (check_entrname_valid(toolname, toolname_len),
        HANDLE("Invalid tool name: '%s'", toolname))

    TRY (get_tool_path(&toolpath, basecacherepo, toolname),
        CATCH(ERR_ENTRY_NOT_FOUND, 
            HANDLE("Suckless tool with name: '%s' not found", toolname))

        CATCH(ERR_SYS, ERROR(ERR_SYS))
    )

    TRY (append_tooldir(&tooldir, basecacherepo, toolpath), 
        ERROR(ERR_SYS, free_toolpath))

    TRY (check_patch_exists(tooldir, patch_name),
        HANDLE_CLEANUP("A patch with name: '%s' not found", patch_name)
    )

    UNWRAP_CLEANUP (build_url(&url, toolpath, patch_name, patchn_len))
    
    UNWRAP_CLEANUP (xdg_open(url))

    CLEANUP(
        free_tooldir: free(tooldir);
        free_toolpath: free(toolpath))
}

result
parse_open_args(int argc, char **argv, const char *basecacherepo) {
    ZIC_RESULT_INIT()
    
    if (argc != 4)
        ERROR (ERR_INVARG)
    
    TRY (openp(argv[2], argv[3], basecacherepo), 
        CATCH(ERR_SYS,
            HANDLE_SYS()
        )

        CATCH(ERR_LOCAL,
            HANDLE_NO_FORMAT (bug(strerror(errno)))
        )
    )

    ZIC_RETURN_RESULT()
}