#include <pthread.h>
#include "def.h"
#include "stdbool.h"

struct search_flags {
	bool print_full_patch;
};

typedef struct searchargs {
    char **words; 
    size_t wordcount;
	struct search_flags s_flags;
} searchsyms;

struct threadargs {
    char *descfname;
    char *patchdir;
    int descffd;
    int outfd;
    int startpoint;
    int endpoint;
    result result;
    pthread_mutex_t *mutex;
    searchsyms *searchargs;
};

typedef struct threadargs lookupthread_args;

void *search_entry(void *thread_args);

result lookup_entries(const lookupthread_args *args);
