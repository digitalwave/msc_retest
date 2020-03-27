#include <list>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <unistd.h>
#include <ctype.h>
// #include <sys/time.h>
#include "regex.h"
#include "regexutils.h"

void showhelp(char * name) {
    std::cerr << "Use: " << name << " [OPTIONS] patternfile subjectfile" << std::endl;
    std::cerr << "OPTIONS:" << std::endl;
    std::cerr << "\t-h\tThis help" << std::endl;
    std::cerr << "\t-n N\titerate pcre_regex as Nth times. Default value is 1." << std::endl;
    std::cerr << "\t-f\tForce to use modified regex matching method." << std::endl;
    std::cerr << std::endl;
}

int main(int argc, char ** argv) {
    Regex *re;
    int rc = 0;
    char rcerror[100];
    char * patternfile = NULL, * subjectfile = NULL;
    char c;
    int icnt = 1;
    bool use_fixed = false;
    bool debuginfo = false;

    if (argc < 3) {
      showhelp(argv[0]);
      return -1;
    }

    while ((c = getopt (argc, argv, "hfmn:")) != -1) {
        switch (c) {
            case 'h':
                showhelp(argv[0]);
                return 0;
            case 'f':
                use_fixed = true;
                break;
            case 'n':
                icnt = atoi(optarg);
                if (icnt <= 0 || icnt > 10) {
                    std::cerr << "Ohh... Try to pass for '-n' an integer between 1 and 10" << std::endl;
                    return -1;
                }
                break;
            case '?':
                if (optopt == 'n') {
                    std::cerr << "Option -" << (char)optopt << " requires an argument." << std::endl;
                }
                else if (isprint (optopt)) {
                    std::cerr << "Unknown option `-" << (char)optopt << "'." << std::endl;
                }
                else {
                    std::cerr << "Unknown option character `\\x%x'." << std::endl;
                }
                return -1;
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
        return -1;
    }

    std::ifstream pattf(patternfile);
    std::string pattern;
    if (pattf) {
        pattern.assign((std::istreambuf_iterator<char>(pattf)),
                             (std::istreambuf_iterator<char>()));
    }
    else {
        std::cout << "Can't open file: " << patternfile << std::endl;
    }

    std::ifstream subjf(subjectfile);
    std::string subject;
    if (subjf) {
        subject.assign((std::istreambuf_iterator<char>(subjf)),
                              (std::istreambuf_iterator<char>()));
    }
    else {
        std::cout << "Can't open file: " << subjectfile << std::endl;
    }

    re = new Regex(pattern);

    for(int i = 0; i < icnt; i++) {
        clock_t m_start = clock();
        if (use_fixed == false) {
            rc = re->searchAll(subject, debuginfo);
        }
        else {
            rc = re->searchAll2(subject, debuginfo);
        }
        clock_t m_end = clock();
        double m_sub = (m_end - m_start) / double(CLOCKS_PER_SEC);
        translate_error(rc, rcerror);
        std::cout << patternfile << " - time elapsed: " << std::fixed << std::setfill('0') << std::setw(6) << m_sub << ", match value: " << rcerror << std::endl;
    }

    return rc;
}

