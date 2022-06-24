#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "def.h"
#include "utils/logutils.h"

static const char *usage_msg =
    "\tUsage: \n"
    "\tspm [command] [args] [options]\n"
    "\n\tCommands:\n"
    "\t\tsearch <tool> [kewords] - search a patch for a <tool> with given [keywords] (default command).\n"
    "\t\tload   <tool> <patch>   - download <patch> for given <tool> with patch name.\n"
    "\t\topen   <tool> <patch>   - show full description for <patch> of specified <tool>.\n"
    "\t\tapply  <tool> <patch>   - download and apply the <patch> for given <tool>.\n\n"
    "\t\tsync                    - synchonize local patches repository.\n"
	
    "\t\thelp    (--help/-h)     - to see this page.\n"
    "\t\tversion (--version/-v)  - to get version info.\n"

    "\n\tOptions:\n"
    "\t\topen: \n"
    "\t\t\t-b:  show the web page on suckless.org for given patch in "
    "browser.\n\n"
    "\t\tload: \n"
    "\t\t\t-a:  load and apply patch at once (the same as spm apply).\n\n"
    "\t\tsearch: \n"
    "\t\t\t-f:  show patch description for each patch found.\n\n"
    "\t\tapply: \n"
    "\t\t\t-f:  apply the patch directly from given file.\n";

void 
error(const char* err_format, ...) {
    va_list args;

    va_start(args, err_format);
    PRINT_TO_STDERR(err_format, args);
    va_end(args);
}

void bug(const char *filename, int linenum, const char *bug_msg, ...) {
    va_list args;

    fprintf(stderr, BUG_PREFIX, filename, linenum, "");
    vfprintf(stderr, bug_msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void print_usage(void) {
    puts(usage_msg);
}

void print_version(void) {
	puts("spm version: " SPM_VERSION);
}

void 
error_nolocalrepo(void) {
    error("Could not find base suckless repo. Try running 'spm sync'");
}
