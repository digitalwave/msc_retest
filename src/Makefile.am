AM_CXXFLAGS = -Wall -g -O0 -lpcre -std=c++11 -fPIC
AM_CFLAGS = -Wall -g -O0 -lpcre

bin_PROGRAMS = pcre4msc2 pcre4msc3

pcre4msc2_SOURCES = msc2.c regexutils.c
pcre4msc2_CFLAGS = $(AM_CFLAGS)

pcre4msc3_SOURCES = msc3.cc regex.cc regexutils.c
pcre4msc3_CXXFLAGS = $(AM_CXXFLAGS)


