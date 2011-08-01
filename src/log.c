/*
 * vim:ts=4:sw=4:expandtab
 *
 * i3 - an improved dynamic tiling window manager
 *
 * © 2009-2010 Michael Stapelberg and contributors
 *
 * See file LICENSE for license information.
 *
 * src/log.c: handles the setting of loglevels, contains the logging functions.
 *
 */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>

#include "util.h"
#include "log.h"

/* loglevels.h is autogenerated at make time */
#include "loglevels.h"

static uint64_t loglevel = 0;
static bool verbose = true;
static FILE *errorfile;
char *errorfilename;

/*
 * Initializes logging by creating an error logfile in /tmp (or
 * XDG_RUNTIME_DIR, see get_process_filename()).
 *
 */
void init_logging() {
    errorfilename = get_process_filename("errorlog");
    if (errorfilename == NULL) {
        ELOG("Could not initialize errorlog\n");
        return;
    }

    errorfile = fopen(errorfilename, "w");
}

/*
 * Set verbosity of i3. If verbose is set to true, informative messages will
 * be printed to stdout. If verbose is set to false, only errors will be
 * printed.
 *
 */
void set_verbosity(bool _verbose) {
    verbose = _verbose;
}

/*
 * Enables the given loglevel.
 *
 */
void add_loglevel(const char *level) {
    /* Handle the special loglevel "all" */
    if (strcasecmp(level, "all") == 0) {
        loglevel = UINT64_MAX;
        return;
    }

    for (int i = 0; i < sizeof(loglevels) / sizeof(char*); i++) {
        if (strcasecmp(loglevels[i], level) != 0)
            continue;

        /* The position in the array (plus one) is the amount of times
         * which we need to shift 1 to the left to get our bitmask for
         * the specific loglevel. */
        loglevel |= (1 << (i+1));
        break;
    }
}

/*
 * Logs the given message to stdout while prefixing the current time to it.
 * This is to be called by *LOG() which includes filename/linenumber/function.
 *
 */
void vlog(char *fmt, va_list args) {
    char timebuf[64];

    /* Get current time */
    time_t t = time(NULL);
    /* Convert time to local time (determined by the locale) */
    struct tm *tmp = localtime(&t);
    /* Generate time prefix */
    strftime(timebuf, sizeof(timebuf), "%x %X - ", tmp);
#ifdef DEBUG_TIMING
    struct timeval tv;
    gettimeofday(&tv, NULL);
    printf("%s%d.%d - ", timebuf, tv.tv_sec, tv.tv_usec);
#else
    printf("%s", timebuf);
#endif
    vprintf(fmt, args);
}

/*
 * Logs the given message to stdout while prefixing the current time to it,
 * but only if verbose mode is activated.
 *
 */
void verboselog(char *fmt, ...) {
    va_list args;

    if (!verbose)
        return;

    va_start(args, fmt);
    vlog(fmt, args);
    va_end(args);
}

/*
 * Logs the given message to stdout while prefixing the current time to it.
 *
 */
void errorlog(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vlog(fmt, args);
    va_end(args);

    /* also log to the error logfile, if opened */
    va_start(args, fmt);
    vfprintf(errorfile, fmt, args);
    fflush(errorfile);
    va_end(args);
}

/*
 * Logs the given message to stdout while prefixing the current time to it,
 * but only if the corresponding debug loglevel was activated.
 * This is to be called by DLOG() which includes filename/linenumber
 *
 */
void debuglog(uint64_t lev, char *fmt, ...) {
    va_list args;

    if ((loglevel & lev) == 0)
        return;

    va_start(args, fmt);
    vlog(fmt, args);
    va_end(args);
}