
#define BASEREPO "/.cache/sise/sites/"
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

#define LINEBUF 4096
#define PATHBUF LINEBUF
#define DESCRIPTION_SECTION "Description" 
#define DESCRIPTION_SECTION_LENGTH 12
#define GREP_BIN "/bin/grep"
#define DESCFILE "descfile.XXXXXX"
#define DESKFILE_LEN 15
#define RESULTCACHE "result.XXXXXX"
#define DEVNULL "/dev/null"
#define ASCNULL '\0'

#define OPTTHREAD_COUNT 4
#define OPTWORK_AMOUNT 80
#define MIN_WORKAMOUNT 40
#define ERRPREFIX_LEN 7
#define AVSEARCH_WORD_LEN 5
#define MAXSEARCH_LEN 512
#define ENTRYLEN 256
#define TOOLNAME_ARGPOS 1
#define KEYWORD_ARGPOS 2
#define PRINTABLE_DESCRIPTION_MAXLEN 5

//#define USE_MULTITHREADED

void 
empty() {}

#define TRY(EXP) if (!(EXP)) 
#define WITH(HANDLE) { HANDLE; return EXIT_FAILURE; }
#define EPERROR() error(strerror(errno));
#define OK(RES) RES == 0
