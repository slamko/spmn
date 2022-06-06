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

static const char *const XDG_OPEN = "/bin/xdg-open";

typedef result (*open_func)(const char *, const char *, const char *);

static result xdg_open(const char *url) {
  int openst;

  if (!url)
    ERROR(ERR_LOCAL)

  if (fork() == 0) {
    if (execl(XDG_OPEN, XDG_OPEN, url, (char *)NULL)) {
      EPERROR()
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

result print_pdescription(const char *toolname, const char *patch_name,
                          const char *basecacherepo) {
  char *pdir = NULL, *md = NULL;
  char *print_buf = NULL;
  FILE *patchf = NULL;
  struct stat sp = {0};
  size_t patchn_len;
  ZIC_RESULT_INIT()

  patchn_len = strnlen(patch_name, ENTRYLEN);
  TRY(build_patch_dir(&pdir, toolname, patch_name, patchn_len, basecacherepo),
      DO_CLEAN(cl_pdir))

  TRY(spappend(&md, pdir, INDEXMD), DO_CLEAN(cl_pdir))

  TRY(stat(md, &sp), DO_CLEAN(cl_pdir))

  patchf = fopen(md, "r");
  UNWRAP_PTR_LABEL(patchf, cl_pdir)

  print_buf = calloc(sp.st_size, sizeof(*print_buf));
  UNWRAP_PTR_LABEL(print_buf, cl_bufclose)

  UNWRAP_NEG_CLEANUP(fread(print_buf, sizeof(*print_buf), sp.st_size, patchf))
  UNWRAP_NEG_CLEANUP(write(STDOUT_FILENO, print_buf, sp.st_size))

  CLEANUP(cl_printbuf, free(print_buf));
  CLEANUP(cl_bufclose, fclose(patchf));
  CLEANUP(cl_pdir, free(pdir));

  ZIC_RETURN_RESULT()
}

result parse_open_args(int argc, char **argv, const char *basecacherepo) {
  ZIC_RESULT_INIT()
  open_func openf = NULL;
  int opt;

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

  TRY(openf(argv[2], argv[3], basecacherepo),
      CATCH(ERR_SYS, HANDLE_SYS())

      CATCH(ERR_LOCAL, bug(strerror(errno)); FAIL()))

  ZIC_RETURN_RESULT()
}
