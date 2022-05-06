#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "utils/logutils.h"
#include "utils/entry-utils.h"
#include "utils/pathutils.h"
#include "deftypes.h"

#define XDG_OPEN "/bin/xdg-open"

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
openp(const char *toolname, const char *patch_name, const char *basecacherepo) {
    int patchn_len = strnlen(patch_name, ENTRYLEN);
    int toolname_len = strnlen(toolname, ENTRYLEN);
    char *toolpath = NULL;
    char *tooldir = NULL;
    char *url = NULL;
    printf("start\n");

    if (entrname_valid(patch_name, patchn_len))
        HANDLE_ERR("Invalid patch name: '%s'", patch_name)

    if (entrname_valid(toolname, toolname_len))
        HANDLE_ERR("Invalid tool name: '%s'", toolname)

    if (!OK(get_tool_path(&toolpath, basecacherepo, toolname)))
        HANDLE_ERR("Suckless tool with name: '%s' not found", toolname);
    
    UNWRAP (append_tooldir(&tooldir, basecacherepo, toolpath))

    if (!OK(check_patch_exists(tooldir, patch_name))) {
        free(tooldir);
        //free(toolpath);
        HANDLE_ERR("A patch with name: '%s' not found", patch_name);
    }

    UNWRAP (append_patch_path(&url, toolpath, patch_name))
    UNWRAP (xdg_open(url))
    printf("hello\n");

    return OK;
}

int
parse_open_args(int argc, char **argv, const char *basecacherepo) {
    result res;
    
    if (argc != 4)
        return ERR_INVARG;
    
    res = openp(argv[2], argv[3], basecacherepo);
    return res;
}