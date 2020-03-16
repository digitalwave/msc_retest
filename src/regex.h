/*
 * original source: libmodsecurity3
 * https://github.com/SpiderLabs/ModSecurity/blob/v3/master/src/utils/regex.cc
 */

#include <iostream>
#include <fstream>
#include <string>
#include <list>

#include "regexutils.h"

#ifndef SRC_UTILS_REGEX_H_
#define SRC_UTILS_REGEX_H_


#define OVECCOUNT 900

class SMatch {
 public:
    SMatch() :
	m_match(),
	m_offset(0) { }

    SMatch(const std::string &match, size_t offset) :
	m_match(match),
	m_offset(offset) { }

    const std::string& str() const { return m_match; }
    size_t offset() const { return m_offset; }

 private:
    std::string m_match;
    size_t m_offset;
};

class Regex {
 public:
    explicit Regex(const std::string& pattern_);
    ~Regex();
    std::string pattern;
    pcre *m_pc = NULL;
    pcre_extra *m_pce = NULL;
    int m_ovector[OVECCOUNT];

    int searchAll(const std::string& s, int debuginfo);
    int searchAll2(const std::string& s, int debuginfo);
};


#endif  // SRC_UTILS_REGEX_H_
