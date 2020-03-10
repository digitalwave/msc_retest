#include <list>
#include <iostream>
#include <fstream>
#include "regex.h"

int main(int argc, char ** argv) {
    Regex *re;
    int rc;
    clock_t start, end;

    if (argc < 3) {
        std::cout << "Argument missing!" << std::endl;
        std::cout << "Use " << argv[0] << " patternfile subjectfile" << std::endl;
        return -1;
    }

    std::ifstream pattf(argv[1]);
    std::string pattern;
    if (pattf) {
        pattern.assign((std::istreambuf_iterator<char>(pattf)),
                             (std::istreambuf_iterator<char>()));
    }
    else {
        std::cout << "Can't open file: " << argv[1] << std::endl;
    }

    std::ifstream subjf(argv[2]);
    std::string subject;
    if (subjf) {
        subject.assign((std::istreambuf_iterator<char>(subjf)),
                              (std::istreambuf_iterator<char>()));
    }
    else {
        std::cout << "Can't open file: " << argv[2] << std::endl;
    }

    re = new Regex(pattern);

    for(int i=0; i<10; i++) {
        start = clock();
        rc = re->searchAll(subject);
        end = clock();
        double sub = double(end - start) / double(CLOCKS_PER_SEC);
        std::cout << "Time elapsed: " << sub << ", rc: " << rc << std::endl;
    }

    return 0;
}

