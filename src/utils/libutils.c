#include <stdlib.h>

int pcalloc(void **cres, size_t nmemb, size_t msize) {
    *cres = calloc(nmemb, msize);
}