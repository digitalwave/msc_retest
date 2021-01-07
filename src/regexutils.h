/*
 * regexutils.h
 */

#ifndef _REGEXUTILS_H
#define _REGEXUTILS_H

#include <pcre.h>
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

#endif
