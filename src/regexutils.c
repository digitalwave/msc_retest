/*
 * regexutils.c
 */

#include "regexutils.h"

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
