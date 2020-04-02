#include <list>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <ctype.h>
#include "regex.h"
#include "regexutils.h"

void showhelp(char * name) {
    std::cout << "Use: " << name << " [OPTIONS] patternfile subjectfile" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "\t-h\tThis help" << std::endl;
    std::cout << "\t-n N\titerate pcre_regex as Nth times. Default value is 1." << std::endl;
    std::cout << "\t-f\tForce to use modified regex matching method." << std::endl;
    std::cout << "\t-t T\tExpects a float value; if the (last) pcre_exec time is greather than this," << std::endl;
    std::cout << "\t    \tthe exit status of program will non-zero." << std::endl;
    std::cout << "\t-d  \tShow debug information." << std::endl;
    std::cout << std::endl;
}

int main(int argc, char ** argv) {
    Regex *re;
    char rcerror[100];
    char * patternfile = NULL, * subjectfile = NULL;
    char c;
    int icnt = 1, rc = 0;
    bool use_fixed = false;
    float time_limit = 0.0;
    double m_sub = 0.0;
    int debuglevel = 0;  // may be later we can use different level...

    if (argc < 3) {
      showhelp(argv[0]);
      return EXIT_FAILURE;
    }

    while ((c = getopt (argc, argv, "hfn:t:d")) != -1) {
        switch (c) {
            case 'h':
                showhelp(argv[0]);
                return EXIT_SUCCESS;
            case 'f':
                use_fixed = true;
                break;
            case 'n':
                icnt = atoi(optarg);
                if (icnt <= 0 || icnt > 10) {
                    std::cerr << "Ohh... Try to pass for '-n' an integer between 1 and 10" << std::endl;
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

    if (patternfile == NULL || subjectfile == NULL) {
        showhelp(argv[0]);
        return EXIT_FAILURE;
    }

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

    // read subject
    std::ifstream subjf(subjectfile);
    std::string subject;
    if (subjf) {
        subject.assign((std::istreambuf_iterator<char>(subjf)),
                              (std::istreambuf_iterator<char>()));
    }
    else {
        std::cout << "Can't open file: " << subjectfile << std::endl;
    }

    debugvalue(debuglevel, std::string("SUBJECT"), subject);

    re = new Regex(pattern, debuglevel);

    std::list<SMatch> retval;

    for(int i = 0; i < icnt; i++) {
        clock_t m_start = clock();
        if (use_fixed == false) {
            retval = re->searchAll(subject);
        }
        else {
            retval = re->searchAll2(subject);
        }
        clock_t m_end = clock();
        m_sub = (m_end - m_start) / double(CLOCKS_PER_SEC);
        // minimal value of re->m_execrc is 0, this means no match
        // in this case we have to decrease the valur for the correct message
        rc = re->m_execrc;
        if (rc == 0) {
            rc = -1;
        }
        translate_error(rc, rcerror);
        debugvalue(debuglevel, std::string("RESULT"), std::string(""));
        std::cout << patternfile << " - time elapsed: " << std::fixed << std::setfill('0') << std::setw(6) << m_sub << ", match value: " << rcerror << std::endl;
    }

    // show captured substrings if debug was set
    if (debuglevel == 1) {
        debugvalue(debuglevel, "CAPTURES", "");
        retval.reverse();
        for(auto s: retval) {
            std::string subpatt = "";
            if (s.offset() > 0) {
                subpatt += subject.substr(0, s.offset());
            }
            subpatt += BOLDGREEN + s.str() + RESET;
            if (s.offset() + s.str().size() < subject.size()) {
                subpatt += subject.substr(s.offset() + s.str().size());
            }
            std::cout << subpatt << std::endl;
        }

        debugvalue(debuglevel, "OVECTOR", "");
        std::cout << "[";
        for(int i = 0; i < rc; i++) {
            std::cout << re->m_ovector[2*i] << ", " << re->m_ovector[2*i+1] << ((i < rc-1) ? ", " : "");
        }
        std::cout << "]" << std::endl;
    }
    // end debug

    if (time_limit > 0.0) {
        if (m_sub > time_limit) {
            return EXIT_FAILURE;
        }
    }

    delete(re);

    return EXIT_SUCCESS;
}

