/*
 * original source: libmodsecurity3
 * https://github.com/SpiderLabs/ModSecurity/blob/v3/master/src/utils/regex.h
 */

#include "regex.h"

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

#if PCRE_CONFIG_JIT
    int pijit;
    int rc = pcre_fullinfo(m_pc, m_pce, PCRE_INFO_JIT, &pijit);
    if ((rc != 0) || (pijit != 1)) {
        std::cout << "Regex does not support JIT" << std::endl;
    }
#endif
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


int Regex::searchAll(const std::string& s, bool debuginfo) {
    const char *subject = s.c_str();
    const std::string tmpString = std::string(s.c_str(), s.size());
    int ovector[OVECCOUNT];
    int rc, offset = 0;

    std::list<SMatch> retList;

    do {
        rc = pcre_exec(m_pc, m_pce, subject,
            s.size(), offset, 0, ovector, OVECCOUNT);

        m_execrc = rc;

        for (int i = 0; i < rc; i++) {
            size_t start = ovector[2*i];
            size_t end = ovector[2*i+1];
            size_t len = end - start;
            if (end > s.size()) {
                rc = 0;
                break;
            }
            std::string match = std::string(tmpString, start, len);
            offset = start + len;
            retList.push_front(SMatch(match, start));
            if (debuginfo == true) {
                std::cout << "captured: " << match << std::endl;
            }
            if (len == 0) {
                rc = 0;
                break;
            }
        }
    } while (rc > 0);

    return retList.size();
}

int Regex::searchAll2(const std::string& s, bool debuginfo) {
    const char *subject = s.c_str();
    const std::string tmpString = std::string(s.c_str(), s.size());
    int ovector[OVECCOUNT];
    int rc, offset = 0;

    std::list<SMatch> retList;

    rc = pcre_exec(m_pc, m_pce, subject,
        s.size(), offset, 0, ovector, OVECCOUNT);

    m_execrc = rc;

    for (int i = 0; i < rc; i++) {
        size_t start = ovector[2*i];
        size_t end = ovector[2*i+1];
        size_t len = end - start;
        if (end > s.size()) {
            rc = 0;
            break;
        }
        std::string match = std::string(tmpString, start, len);
        offset = start + len;
        retList.push_front(SMatch(match, start));
        if (debuginfo == true) {
            std::cout << "captured: " << match << std::endl;
        }
        if (len == 0) {
            rc = 0;
            break;
        }
    }

    return retList.size();
}
