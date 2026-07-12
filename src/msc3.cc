#include <list>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
#include <algorithm>
#include "regex.h"

void showhelp(const char * name) {
    std::cout << "Use: " << name << " [OPTIONS] patternfile subjectfile" << std::endl;
    std::cout << std::endl;
    std::cout << "You can pass subject through stdin, just give the '-' as subjectfile or leave it" << std::endl;
    std::cout << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "\t-h\tThis help" << std::endl;
    std::cout << "\t-n N\titerate pcre_regex as Nth times. Default value is 1." << std::endl;
    std::cout << "\t-t T\tExpects a float value; if the (last) pcre_exec time is greater than this," << std::endl;
    std::cout << "\t    \tthe exit status of program will non-zero." << std::endl;
    std::cout << "\t-m M\tSet value M for the pcre_match_limit for pcre_extra. Default value is 1000." << std::endl;
    std::cout << "\t-q\tDon't show match details and timing." << std::endl;
#ifdef HAVE_PCRE2
    std::cout << "\t-1  \tuse OLD PCRE engine." << std::endl;
#endif
    std::cout << "\t-d  \tShow detailed information." << std::endl;
    std::cout << "\t-i  \tIgnore (?i) modifiers." << std::endl;
    std::cout << "\t-l  \tUse 'lowercase' transformation for the subject." << std::endl;
    std::cout << std::endl;
}

class LowerCase {
 public:
    template<typename Operation>
    static bool convert(std::string &val, Operation op) {
        bool changed = false;

        std::transform(val.begin(), val.end(), val.data(),
                       [&](auto c) {
                            const auto nc = op(c);
                            if(nc != c) changed = true;
                            return nc; });

        return changed;
    }

    // cppcheck-suppress functionStatic
    bool transform(std::string &value) const {
        return convert(value, [](auto c) {
            return std::tolower(c); });
    }

};

int main(int argc, char ** argv) {
    RegexBase *re;
    char rcerror[100];
    char * patternfile = NULL;
    char * subjectfile = NULL;
    char c;
    int icnt = 1, rc = 0;
    float time_limit = 0.0;
    int debuglevel = 0;  // may be later we can use different level...
    char stdinname[] = "-";
    int use_old_pcre = 0;
    int match_limit = 1000;
    int quiet = 0;
    int ignore_case = 0;
    int use_lowercase = 0;

    struct timespec ts_before, ts_after, ts_diff;
    std::vector<long double> ld_diffs;

    if (argc < 2) {
      showhelp(argv[0]);
      return EXIT_FAILURE;
    }

    while ((c = getopt (argc, argv, "hn:m:t:d1qil")) != -1) {
        switch (c) {
            case 'h':
                showhelp(argv[0]);
                return EXIT_SUCCESS;
            case 'm':
                match_limit = atoi(optarg);
                if (match_limit < 0 || match_limit > 100000) {
                    std::cerr << "Ohh... Try to pass for '-m' an integer between 0 and 100000" << std::endl;
                    return EXIT_FAILURE;
                }
                break;
            case 'n':
                {
                char *endptr;
                // read the value as long, and check for errors
                long val = strtol(optarg, &endptr, 10);

                // Check for errors:
                // 1. Not a number (*endptr is not the end of the string)
                // 2. Less than or equal to 0
                // 3. Greater than INT_MAX
                if (*endptr != '\0' || val <= 0 || val > INT_MAX) {
                    fprintf(stderr, "Ohh... Try to pass for '-n' an integer between 1 and %d\n", INT_MAX);
                    return EXIT_FAILURE;
                }

                icnt = (int)val; // cast to int after validation
                }
                break;
            case 't':
                time_limit = atof(optarg);
                if (time_limit <= 0.0) {
                    fprintf(stderr, "Ohh... Time limit should be positive value.\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'd':
                debuglevel = 1;
                break;
#ifdef WITH_OLD_PCRE
            case '1':
                use_old_pcre = 1;
                break;
#else
            case '1':
                fprintf(stderr, "OLD PCRE engine is not available.\n");
                return EXIT_FAILURE;
#endif
            case 'q':
                quiet = 1;
                break;
            case 'i':
                ignore_case = 1;
                break;
            case 'l':
                use_lowercase = 1;
                break;
            case '?':
                if (optopt == 'n' || optopt == 't') {
                    std::cerr << "Option -" << (char)optopt << " requires an argument." << std::endl;
                }
                else if (isprint (optopt)) {
                    std::cerr << "Unknown option `-" << (char)optopt << "'." << std::endl;
                }
                else {
                    std::cerr << "Unknown option character `\\x%x'." << std::endl;
                }
                return EXIT_FAILURE;
            default:
                abort ();
        }
    }

    for (int i = optind; i < argc; i++) {
        if (patternfile == NULL) {
            patternfile = argv[i];
        }
        else {
            if (subjectfile == NULL) {
                subjectfile = argv[i];
            }
        }
    }

    if (subjectfile == NULL) {
        subjectfile = stdinname;
    }

    if (patternfile == NULL) {
        showhelp(argv[0]);
        return EXIT_FAILURE;
    }

#ifdef WITH_OLD_PCRE
#ifndef PCRE_EXTRA_MATCH_LIMIT
        if (match_limit_set == 1) {
            std::cerr << "Match limit is not available in old PCRE" << std::endl;
            return EXIT_FAILURE;
        }
#endif
    if (use_old_pcre == 1) {
        debugvalue(debuglevel, std::string("PCRE"), std::string("OLD"));
    }
    else {
#endif
        debugvalue(debuglevel, std::string("PCRE"), std::string("NEW"));
#ifdef WITH_OLD_PCRE
    }
#endif

    // read pattern
    std::ifstream pattf(patternfile);
    std::string pattern;
    if (pattf) {
        getline(pattf, pattern);
    }
    else {
        std::cout << "Can't open file: " << patternfile << std::endl;
    }

    if (ignore_case > 0) {
        std::vector<char> buffer(pattern.size() + 1);
        strip_ignorecase_modifiers(pattern.c_str(), buffer.data(), buffer.size());
        pattern = std::string(buffer.data());
    }

    debugvalue(debuglevel, std::string("PATTERN"), pattern);

    std::string subject;
    // read subject
    //   if filename was given
    if (strcmp(subjectfile, "-") != 0) {
        std::ifstream subjf(subjectfile);
        if (subjf) {
            subject.assign((std::istreambuf_iterator<char>(subjf)),
                                (std::istreambuf_iterator<char>()));
        }
        else {
            std::cout << "Can't open file: " << subjectfile << std::endl;
        }
    }
    //   or read from stdin
    else {
      // don't skip the whitespace while reading
      std::cin >> std::noskipws;
      // use stream iterators to copy the stream to a string
      std::istream_iterator<char> it(std::cin);
      std::istream_iterator<char> end;
      subject.assign(it, end);
    }

    if (use_lowercase == 1) {
        LowerCase lc;
        lc.transform(subject);
    }
    debugvalue(debuglevel, std::string("SUBJECT"), subject);

    re = nullptr;

#ifdef WITH_OLD_PCRE
    if (use_old_pcre == 1) {
        re = new Regex(pattern, debuglevel);
    }
    else {
#endif
        re = new Regexv2(pattern, debuglevel);
#ifdef WITH_OLD_PCRE
    }
#endif

    std::vector<SMatchCapture> captures;

    ts_diff.tv_sec  = 0;
    ts_diff.tv_nsec = 0;

    for(int i = 0; i < icnt; i++) {

        re->m_retList.clear();

        ts_diff.tv_sec  = 0;
        ts_diff.tv_nsec = 0;

        clock_gettime(CLOCK_REALTIME, &ts_before);

        captures.clear();
        RegexResult res = re->searchOneMatch(subject, captures, match_limit);
        if (res != RegexResult::Ok) {
            if (res == RegexResult::ErrorMatchLimit) {
                std::cerr << "Error: Match limit was reached." << std::endl;
            } else {
                std::cerr << "Error: An error occurred during regex execution." << std::endl;
            }
        }
        rc = captures.size();

        clock_gettime(CLOCK_REALTIME, &ts_after);
        timespec_diff(&ts_after, &ts_before, &ts_diff);
        // minimal value of re->m_execrc is 0, this means no match
        // in this case we have to decrease the valur for the correct message
        if (rc == 0) {
            rc = -1;
        }
        translate_error(use_old_pcre, rc, rcerror);
        debugvalue(debuglevel, std::string("RESULT"), std::string(""));
        if (quiet == 0) {
            std::cout << patternfile << " - time elapsed: " << ts_diff.tv_sec << "." << std::fixed << std::setfill('0') << std::setw(9) << ts_diff.tv_nsec << ", match value: " << rcerror << std::endl;
        }
        if (icnt > 1) {
            ld_diffs.push_back(ts_diff.tv_sec + (ts_diff.tv_nsec/1000000000.0));
        }
    }

    if (icnt > 1) {
        show_stat(&ld_diffs[0], icnt);
    }

    // show captured substrings if debug was set
    if (debuglevel == 1) {
        debugvalue(debuglevel, "CAPTURES", "");

        for (const SMatchCapture& capture : captures) {
            const std::string capture_substring(subject.substr(capture.m_offset, capture.m_length));
            std::string subpatt = "";
            if (capture.m_offset > 0) {
                subpatt += subject.substr(0, capture.m_offset);
            }
            subpatt += BOLDGREEN + capture_substring + RESET;
            if (capture.m_offset + capture_substring.size() < subject.size()) {
                subpatt += subject.substr(capture.m_offset + capture_substring.size());
            }
            std::cout << subpatt << std::endl;
        }

        debugvalue(debuglevel, "OVECTOR", "");
        std::cout << "[";
        size_t si = 0;
        for(auto const& capture: captures) {
            const std::string capture_substring(subject.substr(capture.m_offset, capture.m_length));
            std::cout << capture.m_offset << ", " << capture.m_offset + capture_substring.size() << ((si++ < captures.size()-1) ? ", " : "");
        }
        std::cout << "]" << std::endl;
    }
    // end debug

    if (time_limit > 0.0) {
        if (((double)ts_diff.tv_sec + ((double)(ts_diff.tv_nsec))/1000000000.0) > time_limit) {
            return EXIT_FAILURE;
        }
    }

    delete(re);

    return EXIT_SUCCESS;
}

