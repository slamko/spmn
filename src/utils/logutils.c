/*
Copyright 2022 Viacheslav Chepelyk-Kozhin.

This file is part of Suckless Patch Manager (spmn).
Spmn is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.
Spmn is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
You should have received a copy of the GNU General Public License along with
spmn. If not, see <https://www.gnu.org/licenses/>.
*/


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "def.h"
#include "utils/logutils.h"
#include "zic.h"

static const char *usage_msg =
    "\tUsage: \n"
    "\tspmn [command] [args] [options]\n"
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
    "\t\t\t-a:  load and apply patch at once (the same as spmn apply).\n\n"
    "\t\tsearch: \n"
    "\t\t\t-f:  show patch description for each patch found.\n\n"
    "\t\tapply: \n"
    "\t\t\t-f:  apply the patch directly from given file.\n";

void 
error(const char* err_format, ...) {
    va_list args;

    va_start(args, err_format);
    PRINT_ERR(err_format, args);
    va_end(args);
}

void bug(const char *filename, int linenum, const char *bug_msg, ...) {
    va_list args;

	va_start(args, bug_msg);
    PRINT_TO_STDERR(BUG_PREFIX, filename, linenum, "");
    PRINT_ERR(bug_msg, args);
    va_end(args);
}

void print_usage(void) {
    puts(usage_msg);
}

void print_version(void) {
	puts("spmn version: " SPMN_VERSION);
}
