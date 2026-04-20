/*
 * original source: libmodsecurity3
 * https://github.com/SpiderLabs/ModSecurity/blob/v3/master/src/utils/regex.cc
 */

#include <iostream>
#include <fstream>
#include <string>
#include <list>
#include <vector>

#include "regexutils.h"

#ifndef SRC_UTILS_REGEX_H_
#define SRC_UTILS_REGEX_H_

#define OVECCOUNT 900

enum class RegexResult {
    Ok,
    ErrorMatchLimit,
    ErrorOther,
};

void debugvalue(int debuglevel, const std::string& label, const std::string& value);

class SMatch {
 public:
    SMatch() :
	m_match(),
	m_offset(0) { }

    SMatch(const std::string &match, size_t offset) :
	m_match(match),
	m_offset(offset) { }


 private:
    std::string m_match;
    size_t m_offset;
};

struct SMatchCapture {
    SMatchCapture(size_t group, size_t offset, size_t length) :
	m_group(group),
	m_offset(offset),
	m_length(length) { }

    size_t m_group; // E.g. 0 = full match; 6 = capture group 6
    size_t m_offset; // offset of match within the analyzed string
    size_t m_length;
};

class RegexBase {
 public:
    RegexBase(const std::string& pattern_, int debuglevel, bool ignoreCase = false);
    virtual ~RegexBase() {};
    const std::string pattern;
    // m_debuglevel: not part of original code:
    int m_debuglevel;
    int m_ovector[OVECCOUNT];
    // store pcre_exec return value for caller method
    // note, this also isn't part of original code
    int m_execrc;
    std::list<SMatch> m_retList;

    virtual std::list<SMatch> searchAll(const std::string& s) = 0;
    virtual RegexResult searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures, unsigned long match_limit = 0) const = 0;
    virtual RegexResult to_regex_result(int pcre_exec_result) const = 0;
};

#ifdef WITH_OLD_PCRE
class Regex: public RegexBase {
 public:
    explicit Regex(const std::string& pattern_, int debuglevel, bool ignoreCase = false);
    ~Regex() override;

    std::list<SMatch> searchAll(const std::string& s) override;
    RegexResult searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures, unsigned long match_limit = 0) const override;
    RegexResult to_regex_result(int pcre_exec_result) const override;

 private:
    pcre *m_pc = NULL;
    pcre_extra *m_pce = NULL;
};
#endif

class Regexv2: public RegexBase {
 public:
    explicit Regexv2(const std::string& pattern_, int debuglevel, bool ignoreCase = false);
    ~Regexv2() override;

    std::list<SMatch> searchAll(const std::string& s) override;
    RegexResult searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures, unsigned long match_limit = 0) const override;
    RegexResult to_regex_result(int pcre_exec_result) const override;

 private:
    pcre2_code *m_pc;
    pcre2_match_data *m_match_data;
    int m_pcje;
};

class Pcre2MatchContextPtr {
 public:
    Pcre2MatchContextPtr()
        : m_match_context(pcre2_match_context_create(nullptr)) {}

		Pcre2MatchContextPtr(const Pcre2MatchContextPtr&) = delete;
		Pcre2MatchContextPtr& operator=(const Pcre2MatchContextPtr&) = delete;

    ~Pcre2MatchContextPtr() {
        pcre2_match_context_free(m_match_context);
    }

    explicit operator pcre2_match_context*() const {
        return m_match_context;
    }

 private:
    pcre2_match_context *m_match_context;
};


#endif  // SRC_UTILS_REGEX_H_
