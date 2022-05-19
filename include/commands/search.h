#include <pthread.h>

typedef struct searchargs {
    char **words; 
    char *searchstr;
    size_t wordcount;
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