/*
 * regexutils.c
 */

#include "regexutils.h"
#include <math.h>

#define SETRCERROR(str)  strcpy(rcerror, str); break

void translate_error(int rc, char * rcerror) {
    switch (rc) {
        case  PCRE_ERROR_NOMATCH:           SETRCERROR("NOT MATCHED");
        case  PCRE_ERROR_NULL:              SETRCERROR("NULL ERROR");
        case  PCRE_ERROR_BADOPTION:         SETRCERROR("BAD OPTION");
        case  PCRE_ERROR_BADMAGIC:          SETRCERROR("BAD MAGIC");
        case  PCRE_ERROR_UNKNOWN_OPCODE:    SETRCERROR("UNKNOWN OPCODE");
        case  PCRE_ERROR_NOMEMORY:          SETRCERROR("NO MEMORY");
        case  PCRE_ERROR_NOSUBSTRING:       SETRCERROR("NO SUBSTRING");
        case  PCRE_ERROR_MATCHLIMIT:        SETRCERROR("MATCH LIMIT");
        case  PCRE_ERROR_CALLOUT:           SETRCERROR("CALL OUT");
        case  PCRE_ERROR_BADUTF8:           SETRCERROR("BAD UTF");
        case  PCRE_ERROR_BADUTF8_OFFSET:    SETRCERROR("BAD UTF OFFSET");
        case  PCRE_ERROR_PARTIAL:           SETRCERROR("PARTIAL ERROR");
        case  PCRE_ERROR_BADPARTIAL:        SETRCERROR("BAD PARTIAL ERROR");
        case  PCRE_ERROR_INTERNAL:          SETRCERROR("INTERNAL ERROR");
        case  PCRE_ERROR_BADCOUNT:          SETRCERROR("BAD COUNT");
        case  PCRE_ERROR_DFA_UITEM:         SETRCERROR("DFA UITEM");
        case  PCRE_ERROR_DFA_UCOND:         SETRCERROR("DFA UCOND");
        case  PCRE_ERROR_DFA_UMLIMIT:       SETRCERROR("DFA UMLIMIT");
        case  PCRE_ERROR_DFA_WSSIZE:        SETRCERROR("DFA WSSIZE");
        case  PCRE_ERROR_DFA_RECURSE:       SETRCERROR("DFA RECURSE");
        case  PCRE_ERROR_RECURSIONLIMIT:    SETRCERROR("RECURSION LIMIT");
        case  PCRE_ERROR_NULLWSLIMIT:       SETRCERROR("NULL WS LIMIT");
        case  PCRE_ERROR_BADNEWLINE:        SETRCERROR("BAD NEWLINE");
        case  PCRE_ERROR_BADOFFSET:         SETRCERROR("BAD OFFSET");
        case  PCRE_ERROR_SHORTUTF8:         SETRCERROR("SHORT UTF");
        case  PCRE_ERROR_RECURSELOOP:       SETRCERROR("RECURSE LOOP");
        case  PCRE_ERROR_JIT_STACKLIMIT:    SETRCERROR("JIT STACKLIMIT");
        case  PCRE_ERROR_BADMODE:           SETRCERROR("BAD MODE");
        case  PCRE_ERROR_BADENDIANNESS:     SETRCERROR("BAD ENDIANNESS");
        case  PCRE_ERROR_DFA_BADRESTART:    SETRCERROR("DFA BAD RESTART");
        case  PCRE_ERROR_JIT_BADOPTION:     SETRCERROR("JIT BAD OPTION");
        case  PCRE_ERROR_BADLENGTH:         SETRCERROR("BAD LENGTH");
        case  PCRE_ERROR_UNSET:             SETRCERROR("UNSET");
        default:
            if (rc < 0) {
                SETRCERROR("UNKNOWN ERROR");
            }
            else if (rc == 0) {
                SETRCERROR("OVECTOR OUT OF SPACE");
            }
            else {
                sprintf(rcerror, "SUBJECT MATCHED %d TIME%s", rc, ((rc > 1) ? "S" : ""));
            }
    }
}

int compare_ld (const void * a, const void * b) {
  if (*(long double*)a > *(long double*)b) return 1;
  else if (*(long double*)a < *(long double*)b) return -1;
  else return 0;
}

long double calc_std_deviation(const long double * arr, const int n, const long double avg) {
    long double sum = 0.0;
    for(int i = 0; i < n; i++) {
        sum += powl(arr[i]-avg, 2.0);
    }
    return sqrtl(sum/((long double)n));
}

void show_stat(long double * ld_diffs, int icnt) {

    if (icnt == 0) {
        return;
    }

    double long ld_mean = 0.0, ld_med = 0.0, ld_sums = 0.0;
    double long ld_minval = 0.0, ld_maxval = 0.0;

#ifdef __cplusplus
    qsort(&ld_diffs[0], icnt, sizeof(long double), compare_ld);
#else
    qsort(ld_diffs, icnt, sizeof(long double), compare_ld);
#endif

    ld_minval = ld_diffs[0];
    ld_maxval = ld_diffs[icnt-1];

    for(int i = 0; i < icnt; i++) {
        ld_sums += ld_diffs[i];
    }

    ld_mean = ld_sums / (double)icnt;

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
#ifdef __cplusplus
    printf("Std deviation: %013.9Lf\n", calc_std_deviation(&ld_diffs[0], icnt, ld_mean));
#else
    printf("Std deviation: %013.9Lf\n", calc_std_deviation(ld_diffs, icnt, ld_mean));
#endif
}