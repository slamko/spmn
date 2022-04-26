int check_isdir(struct dirent *dir);

char *sappend(char *base, char *append);

char *bufappend(char *buf, char *append);

char *searchtool(char *baserepodir, char *toolname);

int get_patchdir(char *basecacherepo, char **patchdir, char *toolname);