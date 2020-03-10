/*
 gcc -Wall demo.c -lpcre -o pcredemo

 original source: https://github.com/vmg/pcre/blob/master/pcredemo.c

 */

#include <stdio.h>
#include <string.h>
#include <pcre.h>
#include <sys/time.h>

#define OVECCOUNT 900    /* should be a multiple of 3 */
#define FILESIZEMAX 131072

void showhelp(char * name) {
    printf("Use: %s [OPTIONS] patternfile subjectfile\n", name);
    printf("OPTIONS:\n");
    printf("\t-h\tThis help\n");
#ifdef PCRE_CONFIG_JIT
    printf("\t-j\tUse JIT (if available)\n");
#endif
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
    int rc, i;
    int use_jit = 0;

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
#ifdef PCRE_CONFIG_JIT
        if (strcmp(argv[i], "-j") == 0) {
            use_jit = 1;
        }
        else {
#endif
            if (patternfile == NULL) {
                patternfile = argv[i];
            }
            else {
                if (subjectfile == NULL) {
                    subjectfile = argv[i];
                }
            }
#ifdef PCRE_CONFIG_JIT
        }
#endif
    }

    // read pattern
    fp = fopen(patternfile, "r");
    if (fp == NULL) {
        printf("Can't open file: %s\n", patternfile);
        return -1;
    }
    fgets(pattern, FILESIZEMAX, fp);
    fclose(fp);

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
        0,                    /* default options */
        &error,               /* for error message */
        &erroffset,           /* for error offset */
        NULL                /* use default character tables */
    );

    if (re == NULL) {
        printf("PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return 1;
    }

#ifdef PCRE_CONFIG_JIT
    if (use_jit == 1) {
        pce = pcre_study(re, PCRE_STUDY_JIT_COMPILE, &error);
    }
#endif

    for(i=0; i<10; i++) {
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
        printf("Time elapsed: %ld.%06ld, rc: %d\n", (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, rc);
        memcpy(&tval_results[i], &tval_result, sizeof(struct timeval));
    }

    pcre_free(re);
    if (pce != NULL) {
        pcre_free(pce);
    }
    return 0;
}
