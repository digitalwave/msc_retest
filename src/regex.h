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

void debugvalue(int debuglevel, std::string label, std::string value);

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
    explicit Regex(const std::string& pattern_, int debuglevel);
    ~Regex();
    std::string pattern;
    // m_debuglevel: not part of original code:
    int m_debuglevel;
    pcre *m_pc = NULL;
    pcre_extra *m_pce = NULL;
    int m_ovector[OVECCOUNT];
    // store pcre_exec return value for caller method
    // note, this also isn't part of original code
    int m_execrc;

    std::list<SMatch> searchAll(const std::string& s);
    std::list<SMatch> searchAll2(const std::string& s);
};


#endif  // SRC_UTILS_REGEX_H_
