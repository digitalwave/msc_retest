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
#include <limits.h>
#include "regexutils.h"

#define OVECCOUNT 30    /* should be a multiple of 3 */
#define FILESIZEMAX 131072

void showhelp(char * name) {
    printf("Use: %s [OPTIONS] patternfile subjectfile\n\n", name);
    printf("You can pass subject through stdin, just give the '-' as subjectfile or leave it.\n\n");
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
    printf("\t-t T\tExpects a float value; if the (last) pcre_exec time is greater than this,\n");
    printf("\t    \tthe exit status of program will non-zero.\n");
    printf("\t-d  \tShow detailed information.\n");
    printf("\n");
}

/*
 * this function is just print out the underlined label
 * FOO ->
 * FOO:
 * ====
 */
void debuglabel(int debuglevel, char * label) {
    if (debuglevel == 1) {
        int len = (int)strlen(label)+1; // +1 -> append ":"
        char * line = calloc(sizeof(char), len+2);
        if (line == NULL) {
            fprintf(stderr, "Failed to calloc temp str.");
        }
        for(int i = 0; i < len; i++) {
            strcat(line, "=");
        }
        line[len+1] = '\0';
        printf("\n%s:\n%s\n", label, line);
        free(line);
    }
}

/*
 * show debug info with string argument
 */
void debugstr(int debuglevel, char * label, char * value) {
    if (debuglevel == 1) {
        debuglabel(debuglevel, label);
        printf("%s\n", value);
    }
}

/*
 * show debug info with int argument
 */
void debugint(int debuglevel, char * label, int value) {
    if (debuglevel == 1) {
        debuglabel(debuglevel, label);
        printf("%d\n", value);
    }
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
    int debuglevel = 0;
    char stdinname[] = "-";

    FILE *fp;
    const char * patternfile = NULL, * subjectfile = NULL;
    //struct timeval tval_before, tval_after, tval_result;
    struct timespec ts_before, ts_after, ts_diff;
    long double *ld_diffs = NULL;
    long double ld_minval, ld_maxval;
    long double ld_sums, ld_mean, ld_med;

    //tval_result.tv_sec = 0;
    //tval_result.tv_usec = 0;
    ld_sums         = 0.0;
    ld_med          = 0.0;
    ld_minval       = 0.0;
    ld_maxval       = 0.0;

    if (argc < 2) {
      showhelp(argv[0]);
      return EXIT_FAILURE;
    }

    while ((c = getopt (argc, argv, "hjm:r:sn:t:d")) != -1) {
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
                if (icnt <= 0 || icnt > INT_MAX) {
                    fprintf(stderr, "Ohh... Try to pass for '-n' an integer between 1 and %d\n", INT_MAX);
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
            case 'd':
                debuglevel = 1;
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

    if (subjectfile == NULL) {
        subjectfile = stdinname;
    }

    if (patternfile == NULL) {
        showhelp(argv[0]);
        return EXIT_FAILURE;
    }

    if (icnt > 1) {
        ld_diffs = calloc(icnt, sizeof(long double));
        if (ld_diffs == NULL) {
            fprintf(stderr, "Can't allocate memory for time diff array\n");
            return EXIT_FAILURE;
        }
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
    //   if filename was given
    if (strcmp(subjectfile, "-") != 0) {
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
    }
    //   or read from stdin
    else {
        i = 0;
        while ((ci = getchar()) != '\n' && ci != EOF && i < FILESIZEMAX) {
            subject[i++] = ci;
        }
        subject_length = (int)strlen(subject);
    }
    if (i == FILESIZEMAX && ci != EOF) {
        fprintf (stderr, "File too long: %s\n", subjectfile);
        return EXIT_FAILURE;
    }

    // show debug informations
    debugstr(debuglevel, "RAW pattern", pattern);
    debugstr(debuglevel, "ESCAPED pattern", escaped_pattern);
    debugstr(debuglevel, "SUBJECT", subject);

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
        debugstr(debuglevel, "JIT", "available and enabled");
        debugstr(debuglevel, "STUDY", "enabled");
    }
    else {
        if (use_study == 1) {
            pce = pcre_study(re, 0, &error);
            debugstr(debuglevel, "JIT", "available but disabled");
            debugstr(debuglevel, "STUDY", "enabled");
        }
    }
#else
    if (use_study == 1) {
        pce = pcre_study(re, 0, &error);
        debugstr(debuglevel, "STUDY", "enabled");
    }
    else {
        debugstr(debuglevel, "STUDY", "disabled");
    }
#endif

    if (pce == NULL) {
        pce = calloc(1, sizeof(pcre_extra));
        if (pce == NULL) {
            fprintf(stderr, "Calloc failure for `pce`.\n");
            return EXIT_FAILURE;
        }
    }

#ifdef PCRE_EXTRA_MATCH_LIMIT
    if (match_limit > 0) {
        pce->match_limit = match_limit;
        pce->flags |= PCRE_EXTRA_MATCH_LIMIT;
        debugint(debuglevel, "MATCH LIMIT", match_limit);
    }
#endif
#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
    if (match_limit_recursion > 0) {
        pce->match_limit_recursion = match_limit_recursion;
        pce->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
        debugint(debuglevel, "MATCH LIMIT RECURSION", match_limit_recursion);
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

    rc = 0; // initialize for debug level...

    for(i=0; i<icnt; i++) {
        ts_diff.tv_sec  = 0;
        ts_diff.tv_nsec = 0;

        //gettimeofday(&tval_before, NULL);
        clock_gettime(CLOCK_REALTIME, &ts_before);
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
        //gettimeofday(&tval_after, NULL);
        clock_gettime(CLOCK_REALTIME, &ts_after);
        //timersub(&tval_after, &tval_before, &tval_result);
        timespec_diff(&ts_after, &ts_before, &ts_diff);
        translate_error(rc, rcerror);
        debuglabel(debuglevel, "RESULT");
        // printf("%s - time elapsed: %ld.%06ld, match value: %s\n", patternfile, (long int)tval_result.tv_sec, (long int)tval_result.tv_usec, rcerror);
        printf("%s - time elapsed: %ld.%09ld, match value: %s\n", patternfile, (long int)ts_diff.tv_sec, (long int)ts_diff.tv_nsec, rcerror);
        if (icnt > 1) {
            ld_diffs[i] = ts_diff.tv_sec + (ts_diff.tv_nsec/1000000000.0);

            ld_sums += ld_diffs[i];
            // set minval
            if (i == 0 || ld_diffs[i] < ld_minval) {
                ld_minval = ld_diffs[i];
            }
            // set maxval
            if (ld_diffs[i] > ld_maxval) {
                ld_maxval = ld_diffs[i];
            }
        }
    }

    if (icnt > 1) {
        ld_mean = ld_sums / (double)icnt;

        qsort(ld_diffs, icnt, sizeof(long double), compare_ld);

        if (icnt%2 == 1) {
           ld_med = ld_diffs[(icnt/2)+1];
        }
        else {
            long double ldtemp[2];
            ldtemp[0] = ld_diffs[(icnt/2)];
            ldtemp[1] = ld_diffs[(icnt/2)+1];
            ld_med = ldtemp[0] + ((ldtemp[1]-ldtemp[0])/2.0);
        }

        printf("Num of values: %d\n", icnt);
        printf("         Mean: %013.9Lf\n", ld_mean);
        printf("       Median: %013.9Lf\n", ld_med);
        printf("          Min: %013.9Lf\n", ld_minval);
        printf("          Max: %013.9Lf\n", ld_maxval);
        printf("        Range: %013.9Lf\n", ld_maxval - ld_minval);
        printf("Std deviation: %013.9Lf\n", calc_std_deviation(ld_diffs, icnt, ld_mean));
    }

    // show captured substrings - only once
    if (debuglevel == 1) {
        debuglabel(debuglevel, "CAPTURES");
        for(ci=0; ci < rc; ci++) {

            // "prefix" - the first part of subject if the first match is gt 0
            if (ovector[2*ci] > 0) {
                char * tstr = strndup(subject, ovector[2*ci]);
                printf("%s", tstr);
                free(tstr);
            }

            // colorized substring
            char * tstr = strndup(subject + ovector[2*ci], ovector[2*ci+1] - ovector[2*ci]);
            printf(BOLDGREEN "%s" RESET, tstr);
            free(tstr);

            // "suffix" - the last part of subject if the last match left pos is lt length of subject
            if (ovector[2*ci+1] < strlen(subject)) {
                char * tstr = strndup(subject + ovector[2*ci+1], strlen(subject) - ovector[2*ci+1]);
                printf("%s", tstr);
                free(tstr);
            }
            printf("\n");
        }

        // show the ovector elements
        debuglabel(debuglevel, "OVECTOR");
        printf("[");
        for(ci=0; ci < rc; ci++) {
            printf("%d, %d%s", ovector[2*ci], ovector[2*ci+1], ((ci <= rc-2) ? ", " : ""));
        }
        printf("]\n");
    }

    if (ld_diffs != NULL) {
        free(ld_diffs);
    }

    pcre_free(re);
    if (pce != NULL) {
        pcre_free(pce);
    }
    if (escaped_pattern != NULL) {
        free(escaped_pattern);
    }

    if (time_limit > 0.0) {
        if (((double)ts_diff.tv_sec + ((double)(ts_diff.tv_nsec))/1000000000.0) > time_limit) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}
