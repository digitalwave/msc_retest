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

void debugvalue(int debuglevel, std::string label, std::string value) {
    if (debuglevel == 1) {
        std::cout << std::endl << label << ":" << std::endl;
        std::cout << std::string(label.size()+1, '=') << std::endl;
        if (value.size() > 0) {
            std::cout << value << std::endl;
        }
    }
}

RegexBase::RegexBase(const std::string& pattern_, int debuglevel)
    : pattern(pattern_.empty() ? ".*" : pattern_),
    m_debuglevel(debuglevel),
    m_ovector {0} { };

Regex::Regex(const std::string& pattern_, int debuglevel): RegexBase::RegexBase(pattern_, debuglevel) {

    const char *errptr = NULL;
    int erroffset;

    if (pattern.empty() == true) {
        pattern.assign(".*");
    }

    m_pc = pcre_compile(pattern.c_str(), PCRE_DOTALL|PCRE_MULTILINE,
        &errptr, &erroffset, NULL);

    m_pce = pcre_study(m_pc, pcre_study_opt, &errptr);

#if PCRE_CONFIG_JIT
    debugvalue(m_debuglevel, std::string("JIT"), std::string("avaliable and used"));
    int pijit;
    int rc = pcre_fullinfo(m_pc, m_pce, PCRE_INFO_JIT, &pijit);
    if ((rc != 0) || (pijit != 1)) {
        std::cout << "Regex does not support JIT" << std::endl;
    }
#else
    debugvalue(m_debuglevel, std::string("JIT"), std::string("not avaliable"));
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

bool Regex::searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures) const {
    const char *subject = s.c_str();
    int ovector[OVECCOUNT];

    int rc = pcre_exec(m_pc, m_pce, subject, s.size(), 0, 0, ovector, OVECCOUNT);

    for (int i = 0; i < rc; i++) {
        size_t start = ovector[2*i];
        size_t end = ovector[2*i+1];
        size_t len = end - start;
        if (end > s.size()) {
            continue;
        }
        SMatchCapture capture(i, start, len);
        captures.push_back(capture);
    }

    return (rc > 0);
}

std::list<SMatch> Regex::searchAll(const std::string& s) {
    const char *subject = s.c_str();
    const std::string tmpString = std::string(s.c_str(), s.size());
    int ovector[OVECCOUNT];
    int rc, offset = 0;

    std::list<SMatch> retList;

    m_execrc = 0;

    do {
        rc = pcre_exec(m_pc, m_pce, subject,
            s.size(), offset, 0, ovector, OVECCOUNT);

        if (rc > 0) {
            m_execrc += rc;
        }

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
            if (len == 0) {
                rc = 0;
                break;
            }
        }
    } while (rc > 0);

    return retList;
}

#ifdef HAVE_PCRE2

Regexv2::Regexv2(const std::string& pattern_, int debuglevel): RegexBase::RegexBase(pattern_, debuglevel) {

    PCRE2_SPTR pcre2_pattern = reinterpret_cast<PCRE2_SPTR>(pattern.c_str());
    uint32_t pcre2_options = (PCRE2_DOTALL|PCRE2_MULTILINE);
    int errornumber = 0;
    PCRE2_SIZE erroroffset = 0;
    m_pc = pcre2_compile(pcre2_pattern, PCRE2_ZERO_TERMINATED,
        pcre2_options, &errornumber, &erroroffset, NULL);
    if (m_pc != NULL) {
        m_match_data = pcre2_match_data_create_from_pattern(m_pc, NULL);
        if (m_match_data == NULL) {
            m_pc = NULL;
        }
    }
}

Regexv2::~Regexv2() {
    pcre2_match_data_free(m_match_data);
    pcre2_code_free(m_pc);
}

bool Regexv2::searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures) const {
    PCRE2_SPTR pcre2_s = reinterpret_cast<PCRE2_SPTR>(s.c_str());
    int rc = pcre2_match(m_pc, pcre2_s, s.length(), 0, 0, m_match_data, NULL);
    PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_match_data);

    for (int i = 0; i < rc; i++) {
        size_t start = ovector[2*i];
        size_t end = ovector[2*i+1];
        size_t len = end - start;
        if (end > s.size()) {
            continue;
        }
        SMatchCapture capture(i, start, len);
        captures.push_back(capture);
    }

    return (rc > 0);
}

std::list<SMatch> Regexv2::searchAll(const std::string& s) {

    const std::string tmpString = std::string(s.c_str(), s.size());
    int rc;

    std::list<SMatch> retList;

    PCRE2_SPTR pcre2_s = reinterpret_cast<PCRE2_SPTR>(s.c_str());
    PCRE2_SIZE offset = 0;

    m_execrc = 0;

    do {
        rc = pcre2_match(m_pc, pcre2_s, s.length(),
                         offset, 0, m_match_data, NULL);
        PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(m_match_data);

        for (int i = 0; i < rc; i++) {
            size_t start = ovector[2*i];
            size_t end = ovector[2*i+1];
            size_t len = end - start;
            if (end > s.size()) {
                rc = -1;
                break;
            }
            std::string match = std::string(s, start, len);
            offset = start + len;
            retList.push_front(SMatch(match, start));

            if (len == 0) {
                rc = 0;
                break;
            }
        }
    } while (rc > 0);

    return retList;
}

#endif