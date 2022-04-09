#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#define DWM_PATCHES "/usr/local/src/sites/dwm.suckless.org/patches/"

char *
concat(char *base, char *append) {
    int pdirlen = strlen(base);
    int pnamelen = strlen(append);
    char *buf = (char *)calloc(pdirlen + pnamelen + 1, sizeof(char));

    if (!buf)
        exit(0);

    strcpy(buf, base);
    strcpy(buf + pdirlen, append);
    return buf; 
}

int
append_patchmd(char **buf, char *patch) {
    char *patchmd = concat(patch, "/index.md");
    *buf = concat(DWM_PATCHES, patchmd);
    free(patchmd);
    return 0;
}

int
main(int argc, char **argv) {
    DIR *pd;
    struct dirent *dirent;

    pd = opendir(DWM_PATCHES);

    while ((dirent = readdir(pd)) != NULL) {
        if (dirent->d_type == DT_DIR) {
            char *indexmd = NULL; 
            append_patchmd(&indexmd, dirent->d_name);
            if (fork() == 0) {
                execl("/bin/grep", "/bin/grep", "some", indexmd, NULL);
            }
            free(indexmd);
        }
    }
    closedir(pd);
    return 0;
}