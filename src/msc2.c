/*
 original sources are tree of mod_security2:

 https://github.com/SpiderLabs/ModSecurity/blob/v2/master/apache2/msc_pcre.c
 https://github.com/SpiderLabs/ModSecurity/blob/v2/master/apache2/re_operators.c

 strip_slashes() is from Apache2 source tree - server/core.c

 */

#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
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
    printf("\t-t T\tExpects a float value; if the (last) pcre_exec time is greather than this,\n");
    printf("\t    \tthe exit status of program will non-zero.\n");
    printf("\n");
}

/*
  In mod_security2, the Apache reads the config, not the module.
  The lines readed by ap_getword_conf(), declared in httpd.h,
  code in server/util.c.

  It's called from ap_build_config(), which declared in http_config.h,
  code in server/config.c.

  The root of the whole chain is ap_read_config(), which delared
  also in http_config.h, code also in server/config.c

  ap_getword_conf() splits the line into words, and call substring_conf()
  for all splitted token. substring_conf() code is in server/core.c, here
  is the modified version with name strip_slashes()

  This function does the same thing: removes the extra slashes:
  \\   -> \
  \\\\ -> \\
  \"   -> "

  Argument of @rx op usually quoted by " in modsecurity rules, so the
  variable `quote` is fixed.

  This routine is important, because this chain modifies the regular
  expression near the operator.
*/

static char *strip_slashes(const char *start, int len) {
    char *result = calloc(sizeof(char), len + 1);
    char *resp = result;
    char quote = '"';
    int i;

    for (i = 0; i < len; ++i) {
        if (start[i] == '\\' && (start[i + 1] == '\\'
                                 || (quote && start[i + 1] == quote)))
            *resp++ = start[++i];
        else
            *resp++ = start[i];
    }

    *resp++ = '\0';
    return result;
}

int main(int argc, char **argv) {
    pcre *re;
    pcre_extra *pce = NULL;
    const char *error;
    char pattern[FILESIZEMAX+1] = "";
    char *escaped_pattern;
    char subject[FILESIZEMAX+1] = "";
    int erroffset;
    int ovector[OVECCOUNT];
    int subject_length;
    int rc = 0, i, icnt = 1, pijit;
    char rcerror[100];
    char c;
    int ci;
    int use_jit = 0;
    int use_study = 1;
    int match_limit = 1000;
    int match_limit_recursion = 1000;
    float time_limit = 0.0;

    FILE *fp;
    const char * patternfile = NULL, * subjectfile = NULL;
    struct timeval tval_before, tval_after, tval_result;

    tval_result.tv_sec = 0;
    tval_result.tv_usec = 0;

    if (argc < 3) {
      showhelp(argv[0]);
      return EXIT_FAILURE;
    }

    while ((c = getopt (argc, argv, "hjm:r:sn:t:")) != -1) {
        switch (c) {
            case 'h':
                showhelp(argv[0]);
                return EXIT_SUCCESS;
#ifdef PCRE_CONFIG_JIT
            case 'j':
                use_jit = 1;
                break;
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT
            case 'm':
                match_limit = atoi(optarg);
                if (match_limit < 0 || icnt > 100000) {
                    fprintf(stderr, "Ohh... Try to pass for '-m' an integer between 0 and 100000\n");
                    return EXIT_FAILURE;
                }
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
            case 'r':
                match_limit_recursion = atoi(optarg);
                if (match_limit_recursion < 0 || icnt > 100000) {
                    fprintf(stderr, "Ohh... Try to pass for '-r' an integer between 0 and 100000\n");
                    return EXIT_FAILURE;
                }
#endif
            case 's':
                use_study = 0;
                break;
            case 'n':
                icnt = atoi(optarg);
                if (icnt <= 0 || icnt > 10) {
                    fprintf(stderr, "Ohh... Try to pass for '-n' an integer between 1 and 10\n");
                    return EXIT_FAILURE;
                }
                break;
            case 't':
                time_limit = atof(optarg);
                if (time_limit <= 0.0) {
                    fprintf(stderr, "Ohh... Time limit should be positive value.\n");
                    return EXIT_FAILURE;
                }
                break;
            case '?':
                if (optopt == 'n' || optopt == 'm' || optopt == 'r' || optopt == 't') {
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                }
                else if (isprint (optopt)) {
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                }
                else {
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                }
                return EXIT_FAILURE;
            default:
                abort ();
        }
    }

    for (i = optind; i < argc; i++) {
        if (patternfile == NULL) {
            patternfile = argv[i];
        }
        else {
            if (subjectfile == NULL) {
                subjectfile = argv[i];
            }
        }
    }

    if (patternfile == NULL || subjectfile == NULL) {
        showhelp(argv[0]);
        return EXIT_FAILURE;
    }

    // read pattern
    fp = fopen(patternfile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open file: %s\n", patternfile);
        return EXIT_FAILURE;
    }
    i = 0;
    while ((ci = fgetc(fp))) {
        if (ci == EOF || ci == '\n' || ci == '\r' || !(i < FILESIZEMAX)) {
            break;
        }
        pattern[i++] = ci;
    }
    fclose(fp);
    if (i == FILESIZEMAX && ci != EOF) {
        fprintf (stderr, "File too long: %s\n", patternfile);
        return EXIT_FAILURE;
    }

    // remove extra slashes
    escaped_pattern = strip_slashes(pattern, strlen(pattern));

    // read subject
    fp = fopen(subjectfile, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open file: %s\n", subjectfile);
        return EXIT_FAILURE;
    }
    i = 0;
    while ((ci = fgetc(fp)) != EOF && i < FILESIZEMAX) {
        subject[i++] = ci;
    }
    subject_length = (int)strlen(subject);
    fclose(fp);
    if (i == FILESIZEMAX && ci != EOF) {
        fprintf (stderr, "File too long: %s\n", subjectfile);
        return EXIT_FAILURE;
    }

    re = pcre_compile(
        escaped_pattern,      /* the pattern */
        PCRE_DOTALL | PCRE_DOLLAR_ENDONLY, /* options from re_operators */
        &error,               /* for error message */
        &erroffset,           /* for error offset */
        NULL                  /* use default character tables */
    );

    if (re == NULL) {
        fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, error);
        fprintf(stderr, "Stripped regex: '%s'\n", escaped_pattern);
        return EXIT_FAILURE;
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
        pce = calloc(1, sizeof(pcre_extra));
        if (pce == NULL) {
            return EXIT_FAILURE;
        }
    }

#ifdef PCRE_EXTRA_MATCH_LIMIT
    if (match_limit > 0) {
        pce->match_limit = match_limit;
        pce->flags |= PCRE_EXTRA_MATCH_LIMIT;
    }
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
    if (match_limit_recursion > 0) {
        pce->match_limit_recursion = match_limit_recursion;
        pce->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
    }
#endif

#ifdef PCRE_CONFIG_JIT
    if (use_jit == 1) {
        rc = pcre_fullinfo(re, pce, PCRE_INFO_JIT, &pijit);
        if ((rc != 0) || (pijit != 1)) {
            fprintf(stderr, "Regex does not support JIT (%d)\n", rc);
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
        translate_error(rc, rcerror);
        printf("%s - time elapsed: %ld.%06ld, match value: %s\n", patternfile, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, rcerror);
    }

    pcre_free(re);
    if (pce != NULL) {
        pcre_free(pce);
    }
    if (escaped_pattern != NULL) {
        free(escaped_pattern);
    }

    if (time_limit > 0.0) {
        if (((double)tval_result.tv_sec + ((double)(tval_result.tv_usec))/1000000.0) > time_limit) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
