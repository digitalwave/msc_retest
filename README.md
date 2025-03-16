# msc_retest

Welcome to the `msc_retest` documentation.

Description
===========

This tool compiles two binaries: `pcre4msc2` and `pcre4msc3`. The binaries emulates the behaviors of regex engines (both PCRE2 - the **NEW** - and PCRE3  - the old - versions) in mod_security2 (Apache module) and the libmodsecurity3. Please note, that the default PCRE engine is **PCRE2**. If you want to use the old PCRE engine, you should pass `-1` option to the tool. With these programs, you can check the evaluation time and result of every regular expressions with any random (including very extreme long) input. Both of them (regex pattern, input subject) needs to exists in two separated files, and you can pass them as argument. Subject can be passed through stdin, if you give '-' for subject file, eg:

```bash
echo -n "arg=../../../etc/passwd&foo=var" | src/pcre4msc2 regexes/930110_1.txt -
regexes/930110_1.txt - time elapsed: 0.000011753, match value: SUBJECT MATCHED 1 TIME
```
or just simple leave it:

```bash
echo -n "arg=../../../etc/passwd&foo=var" | src/pcre4msc3 regexes/930110_1.txt
regexes/930110_1.txt - time elapsed: 0.000006, match value: SUBJECT MATCHED 1 TIME
```

The source tree contains some extra directories: under the `regexes/` you can find all of the regular expressions, what CRS uses. The files contains the id of the rule, and a suffix (there are some chained rules, where more parts uses `@rx`).

The `utils/` directory contains a python tool, which helps to re-generate the regex's above. It uses `msc_pyparser` - for more information, see the README.md in that directory.

Useful information
==================

There are so many difference between mod_security2 and libmodsecurity3 implementations:

Parsing:
--------

* mod_security2 is an Apache module, so it uses the available functions, including parsing of configuration files. Apache config parser strips the extra `\` (backslash) characters and escaped " if it is inside of quoted string. Eg. if the rule in the config contains a sub-string like `\\\\'`, then it will evaluated as `\\'`, if there is a pattern `\"` in a quoted string, then it will be `"`.
* libmodsecurity3 uses an own parser, and it doesn't make any strip methods. But it does if the rule defined in the configuration file of the web server.

This mean if the rule has been read from the external file (like CRS), that will not be stripped, but if there is an inline rule (see [this](https://github.com/owasp-modsecurity/ModSecurity-nginx#modsecurity_rules)), then it will.

This strip function is implemented in `pcre4msc2` code itself.

pcre_study(), JIT
-----------------

* mod_security2 module uses `pcre_study()` if it's available, but **you can disable** it (with `--enable-pcre-study=no`)
* mod_security2 module uses `PCRE_JIT` if it's available **and you enabled it in build time** (with `--enable-pcre-jit`)
* in case of `pcre4msc2` (which emulates mod_security2) you can force JIT with `-j`, it it's available
* libmodsecurity3 uses `pcre_study()` by default, and you **can't disable** it
* libmodsecurity3 uses `PCRE_JIT` if it supported by the pcre library, **there isn't any option** to tune the engine

limit match, limit match recursion
----------------------------------

* mod_security2 module uses the `PCRE_EXTRA_MATCH_LIMIT` and `PCRE_EXTRA_MATCH_LIMIT_RECURSION` (see this on same page) flags; if you don't set them before you build the source, the default values will be 1500
* you can overwrite these values with `--enable-pcre-match-limit=N` and `--enable-pcre-match-limit-recursion=N`
* you can disable these features with `--enable-pcre-match-limit=no` and `--enable-pcre-match-limit-recursion=no`
* libmodsecurity3 doesn't supports these features at all

Here you can find more information about [pcre_study](https://www.pcre.org/original/doc/html/pcre_study.html), and [pcrejit](https://www.pcre.org/original/doc/html/pcrejit.html#SEC1). Also you can find some useful information about recursion limits [here](https://www.pcre.org/original/doc/html/pcreapi.html).

Capture patterns with OVECTOR
-----------------------------

If `pcre_exec()` matches a regular expression against a given subject string, then it collects the sub-patterns. The offsets values will stored in a vector of integers - this is the OVECTOR. It's very important, that the length if this vector must be divide by 3 (the correct values could be multiple of 3). It's important how long of this vector, this value determines the number of possible sub-patterns.

Both versions uses a precompiled value, mod_security2 uses 30, libmodsecurity3 uses 900. You can change these values only if you modify the source and recompile it again.

These information are very important to understand, why and how works the tools.

Requirements
============

To compile the code you need the pcre2 library (`libpcre2-dev` package on Debian - this is the new version), and the autotools. You can add the old engine too, then you need `libpcre3-dev`, and tell to `configure` script that you want to use it: `./configure --with-old-pcre`.

Build
=====

Download and unpack the source, and

```
autoreconf --install
./configure --with-old-pcre
make
```

Please note that you can't disable `pcre2` support.

Use
===

A quick how to:

```bash
echo -n "this is what is this" > subject.txt
echo -n "(?:is)" > pattern.txt
src/pcre4msc2 pattern.txt subject.txt
pattern.txt - time elapsed: 0.000017264, match value: SUBJECT MATCHED 1 TIME
```
Please note that `-n` option above supresses the `\n` character at the end of the lines.

This means the subject matched with the regex, and the time taken 0.000017264 sec (17.264 usec = 17 264 nanosecs).

**Note: all measured time are in nanoseconds.**

You can check the exit status of program:
```bash
echo $?
0
```

Now check the elapsed time in the output - that's a very important value. That shows you how long the regex engine runs. You can pass an argument to check that this time below your limit or not. For this, try to run:

```bash
src/pcre4msc2 -t 0.000001 pattern.txt subject.txt
pattern.txt - time elapsed: 0.000016613, match value: SUBJECT MATCHED 1 TIME
echo $?
1
```

As you can see, the time limit (argument of `-t`) is 0.000001 second (1 ms), it's less than the running time. If this is greater than your limit, the exit codes will 1. Try it again with a greater value:

```bash
src/pcre4msc2 -t 0.0001 pattern.txt subject.txt
pattern.txt - time elapsed: 0.000017574, match value: SUBJECT MATCHED 1 TIME
echo $?
0
```

`src/pcre4msc3` works as same way.

Extra options
-------------

Based on the [information](#Useful%20information':ignore') above, now let's see the possible options for the tools.

 
| option | parameter needed | meaning          |  pcre4msc2    |     pcre4msc3 |
|-------:|-----------------:|-----------------:|--------------:|--------------:|
|   `-j` | no               | use jit          |     supported | not supported |
|   `-s` | no               | ignore study     |     supported | not supported |
|   `-n` | yes              | number of runs   |     supported |     supported |
|   `-m` | yes              | match limit      |     supported | not supported |
|   `-r` | yes              | match lim. rec.  |     supported | not supported |
|   `-t` | yes              | exec. time lim.  |     supported |     supported |
|   `-d` | no               | show details     |     supported |     supported |
|   `-1` | no               | use pcre3 if av. |     supported |     supported |

and `-h` of course.

Examples
--------

Here are some examples:

(Note, that I assumed you have a file with the content for subject.)

**Run a tests with all used regular expression with a pattern.**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc2 ${d} /path/to/subject.txt; done
```

**Same test but filter the rules with extra long times.**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc2 ${d} /path/to/subject.txt; done | grep "\([1-9][0-9]\|[1-9]\)\.[0-9]"
```

**Same tests but run it for libmodsecurity3 'engine'.**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc3 ${d} /path/to/subject.txt; done
```

**Use the filter here too.**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc3 ${d} /path/to/subject.txt; done | grep "\([1-9][0-9]\|[1-9]\)\.[0-9]"
```

**Collect regex's where the run time over your limit (0.00001 sec)**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc2 -t 0.00001 -j ${d} /path/to/subject.txt; if [ $? -ne 0 ]; then echo "Time limit exceeded: ${d}"; fi; done | grep "Time limit"
```

**Same tests but use JIT.**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc2 -j ${d} /path/to/subject.txt; done
```

**Check again and compare the long times with 2nd example.**

```bash
for d in `ls -1 regexes/9*.txt`; do src/pcre4msc2 -j ${d} /path/to/subject.txt; done | grep "\([1-9][0-9]\|[1-9]\)\.[0-9]"
```

Note, see the difference in case of some patterns - it's awesome performance increase.

**Run a test N times**
```bash
src/pcre4msc3 -n 5 path/to/pattern path/to/subject
path/to/pattern - time elapsed: 0.000016934, match value: SUBJECT MATCHED 1 TIME
path/to/pattern - time elapsed: 0.000002035, match value: SUBJECT MATCHED 1 TIME
path/to/pattern - time elapsed: 0.000001125, match value: SUBJECT MATCHED 1 TIME
path/to/pattern - time elapsed: 0.000001013, match value: SUBJECT MATCHED 1 TIME
path/to/pattern - time elapsed: 0.000001029, match value: SUBJECT MATCHED 1 TIME
Num of values: 5
         Mean: 000.000004427
       Median: 000.000002035
          Min: 000.000001013
          Max: 000.000016934
        Range: 000.000015921
Std deviation: 000.000006265
```

**Statistics about run time if you run the tools with `-n`**

Take a look to this example:

```bash
$ src/pcre4msc2 -j -n 10 pattern.txt subject.txt
pattern.txt - time elapsed: 0.000003900, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000348, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000215, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000188, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000196, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000190, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000191, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000191, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000192, match value: SUBJECT MATCHED 1 TIME
pattern.txt - time elapsed: 0.000000190, match value: SUBJECT MATCHED 1 TIME
Num of values: 10
         Mean: 000.000000580
       Median: 000.000000194
          Min: 000.000000188
          Max: 000.000003900
        Range: 000.000003712
Std deviation: 000.000001108
```

All time values are in nanoseconds. Below the last line you can see the quick summary:

 * `Num of values` - how many times did you run
 * `Mean` - the average value of all measured times
 * `Median` - the median value of all measured times
 * `Min`, `Max` - the minimum and maximum values of all measured times
 * `Range` - the distance of max and min values
 * `Std deviation` - the [standard deviation](https://en.wikipedia.org/wiki/Standard_deviation)

**Note: the summary will displayed only if you run the tool with `-n N`, where `N` > 1.**

**Change the match and/or recursion limit**
```bash
src/pcre4msc2 -m 4000 -r 4000 path/to/pattern path/to/subject
```

[See the difference between the modsec_v2 **without** JIT, modsec v2 **with** JIT, modsec v3, and old modsec v3 matches algorithm](#diff-between-methods)
```bash
echo -n "this is what is this" > subject.txt
echo -n "(?:is)" > pattern.txt
# v2
src/pcre4msc2 pattern.txt subject.txt
pattern.txt - time elapsed: 0.000015, match value: SUBJECT MATCHED 1 TIME
# v2 with JIT
src/pcre4msc2 -j pattern.txt subject.txt
pattern.txt - time elapsed: 0.000007, match value: SUBJECT MATCHED 1 TIME
# v3 original
src/pcre4msc3 pattern.txt subject.txt
pattern.txt - time elapsed: 0.000014, match value: SUBJECT MATCHED 4 TIMES
# v3 modified
src/pcre4msc3 -f pattern.txt subject.txt
pattern.txt - time elapsed: 0.000010, match value: SUBJECT MATCHED 1 TIME
```

Of course, for a more precision result you have to run the tests with `-n 10`, or with more sophisticated pattern and subject.

**Show the details with created files above (pattern.txt, subject.txt)***
```bash
src/pcre4msc2 -d pattern.txt subject.txt

PCRE
====
NEW

RAW pattern:
============
is

ESCAPED pattern:
================
is

SUBJECT:
========
this is what is this

JIT:
====
available but disabled

STUDY:
======
enabled

MATCH LIMIT:
============
1000

MATCH LIMIT RECURSION:
======================
1000

RESULT:
=======
pattern.txt - time elapsed: 0.000019, match value: SUBJECT MATCHED 1 TIME

CAPTURES:
=========
th*is* is what is this

OVECTOR:
========
[2, 4]

```

where the firs is printed with green color.

**Show the details with pattern what contains escape***
```bash
echo -n "\"//onerror=\"" > subject2.txt
src/pcre4msc2 -d regexes/941120_1.txt subject2.txt

PCRE
====
NEW

RAW pattern:
============
(?i)[\s\"'`;\/0-9=\x0B\x09\x0C\x3B\x2C\x28\x3B]+on[a-zA-Z]+[\s\x0B\x09\x0C\x3B\x2C\x28\x3B]*?=

ESCAPED pattern:
================
(?i)[\s"'`;\/0-9=\x0B\x09\x0C\x3B\x2C\x28\x3B]+on[a-zA-Z]+[\s\x0B\x09\x0C\x3B\x2C\x28\x3B]*?=

SUBJECT:
========
"//onerror="

JIT:
====
available but disabled

STUDY:
======
enabled

MATCH LIMIT:
============
1000

MATCH LIMIT RECURSION:
======================
1000

RESULT:
=======
regexes/941120_1.txt - time elapsed: 0.000015, match value: SUBJECT MATCHED 1 TIME

CAPTURES:
=========
*"//onerror=*"

OVECTOR:
========
[0, 11]

```

As you can see, the escaped pattern is shorter than the original.

Note, the sub-string between the two asterisks is also highlighted.

**See the detailed output from the previous example with pcre4msc3**
```bash
src/pcre4msc3 -d pattern.txt subject.txt

PCRE:
=====
NEW

PATTERN:
========
(?:is)

SUBJECT:
========
this is what is this

JIT:
====
avaliable and used

RESULT:
=======
pattern.txt - time elapsed: 0.000030633, match value: SUBJECT MATCHED 1 TIME

CAPTURES:
=========
this is what is this

OVECTOR:
========
[2, 4]


```

Issues, feature requests
------------------------

If you ran an unexpected behavior, found a bug, or have a feature request, just open an issue here, or drop an e-mail to us: modsecurity at digitalwave dot hu.
