#include <list>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>
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
#ifdef HAVE_PCRE2
    std::cout << "\t-1  \tuse OLD PCRE engine." << std::endl;
#endif
    std::cout << "\t-d  \tShow detailed information." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char ** argv) {
    RegexBase *re;
    char rcerror[100];
    char * patternfile = NULL, * subjectfile = NULL;
    char c;
    int icnt = 1, rc = 0;
    float time_limit = 0.0;
    int debuglevel = 0;  // may be later we can use different level...
    char stdinname[] = "-";
    int use_old_pcre = 0;

    struct timespec ts_before, ts_after, ts_diff;
    std::vector<long double> ld_diffs;

    if (argc < 2) {
      showhelp(argv[0]);
      return EXIT_FAILURE;
    }

    while ((c = getopt (argc, argv, "hn:t:d1")) != -1) {
        switch (c) {
            case 'h':
                showhelp(argv[0]);
                return EXIT_SUCCESS;
            case 'n':
                icnt = atoi(optarg);
                if (icnt <= 0 || icnt > INT_MAX) {
                    std::cerr << "Ohh... Try to pass for '-n' an integer between 1 and " << INT_MAX << std::endl;
                    return EXIT_FAILURE;
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

    debugvalue(debuglevel, std::string("SUBJECT"), subject);

    re = NULL;

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
        re->searchOneMatch(subject, captures);
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
        std::cout << patternfile << " - time elapsed: " << ts_diff.tv_sec << "." << std::fixed << std::setfill('0') << std::setw(9) << ts_diff.tv_nsec << ", match value: " << rcerror << std::endl;
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

