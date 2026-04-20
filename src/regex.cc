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

void debugvalue(int debuglevel, const std::string &label, const std::string &value) {
    if (debuglevel == 1) {
        std::cout << std::endl << label << ":" << std::endl;
        std::cout << std::string(label.size()+1, '=') << std::endl;
        if (value.size() > 0) {
            std::cout << value << std::endl;
        }
    }
}

RegexBase::RegexBase(const std::string& pattern_, int debuglevel, bool ignoreCase)
    : pattern(pattern_.empty() ? ".*" : pattern_),
    m_debuglevel(debuglevel),
    m_ovector {0},
    m_execrc(0) { };

#ifdef WITH_OLD_PCRE
Regex::Regex(const std::string& pattern_, int debuglevel, bool ignoreCase):
    RegexBase::RegexBase(pattern_, debuglevel, ignoreCase) {

    const char *errptr = nullptr;
    int erroffset;
    int flags = (PCRE_DOTALL|PCRE_MULTILINE);

    if (ignoreCase == true) {
        flags |= PCRE_CASELESS;
    }
    m_pc = pcre_compile(pattern.c_str(), flags,
        &errptr, &erroffset, NULL);
    if (m_pc == NULL) {
        fprintf(stderr, "PCRE compilation failed at offset %d: %s\n", erroffset, errptr);
        fprintf(stderr, "Regex: '%s'\n", pattern.c_str());
        exit(1);
    }
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

RegexResult Regex::searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures, unsigned long match_limit) const {
    const char *subject = s.c_str();
    int ovector[OVECCOUNT];
    pcre_extra local_pce;
    pcre_extra *pce = m_pce;

    if (m_pce != nullptr && match_limit > 0) {
        local_pce = *m_pce;
        local_pce.match_limit = match_limit;
        local_pce.flags |= PCRE_EXTRA_MATCH_LIMIT;
        pce = &local_pce;
    }

    int rc = pcre_exec(m_pc, pce, subject, s.size(), 0, 0, ovector, OVECCOUNT);

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

    return to_regex_result(rc);
}
#endif
// end of old pcre implementation

Regexv2::Regexv2(const std::string& pattern_, int debuglevel, bool ignoreCase):
    RegexBase::RegexBase(pattern_, debuglevel, ignoreCase),
    m_match_data(nullptr) {
    PCRE2_SPTR pcre2_pattern = reinterpret_cast<PCRE2_SPTR>(pattern.c_str());
    uint32_t pcre2_options = (PCRE2_DOTALL|PCRE2_MULTILINE);
    if (ignoreCase) {
        pcre2_options |= PCRE2_CASELESS;
    }
    int errornumber = 0;
    PCRE2_SIZE erroroffset = 0;

    m_match_data = nullptr;

    m_pc = pcre2_compile(pcre2_pattern, PCRE2_ZERO_TERMINATED,
        pcre2_options, &errornumber, &erroroffset, nullptr);
    if (m_pc == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed: %s\n", buffer);
        fprintf(stderr, "Regex: '%s'\n", pcre2_pattern);
        exit(1);
    }

    m_pcje = pcre2_jit_compile(m_pc, PCRE2_JIT_COMPLETE);
    if (m_pcje == 0) {
        debugvalue(m_debuglevel, std::string("JIT"), std::string("avaliable and used"));
    }
    else {
        debugvalue(m_debuglevel, std::string("JIT"), std::string("not avaliable"));
    }
}

Regexv2::~Regexv2() {
    pcre2_code_free(m_pc);
}

RegexResult Regexv2::searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures, unsigned long match_limit) const {
    Pcre2MatchContextPtr match_context;
    if (match_limit > 0) {
        // TODO: What if setting the match limit fails?
        pcre2_set_match_limit(static_cast<pcre2_match_context*>(match_context), match_limit);
    }

    PCRE2_SPTR pcre2_s = reinterpret_cast<PCRE2_SPTR>(s.c_str());
    pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(m_pc, nullptr);
    if (match_data == nullptr) {
        return RegexResult::ErrorOther;
    }
    int rc = 0;
    if (m_pcje == 0) {
        rc = pcre2_jit_match(m_pc, pcre2_s, s.length(), 0, 0, match_data, static_cast<pcre2_match_context*>(match_context));
    }

    if (m_pcje != 0 || rc == PCRE2_ERROR_JIT_STACKLIMIT) {
        rc = pcre2_match(m_pc, pcre2_s, s.length(), 0, PCRE2_NO_JIT, match_data, static_cast<pcre2_match_context*>(match_context));
    }
    const PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);

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
    pcre2_match_data_free(match_data);

    return to_regex_result(rc);
}



#ifdef WITH_OLD_PCRE
// cppcheck-suppress functionStatic
RegexResult Regex::to_regex_result(int pcre_exec_result) const {
    if (
        pcre_exec_result > 0 ||
        pcre_exec_result == PCRE_ERROR_NOMATCH
    ) {
        return RegexResult::Ok;
    } else if(
        pcre_exec_result == PCRE_ERROR_MATCHLIMIT
    ) {
        return RegexResult::ErrorMatchLimit;
    } else {
        // Note that this can include the case where the PCRE result was zero.
        // Zero is returned if the offset vector is not large enough and can be considered an error.
        return RegexResult::ErrorOther;
    }
}
#endif

// cppcheck-suppress functionStatic
RegexResult Regexv2::to_regex_result(int pcre_exec_result) const {
    if (
        pcre_exec_result > 0 ||
        pcre_exec_result == PCRE2_ERROR_NOMATCH
    ) {
        return RegexResult::Ok;
    } else if(
        pcre_exec_result == PCRE2_ERROR_MATCHLIMIT
    ) {
        return RegexResult::ErrorMatchLimit;
    } else {
        // Note that this can include the case where the PCRE result was zero.
        // Zero is returned if the offset vector is not large enough and can be considered an error.
        return RegexResult::ErrorOther;
    }
}

