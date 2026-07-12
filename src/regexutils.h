/*
 * regexutils.h
 */

#ifndef _REGEXUTILS_H
#define _REGEXUTILS_H

#include "config.h"

#ifdef WITH_OLD_PCRE
#include <pcre.h>
#endif /* WITH_OLD_PCRE */
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>
#include <string.h>
#include <stdlib.h>
#ifndef __cplusplus
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" void translate_error(int use_old_ver, int rc, char * rcerror);
extern "C" int compare_ld (const void *, const void *);
extern "C" long double calc_std_deviation(const long double *, const int, const long double);
extern "C" void show_stat(long double * ld_diffs, int icnt);
extern "C" void strip_ignorecase_modifiers(const char *source, char *output, size_t out_size);
#else
void translate_error(int use_old_ver, int rc, char * rcerror);
void show_stat(long double * ld_diffs, int icnt);
void strip_ignorecase_modifiers(const char *source, char *output, size_t out_size);
#endif

#include <time.h>
#include <limits.h>

#define RESET       "\033[0m"
#define GREEN       "\033[32m"          /* Green */
#define BOLDGREEN   "\033[1m\033[32m"   /* Bold Green */

// https://gist.github.com/diabloneo/9619917#gistcomment-3364033
static inline void timespec_diff(const struct timespec *a, const struct timespec *b, struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

#endif
