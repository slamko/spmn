#include "def.h"
#include "utils/entry-utils.h"
#include "utils/logutils.h"
#include "utils/pathutils.h"
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static const char *const LESS_CMD = "/bin/less";
static const char *const XDG_OPEN = "/bin/xdg-open";

typedef result (*open_func)(const char *, const char *, const char *);

static result xdg_open(const char *url) {
  int openst;

  if (!url)
    ERROR(ERR_LOCAL)

  if (fork() == 0) {
    if (execl(XDG_OPEN, XDG_OPEN, url, (char *)NULL)) {
		perror("error");
		exit(1);
    }
  }

  UNWRAP_NEG(wait(&openst))
  return !!openst;
}

result openp(const char *toolname, const char *patch_name,
             const char *basecacherepo) {
  char *url = NULL;
  ZIC_RESULT_INIT()

  UNWRAP(build_patch_url(&url, toolname, patch_name, basecacherepo))

  TRY(xdg_open(url), DO_CLEAN(cl_url););

  CLEANUP(cl_url, free(url));
  ZIC_RETURN_RESULT()
}

static result print_pdescription(const char *toolname, const char *patch_name,
                          const char *basecacherepo) {
  char *pdir = NULL, *md = NULL;
  char *print_buf = NULL;
  FILE *patchf = NULL, *targetp = NULL;
  struct stat sp = {0};
  size_t patchn_len;
  ZIC_RESULT_INIT()

  patchn_len = strnlen(patch_name, ENTRYLEN);
  TRY(build_patch_dir(&pdir, toolname, patch_name, patchn_len, basecacherepo),
      DO_CLEAN(cl_pdir));

  TRY(spappend(&md, pdir, INDEXMD), DO_CLEAN(cl_pdir));

  TRY(stat(md, &sp), DO_CLEAN(cl_md));

  TRY_PTR(patchf = fopen(md, "r"), DO_CLEAN(cl_md));

  print_buf = calloc(sp.st_size, sizeof(*print_buf));
  TRY_PTR(print_buf, DO_CLEAN(cl_bufclose));

  TRY_PTR(targetp = popen(LESS_CMD, "w"), DO_CLEAN(cl_printbuf))
  
  TRY_UNWRAP_NEG(
      fread(print_buf, sizeof(*print_buf), sp.st_size, patchf), DO_CLEAN_ALL());
  TRY_UNWRAP_NEG (fwrite(print_buf, sizeof(*print_buf), sp.st_size, targetp), DO_CLEAN_ALL());

  CLEANUP_ALL(pclose(targetp));
  CLEANUP(cl_printbuf, free(print_buf));
  CLEANUP(cl_bufclose, fclose(patchf));
  CLEANUP(cl_md, free(md));
  CLEANUP(cl_pdir, free(pdir));

  ZIC_RETURN_RESULT()
}

result parse_open_args(int argc, char **argv, const char *basecacherepo) {
  open_func openf = NULL;
  int opt;
  char *toolname = NULL, *patchname = NULL;
  ZIC_RESULT_INIT();

  while ((opt = getopt(argc, argv, "b")) != -1) {
    switch (opt) {
    case 'b':
      openf = &openp;
      break;
    case '?':
      ERROR(ERR_INVARG)
      break;
    }
  }

  if (argc < 4)
    ERROR(ERR_INVARG)

  openf = &print_pdescription;

  TRY(parse_tool_and_patch_name(argc, argv, &toolname, &patchname, TOOLNAME_ARGPOS), DO_CLEAN_ALL());
  TRY(openf(toolname, patchname, basecacherepo),
      CATCH(ERR_SYS, HANDLE_SYS());

	  CATCH(ERR_LOCAL, bug(strerror(errno)); FAIL()));


  CLEANUP_ALL(
	  free(toolname);
	  free(patchname));
  ZIC_RETURN_RESULT()
}
