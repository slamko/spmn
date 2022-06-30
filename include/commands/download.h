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


#include <stdbool.h>
#include "zic.h"

struct load_args {
	bool apply;
};

result loadp(const char *toolname, const char *patchname,
             const char *basecacherepo, struct load_args flags);

int parse_load_args(int argc, char **argv, const char *basecacherepo);
