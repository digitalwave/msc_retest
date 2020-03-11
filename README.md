msc_pcretest
===========

Welcome to the `msc_pcretest` documentation.

Description
===========

This tool contains two binaries: `pcre4msc2` and `pcre4msc3`. The binaries emulates the behaviors of regex engine (PCRE - the old version) in mod_security2 (Apache module) and the libmodsecurity3. With this programs, you can check the evaulation time of every regular expressions with any random input. Both of them (regex pattern, input subject) needs to exists in two separated files, and you can pass them as argument.

In case of `pcre4msc2` if the engine supports the JIT, you can use the `-j` flag to enable it. Otherwise, that will ignore of use it.

Every time when you run some of the programs, the engine will searches 10 times the pattern in the subject, and will inform about the elapsed times.

Requirements
============

To compile the code you need the pcre library (eg. `libpcre3-dev` package on Debian - this is the old version), and the autotools.

Build
=====

Download and unpack the source, and

```
autoreconf --install
./configure
make
```

Use
===

A quick how to:

```
echo "(?i:random_regular_expression)" > pattern1.txt
echo "random_subject" > subject1.txt
src/pcre4msc2 -j pattern1.txt subject1.txt
Time elapsed: 0.00nnn, rc: -1
Time elapsed: 0.00nnn, rc: -1
....
Time elapsed: 0.00nnn, rc: -1
```

Note, the `rc` means the return code of `pcre_exec()`.

Now try again without `-j`, and the `src/pcre4msc3` too.

For a complete CRS 3.2 test, try this:

```
for d in `ls -1 data/9*.txt`; do echo ${d}; src/pcre4msc2 ${d} /path/to/your/sibject; done
for d in `ls -1 data/9*.txt`; do echo ${d}; src/pcre4msc2 -j ${d} /path/to/your/sibject; done
for d in `ls -1 data/9*.txt`; do echo ${d}; src/pcre4msc3 ${d} /path/to/your/sibject; done
```
