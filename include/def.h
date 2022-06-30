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



#ifndef DEF_BASE
#define DEF_BASE

#define BASEREPO "/.cache/spmn/sites/"
#define PATCHESDIR ".suckless.org/patches/"
#define PATCHESP "/patches/"
#define DWM "dwm"
#define ST "st"
#define SURF "surf"
#define DWM_PATCHESDIR "dwm.suckless.org/patches/"
#define ST_PATCHESDIR "st.suckless.org/patches/"
#define SURF_PATCHESDIR "surf.suckless.org/patches/"
#define TOOLSDIR "tools.suckless.org/"
#define INDEXMD "/index.md"
#define DESCRIPTION_SECTION "Description"

#define GREP_BIN "/bin/grep"
#define DESCFILE "descfile.XXXXXX"
#define RESULTCACHE "result.XXXXXX"
#define DEVNULL "/dev/null"
#define ASCNULL '\0'

#define ERR_PREFIX "error: "
#define BUG_PREFIX "bug report: %s:%d: %s"
#define FATALERR_PREFIX "fatal error: "

#define LINEBUF 4096
#define PATHBUF LINEBUF
#define OPTTHREAD_COUNT 4
#define OPTWORK_AMOUNT 80
#define MIN_WORKAMOUNT 40

#define BUG_PREFIX_LEN sizeof(BUG_PREFIX)
#define ERR_PREFIX_LEN sizeof(ERR_PREFIX)
#define DESCFILE_LEN sizeof(DESCFILE)
#define DESCRIPTION_SECTION_LENGTH sizeof(DESCRIPTION_SECTION)

#define AVSEARCH_WORD_LEN 5
#define MAXSEARCH_LEN 512
#define ENTRYLEN 256
#define TOOLNAME_ARGPOS 2
#define KEYWORD_ARGPOS 2
#define PRINTABLE_DESCRIPTION_MAXLEN 5
#define CMD_LEN 8
#define CMD_ARGPOS 1

#define SPMN_VERSION "0.2"

#define MINI_ZIC
#include "zic.h"

DEFINE_ERROR(ERR_ENTRY_NOT_FOUND, 6)

DEFINE_ERROR(ERR_INVARG, 7)

#define KINDA_USE_ARG(ARG) (void)ARG;

#define KINDA_USE_2ARG(ARG1, ARG2)                                             \
    (void)ARG1;                                                                \
    (void)ARG2;

#define KINDA_USE_3ARG(ARG1, ARG2, ARG3)                                       \
    (void)ARG1;                                                                \
    (void)ARG2;                                                                \
    (void)ARG3;

//#define USE_MULTITHREADED

#endif
