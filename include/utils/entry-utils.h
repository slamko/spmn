#include "def.h"
#include <stddef.h>

#define HTTPS_PREF "https://"

int append_patchmd(char **buf, const char *patchdir, char *patch);

int check_entrname_valid(const char *entryname, const int enamelen);

result build_patch_path(char **path, const char *toolname, 
                        const char *patch_name, size_t patchn_len, 
                        const char *basecacherepo);

result build_patch_dir(char **pdir, const char *toolname,
                        const char *patch_name, size_t patchn_len,
                        const char *basecacherepo);

result build_patch_url(char **url, const char *toolname, 
                        const char *patch_name, const char *basecacherepo);

result parse_tool_and_patch_name(int argc, char **argv, char **toolname, char **patchname, size_t init_search_pos);
