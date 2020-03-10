/*
 * original source: libmodsecurity3
 * https://github.com/SpiderLabs/ModSecurity/blob/v3/master/src/utils/regex.h
 */

#include "regex.h"

#include <pcre.h>
#include <string>


#if PCRE_CONFIG_JIT
#define pcre_study_opt PCRE_STUDY_JIT_COMPILE
#else
#define pcre_study_opt 0
#endif


Regex::Regex(const std::string& pattern_)
    : pattern(pattern_),
    m_ovector {0} {
    const char *errptr = NULL;
    int erroffset;

    if (pattern.empty() == true) {
        pattern.assign(".*");
    }

    m_pc = pcre_compile(pattern.c_str(), PCRE_DOTALL|PCRE_MULTILINE,
        &errptr, &erroffset, NULL);

    m_pce = pcre_study(m_pc, pcre_study_opt, &errptr);
}


Regex::~Regex() {
    if (m_pc != NULL) {
        pcre_free(m_pc);
        m_pc = NULL;
    }
    if (m_pce != NULL) {
#if PCRE_CONFIG_JIT
        pcre_free_study(m_pce);
#else
        pcre_free(m_pce);
#endif
        m_pce = NULL;
    }
}


int Regex::searchAll(const std::string& s) {
    const char *subject = s.c_str();
    const std::string tmpString = std::string(s.c_str(), s.size());
    int ovector[OVECCOUNT];
    int rc, offset = 0;

    rc = pcre_exec(m_pc, m_pce, subject,
        s.size(), offset, 0, ovector, OVECCOUNT);

    return rc;
}

