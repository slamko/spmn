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

static result 
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

    UNWRAP_NEG (wait(&openst))
    return !!openst;
}

result 
openp(const char *toolname, const char *patch_name, const char *basecacherepo) {
    char *url = NULL;
    ZIC_RESULT_INIT()

    UNWRAP (build_patch_url(&url, toolname, patch_name, basecacherepo))
    
    TRY (xdg_open(url),
        ERROR_CLEANUP(ERR_SYS)
    )

    CLEANUP(free(url))
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
            bug(strerror(errno));
            FAIL()
        )
    )

    ZIC_RETURN_RESULT()
}
