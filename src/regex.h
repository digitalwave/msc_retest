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

struct SMatchCapture {
    SMatchCapture(size_t group, size_t offset, size_t length) :
	m_group(group),
	m_offset(offset),
	m_length(length) { }

    size_t m_group; // E.g. 0 = full match; 6 = capture group 6
    size_t m_offset; // offset of match within the analyzed string
    size_t m_length;
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
    std::list<SMatch> m_retList;

    std::list<SMatch> searchAll(const std::string& s);
    bool searchOneMatch(const std::string& s, std::vector<SMatchCapture>& captures) const;
    int searchAll2(const std::string& s, size_t capturelen);
};


#endif  // SRC_UTILS_REGEX_H_
