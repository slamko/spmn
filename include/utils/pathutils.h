int check_isdir(const struct dirent *dir);

char *sappend(const char *base, const char *append);

char *bufappend(char *buf, const char *append);

char *searchtool(char *baserepodir, const char *toolname);

int get_patchdir(char *basecacherepo, char **patchdir, const char *toolname);