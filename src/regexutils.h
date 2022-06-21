/*
 * regexutils.h
 */

#ifndef _REGEXUTILS_H
#define _REGEXUTILS_H

#include "config.h"

#include <pcre.h>
#ifdef HAVE_PCRE2
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#endif /* HAVE_PCRE2 */
#include <string.h>
#include <stdlib.h>
#ifndef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" void translate_error(int, char *);
extern "C" int compare_ld (const void *, const void *);
extern "C" long double calc_std_deviation(const long double *, const int, const long double);
extern "C" void show_stat(long double *, int);
#else
void translate_error(int, char *);
int compare_ld (const void *, const void *);
long double calc_std_deviation(const long double *, const int, const long double);
void show_stat(long double *, int);
#endif

#include <time.h>
#include <limits.h>

#define RESET       "\033[0m"
#define GREEN       "\033[32m"          /* Green */
#define BOLDGREEN   "\033[1m\033[32m"   /* Bold Green */

// https://gist.github.com/diabloneo/9619917#gistcomment-3364033
static inline void timespec_diff(struct timespec *a, struct timespec *b, struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

/*
 * https://gist.github.com/tinselcity/acdf11737b5e2cc5ac9f14bd96310a3a
 */
#ifndef NDBG_OUTPUT
#define NDBG_OUTPUT(...) \
        do { \
                fprintf(stdout, __VA_ARGS__); \
                fflush(stdout); \
        } while(0)
#endif

#define _DISPLAY_PCRE_PROP(_what) do { \
                int l_opt; \
                pcre_fullinfo(re, pce, _what, &l_opt); \
                NDBG_OUTPUT("%25s: %d\n", #_what, l_opt); \
        } while(0)
#define _DISPLAY_PCRE_PROP_U(_what) do { \
                uint32_t l_opt; \
                pcre_fullinfo(re, pce, _what, &l_opt); \
                NDBG_OUTPUT("%25s: %u\n", #_what, l_opt); \
        } while(0)
#define _DISPLAY_PCRE_PROP_UL(_what) do { \
                size_t l_opt; \
                pcre_fullinfo(re, pce, _what, &l_opt); \
                NDBG_OUTPUT("%25s: %lu\n", #_what, l_opt); \
        } while(0)

#endif
