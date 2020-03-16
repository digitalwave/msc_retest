/*
 gcc -Wall demo.c -lpcre -o pcredemo

 original source: https://github.com/vmg/pcre/blob/master/pcredemo.c

 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "regexutils.h"

#define OVECCOUNT 30    /* should be a multiple of 3 */
#define FILESIZEMAX 131072

void showhelp(char * name) {
    printf("Use: %s [OPTIONS] patternfile subjectfile\n", name);
    printf("OPTIONS:\n");
    printf("\t-h\tThis help\n");
#ifdef PCRE_CONFIG_JIT
    printf("\t-j\tUse JIT (if available). Default is NO.\n");
#endif
    printf("\t-s\tIgnore using of pcre_study function. Default is NO.\n");
    printf("\t-n N\titerate pcre_regex as Nth times. Default value is 1.\n");
#ifdef PCRE_EXTRA_MATCH_LIMIT
    printf("\t-m M\tSet value M for the pcre_match_limit for pcre_extra. Default value is 1000.\n");
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
    printf("\t-r R\tSet value R for the pcre_match_limit_recursion for pcre_extra. Default value is 1000.\n");
#endif
    printf("\n");
}

void strip_slashes(char * str) {
    char tstr[FILESIZEMAX+1];
    int i = 0;
    char *c;
    int last_is_slash = 0;

    c = str;
    while(*c != '\0') {
        if (last_is_slash == 0) {
            if (*c == '\\') {
                last_is_slash = 1;
            }
            tstr[i++] = *c;
        }
        else {
            last_is_slash = 0;
            if (*c != '\\') {
                tstr[i++] = *c;
            }
        }
        c++;
    }
    tstr[i] = '\0';

    strcpy(str, tstr);
}

int main(int argc, char **argv) {
    pcre *re;
    pcre_extra *pce = NULL;
    const char *error;
    char pattern[FILESIZEMAX+1];
    char subject[FILESIZEMAX+1];
    int erroffset;
    int ovector[OVECCOUNT];
    int subject_length;
    int rc = 0, i, icnt = 1, pijit;
    char rcerror[100], rcmatch[50];
    int use_jit = 0;
    int use_study = 1;
    int match_limit = 1000;
    int match_limit_recursion = 1000;


    FILE *fp;
    const char * patternfile = NULL, * subjectfile = NULL;
    struct timeval tval_before, tval_after, tval_result;
    struct timeval tval_results[10];

    if (argc < 3) {
      showhelp(argv[0]);
      return 1;
    }

    for(i = 1; i < argc; i++) {
        if (i == 1 && strcmp(argv[i], "-h") == 0) {
            showhelp(argv[0]);
            return 0;
        }
        if (strcmp(argv[i], "-n") == 0) {
            icnt = atoi(argv[++i]);
            if (icnt <= 0 || icnt > 10) {
                printf("Ohh... Try to pass for '-n' an integer between 1 and 10\n");
                return -1;
            }
        }
        else if (strcmp(argv[i], "-s") == 0) {
            use_study = 0;
        }
#ifdef PCRE_CONFIG_JIT
        else if (strcmp(argv[i], "-j") == 0) {
            use_jit = 1;
        }
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT
        else if (strcmp(argv[i], "-m") == 0) {
            match_limit = atoi(argv[++i]);
            if (match_limit < 0 || icnt > 100000) {
                printf("Ohh... Try to pass for '-m' an integer between 0 and 100000\n");
                return -1;
            }
        }
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
        else if (strcmp(argv[i], "-r") == 0) {
            match_limit_recursion = atoi(argv[++i]);
            if (match_limit_recursion < 0 || icnt > 100000) {
                printf("Ohh... Try to pass for '-r' an integer between 0 and 100000\n");
                return -1;
            }
        }
#endif
        else {
            if (patternfile == NULL) {
                patternfile = argv[i];
            }
            else {
                if (subjectfile == NULL) {
                    subjectfile = argv[i];
                }
            }
        }
    }

    // read pattern
    fp = fopen(patternfile, "r");
    if (fp == NULL) {
        printf("Can't open file: %s\n", patternfile);
        return -1;
    }
    fgets(pattern, FILESIZEMAX, fp);
    fclose(fp);

    // remove extra slashes
    strip_slashes(pattern);

    // read subject
    fp = fopen(subjectfile, "r");
    if (fp == NULL) {
        printf("Can't open file: %s\n", subjectfile);
        return -1;
    }
    fgets(subject, FILESIZEMAX, fp);
    subject_length = (int)strlen(subject);
    fclose(fp);

    re = pcre_compile(
        pattern,              /* the pattern */
        PCRE_DOTALL | PCRE_DOLLAR_ENDONLY, /* options from re_operators */
        &error,               /* for error message */
        &erroffset,           /* for error offset */
        NULL                /* use default character tables */
    );

    if (re == NULL) {
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        printf("Stripped regex: '%s'\n", pattern);
        return 1;
    }

#ifdef PCRE_CONFIG_JIT
    if (use_jit == 1) {
        pce = pcre_study(re, PCRE_STUDY_JIT_COMPILE, &error);
    }
    else {
        if (use_study == 1) {
            pce = pcre_study(re, 0, &error);
        }
    }
#else
    if (use_study == 1) {
        pce = pcre_study(re, 0, &error);
    }
#endif

    if (pce == NULL) {
        pce = malloc(sizeof(pcre_extra));
        if (pce == NULL) {
            return -1;
        }
        memset(pce, 0, sizeof(pcre_extra));
    }

#ifdef PCRE_EXTRA_MATCH_LIMIT
    if (match_limit > 0) {
        pce->match_limit = match_limit;
        pce->flags |= PCRE_EXTRA_MATCH_LIMIT;
    }
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
    if (match_limit_recursion > 0) {
        pce->match_limit = match_limit_recursion;
        pce->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    }
#endif

#ifdef PCRE_CONFIG_JIT
    if (use_jit == 1) {
        rc = pcre_fullinfo(re, pce, PCRE_INFO_JIT, &pijit);
        if ((rc != 0) || (pijit != 1)) {
            printf("Regex does not support JIT (%d)\n", rc);
        }
    }
#endif

    for(i=0; i<icnt; i++) {
        gettimeofday(&tval_before, NULL);
        rc = pcre_exec(
            re,                   /* the compiled pattern */
            pce,                  /* no extra data - we didn't study the pattern */
            subject,              /* the subject string */
            subject_length,       /* the length of the subject */
            0,                    /* start at offset 0 in the subject */
            0,                    /* default options */
            ovector,              /* output vector for substring information */
            OVECCOUNT             /* number of elements in the output vector */
        );
        gettimeofday(&tval_after, NULL);
        timersub(&tval_after, &tval_before, &tval_result);
        if (rc <= 0) {
            strcpy(rcmatch, "not matched");
        }
        else {
            strcpy(rcmatch, "matched");
        }
        translate_error(rc, rcerror);
        printf("%s - time elapsed: %ld.%06ld, matched: %s, regex error: %s\n", patternfile, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, rcmatch, rcerror);
        memcpy(&tval_results[i], &tval_result, sizeof(struct timeval));
    }

    pcre_free(re);
    if (pce != NULL) {
        pcre_free(pce);
    }
    return rc;
}
