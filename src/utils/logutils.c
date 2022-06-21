#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "def.h"
#include "utils/logutils.h"

/*static const char *usage_msg_short = "Usage: \n"
	"\tspm [search] <tool> <keywords>\n"
	"\tspm <sync>\n"
	"\tspm <open> <tool> <patch> [-b]\n"
	"\tspm <load> <tool> <patch> [-a]\n";
*/
static const char *usage_msg =
	"\tUsage: \n"
	"\tspm [command] [args] [options]\n"
	"\n\tCommands:\n"
	"\t\tsearch - search for patch with given keywords (default command).\n"
	"\t\tsync   - synchonize local patches repository.\n"
	"\t\tload   - download patch for given tool with patch name.\n"
	"\t\topen   - show full patch description.\n"
	"\t\tapply  - download and apply the patch.\n"
	"\n\tOptions:\n"
	"\t\topen: \n"
	"\t\t\t-b:  show the web page on suckless.org for given patch in browser.\n\n"
	"\t\tload: \n"
	"\t\t\t-a:  load and apply patch at once (the same as spm apply)"
	"\t\tsearch: \n"
	"\t\t\t-f:  show patch description for each patch found.";
	
void 
error(const char* err_format, ...) {
    va_list args;

    va_start(args, err_format);
    FORMAT_ERR(err_format, args)
    va_end(args);
}

void bug(const char *filename, int linenum, const char *bug_msg, ...) {
    va_list args;

    fprintf(stderr, BUG_PREFIX, filename, linenum, "");
    vfprintf(stderr, bug_msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void
print_usage(void) {
    puts(usage_msg);
}

void 
error_nolocalrepo(void) {
    error("Could not find base suckless repo. Try running 'spm sync'");
}
