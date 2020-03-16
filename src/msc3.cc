#include <list>
#include <iostream>
#include <fstream>
#include "regex.h"
#include "regexutils.h"

void showhelp() {
    std::cout << "\t-h\tThis help" << std::endl;
}

int main(int argc, char ** argv) {
    Regex *re;
    int rc;
    clock_t start, end;
    char rcerror[100];
    char * patternfile = NULL, * subjectfile = NULL;
    int icnt = 1;
    int use_fixed = 0;
    int debuginfo = 0;

    if (argc < 3) {
        std::cout << "Argument missing!" << std::endl;
        std::cout << "Use " << argv[0] << " patternfile subjectfile" << std::endl;
        return -1;
    }

    for(int i = 1; i < argc; i++) {

        if(std::string(argv[i]) == "-h") {
            std::cout << "Avaliable arguments:" << std::endl;
            showhelp();
            return 0;
        }
        else if(std::string(argv[i]) == "-f") {
            use_fixed = 1;
        }
        else if(std::string(argv[i]) == "-d") {
            debuginfo = 1;
        }
        else if (std::string(argv[i]) == "-n") {
            icnt = atoi(argv[++i]);
            if (icnt <= 0 || icnt > 10) {
                printf("Ohh... Try to pass for '-n' an integer between 1 and 10\n");
                return -1;
            }
        }
        else {
            if (patternfile == NULL) {
                patternfile = argv[i];
            }
            else {
                if (subjectfile == NULL) {
                    subjectfile = argv[i];
                }
            }
        }
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
        start = clock();
        if (use_fixed == 0) {
            rc = re->searchAll(subject, debuginfo);
        }
        else {
            rc = re->searchAll2(subject, debuginfo);
        }
        end = clock();
        double sub = double(end - start) / double(CLOCKS_PER_SEC);
        translate_error(rc, rcerror);
        std::cout << patternfile << " - time elapsed: " << sub << ", matched: " << ((rc <= 0) ? "not matched" : "matched") << ", regex error: " << rcerror << std::endl;
    }

    return 0;
}

