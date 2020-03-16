/*
 * regexutils.h
 */

#ifndef _REGEXUTILS_H
#define _REGEXUTILS_H

#include <pcre.h>
#include <string.h>

#define SETRCERROR(str)  strcpy(rcerror, #str); break;

#ifdef __cplusplus
extern "C" void translate_error(int, char *);
#else
void translate_error(int, char *);
#endif

#endif
