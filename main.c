#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>

#define DWM_PATCHES "/usr/local/src/sites/dwm.suckless.org/patches/"
#define LINEBUF 4096
#define DESCRIPTION_SECTION "Description" 
#define DESCRIPTION_SECTION_LENGTH 11
#define GREP_BIN "/bin/grep"
#define DESCFILE "~/.config/sise/descfile.XXXXXX"
#define DEVNULL "/dev/null"

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
is_line_separator(char *line) {
    return line[0] == '-' && line[1] == '-' && line[2] == '-';
}

int
read_description(char *indexmd, int descffd) {
    FILE *descfile;
    FILE *index;
    int res = 1;
    int descrlen = 0;
    char linebuf[4096];
    bool description_exists = false;

    index = fopen(indexmd, "r");
    if (!index) {
        return 1;
    }

    memset(linebuf, '\0', LINEBUF);

    while(fgets(linebuf, LINEBUF, index) != NULL) {
        if (description_exists) {
            descrlen++;
            if (is_line_separator(linebuf)) {
                if (descrlen > 1)
                    break;
            } else {
                fwrite(linebuf, sizeof(char), strlen(linebuf), descfile);
            }
        } else {
            char tittle[DESCRIPTION_SECTION_LENGTH + 1];
            memcpy(tittle, linebuf, DESCRIPTION_SECTION_LENGTH);
            tittle[DESCRIPTION_SECTION_LENGTH] = '\0';
            if (strcmp(tittle, DESCRIPTION_SECTION) == 0) {
                description_exists = true;
                descfile = fdopen(descffd, "w");
                if (!descfile) {
                    goto cleanup;
                }
            } 
        }
        memset(linebuf, '\0', LINEBUF);
    }

    if (description_exists) {
        fclose(descfile);
        res = 0;
    }

cleanup:
    fclose(index);
    return res;
}

int
main(int argc, char **argv) {
    DIR *pd;
    struct dirent *dirent;
    int descffd;
    char *deskfname;
    pd = opendir(DWM_PATCHES);
    deskfname = DESCFILE;
    descffd = mkstemp(deskfname);

    while ((dirent = readdir(pd)) != NULL) {
        if (dirent->d_type == DT_DIR) {
            FILE *descfile = NULL;
            char *indexmd = NULL; 
            int grep, grepst, devnull;
            char dch;

            append_patchmd(&indexmd, dirent->d_name);

            if (read_description(indexmd, descffd)) {
                grep = fork();
                descfile = fDopen(descffd, "r");
                devnull = open(DEVNULL, O_WRONLY);
                if (grep == 0) {
                    dup2(devnull, STDOUT_FILENO);
                    execl(GREP_BIN, GREP_BIN, argv[1], deskfname, NULL);
                }
                wait(&grepst);

                if (grepst == 0) {
                    printf("\n%s:\n", dirent->d_name);
                    while((dch = fgetc(descfile)) != EOF) {
                        fputc(dch, stdout);
                    }
                }
                fclose(descfile);
                descfile = NULL;
            }
            free(indexmd);
        }
    }
    closedir(pd);
    return 0;
}