/*
 * original source: libmodsecurity3
 * https://github.com/SpiderLabs/ModSecurity/blob/v3/master/src/utils/regex.cc
 */
#include <pcre.h>

#include <iostream>
#include <fstream>
#include <string>
#include <list>

#ifndef SRC_UTILS_REGEX_H_
#define SRC_UTILS_REGEX_H_


#define OVECCOUNT 900

class Regex {
 public:
    explicit Regex(const std::string& pattern_);
    ~Regex();
    std::string pattern;
    pcre *m_pc = NULL;
    pcre_extra *m_pce = NULL;
    int m_ovector[OVECCOUNT];

    int searchAll(const std::string& s);
};


#endif  // SRC_UTILS_REGEX_H_
