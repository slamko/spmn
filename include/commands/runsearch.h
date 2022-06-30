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


#ifndef RUN_SEARCH_DEF
#define RUN_SEARCH_DEF

#define SEARCH_CMD "search"

#include "commands/search.h"

int run_search(char *patchdir, searchsyms *searchsyms);

int parse_search_args(int argc, char **argv, const char *basecacherepo);
#endif
