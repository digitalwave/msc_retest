# msc_pcretest

Welcome to the `msc_pcretest` documentation.

Description
===========

This tool compiles two binaries: `pcre4msc2` and `pcre4msc3`. The binaries emulates the behaviors of regex engine (PCRE - the old version) in mod_security2 (Apache module) and the libmodsecurity3. With this programs, you can check the evaulation time and result of every regular expressions with any random (including very extreme long) input. Both of them (regex pattern, input subject) needs to exists in two separated files, and you can pass them as argument.

The source tree contains some extra directories: under the `data/` you can find all of the regular expressions, what CRS uses. The files contains the id of the rule, and a suffix (there are some chained rules, where more parts uses `@rx`).

The `utils/` directory contains a python tool, which helps to re-generate the regexes above. It uses `msc_pyparser` - for more information, see the README.md in that directory.

Useful information
==================

There are so many difference between mod_security2 and libmodsecurity3 implementations:

pcre_study(), JIT
-----------------

* mod_security2 module uses `pcre_study()` if it's avaliable, but **you can disable** it (with `--enable-pcre-study=no`)
* mod_security2 module uses `PCRE_JIT` if it's avaliable **and you enabled it in build time** (with `--enable-pcre-jit`)
* libmodsecurity3 uses `pcre_study()` by default, and you **can't disable** it
* libmodsecurity3 uses `PCRE_JIT` if it supported by the pcre library, **there isn't any option** to tune the engine

limit match, limit match recursion
----------------------------------

* mod_security2 module uses the `PCRE_EXTRA_MATCH_LIMIT` and `PCRE_EXTRA_MATCH_LIMIT_RECURSION` (see this on same page) flags; if you don't set them befure you build the source, the default values will be 1500
* you can overwrite these values with `--enable-pcre-match-limit=N` and `--enable-pcre-match-limit-recursion=N`
* you can disable these features with `--enable-pcre-match-limit=no` and `--enable-pcre-match-limit-recursion=no`
* libmodsecurity3 doesn't supports these features at all

Here you can find more information about [pcre_study](https://www.pcre.org/original/doc/html/pcre_study.html), and [pcrejit](https://www.pcre.org/original/doc/html/pcrejit.html#SEC1). Also you can find some useful information about recursion limits [here](https://www.pcre.org/original/doc/html/pcreapi.html).

Capture patterns with OVECTOR
-----------------------------

If `pcre_exec()` matches a regular expression against a given subject string, then it collects the subpatterns. The offsets values will stored in a vector of integers - this is the OVECTOR. It's very important, that the length if this vector must be divide by 3 (the correct values could be multiple of 3). It's important how long of this vector, this value determines the number of possible subpatterns.

Both versions uses a precompiled value, mod_security2 uses 30, libmodsecurity3 uses 900. You can change these values only if you modify the source and recompile it again.

These informations are very important to understand, why and how works the tools.

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

```bash
echo "(?i:random_regular_expression)" > pattern1.txt
echo "random_subject" > subject1.txt
src/pcre4msc2 pattern1.txt subject1.txt
pattern1.txt - Time elapsed: 0.00nnn, matched: not matched, regex error: NOMATCH
```

which means the subject doesn't matches with the regex.

`src/pcre4msc3` works as same way.

Extra options
-------------

Based on the [information](#Useful%20information':ignore') above, now let's see the possible options for the tools.

 
| option | parameter needed | meaning         |  pcre4msc2 |    pcre4msc3 |
|-------:|-----------------:|----------------:|-----------:|-------------:|
|   `-j` | no               | use jit         |   supports | not supports |
|   `-s` | no               | ignore study    |   supports | not supports |
|   `-n` | yes              | number of runs  |   supports | supports     |
|   `-m` | yes              | mathch limit    |   supports | not supports |
|   `-r` | yes              | match lim. rec. |   supports | not supports |

and `-h` of course.

Examples
--------

Here are some examples:

(Note, that I assumed you have a file with the content for subject.)

**Run a tests with all used regular expression with a pattern.**

```bash
for d in `ls -1 data/9*.txt`; do src/pcre4msc2 ${d} /path/to/subject.txt; done
```

**Same tests but run it for libmodsecurity3 'engine'.**

```bash
for d in `ls -1 data/9*.txt`; do src/pcre4msc3 ${d} /path/to/subject.txt; done
```

**Same tests but use JIT.**

```bash
for d in `ls -1 data/9*.txt`; do src/pcre4msc2 -j ${d} /path/to/subject.txt; done
```

Note, see the difference in case of some patterns - it's awesome performance increase.

**Run a test N times**
```bash
src/pcre4msc3 -n 5 path/to/pattern path/to/subject
```

**Change the match and/or recursion limit**
```bash
src/pcre4msc2 -m 4000 -r 4000 path/to/pattern path/to/subject
```

Enjoy.