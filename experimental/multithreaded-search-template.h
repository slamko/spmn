/*
static size_t calc_threadcount(int entrycnt) {
  int optentrycnt = OPTWORK_AMOUNT * OPTTHREAD_COUNT;
  if (entrycnt > optentrycnt) {
    return OPTTHREAD_COUNT;
  } else {
          float actualthrcount = (double)entrycnt / (double)OPTWORK_AMOUNT;
    int nearest_thrcount = floor((double)actualthrcount);
    int workdiff = (actualthrcount * 100) - (nearest_thrcount * 100);

    if (workdiff < MIN_WORKAMOUNT)
      return nearest_thrcount;

    return nearest_thrcount + 1;
  }
}
*/




/*
int
run_multithreaded(char *patchdir, searchsyms *searchargs, const int entrycnt) {
    pthread_t *threadpool;
    pthread_mutex_t fmutex;
    lookupthread_args *thargs;
    FILE *rescache;
    int rescachefd;
    size_t thpoolsize, thcount;
    char rescachename[] = RESULTCACHE, resc;
    int res = EXIT_FAILURE;

    rescachefd = mkstemp(rescachename);
    if (rescachefd == -1) {
        EPERROR();
        return res;
    }

    thcount = calc_threadcount(entrycnt);
    thpoolsize = sizeof(*threadpool) * thcount;

    threadpool = calloc(1, thpoolsize);
    if (!threadpool) {
        perrfatal();
        goto cleanup;
    }

    thargs = malloc(sizeof(*thargs) * thcount);
    if (!thargs) {
        perrfatal();
        goto cleanup;
    }

    pthread_mutex_init(&fmutex, NULL);

    for (size_t tid = 0; tid < thcount; tid++) {
        if (setup_threadargs(thargs, tid, thcount, entrycnt, rescachefd,
                            searchargs, patchdir, &fmutex)) {
            return res;
        }
        pthread_create(threadpool + tid, NULL, &search_entry, thargs + tid);
    }

    for (size_t tid = 0; tid < thcount; tid++) {
        pthread_join(threadpool[tid], NULL);
        res |= thargs[tid].result;
        cleanup_threadargs(thargs + tid);
    }

    UNWRAP_L (pthread_mutex_destroy(&fmutex))
    free(threadpool);
    TRY(rescache = fdopen(rescachefd, "r"))
    WITH(
        fcache_error();
        res = 1;
        goto cleanup)

    while ((resc = fgetc(rescache)) != EOF) {
        fputc(resc, stdout);
    }

cleanup:
    fclose(rescache);
    remove(rescachename);
    return !!res;
    return 0;
}*/
