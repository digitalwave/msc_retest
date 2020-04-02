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
#else
void translate_error(int, char *);
#endif

#define RESET       "\033[0m"
#define GREEN       "\033[32m"          /* Green */
#define BOLDGREEN   "\033[1m\033[32m"   /* Bold Green */

#endif
