# How to collect the `@rx` arguments from ruleset?

In this directory, you can find a tool named `collect_rx_args.py`.

With helps of the script, you can collect the arguments of all `@rx` operators.

Install
=======

This tool uses the [msc_pyparser](https://github.com/digitalwave/msc_pyparser). You can install it from the link (use `git clone` or download the zip), or through pip:

```python
pip install msc-pyparser
```

or

```python
pip install -r requirements.txt
```

How does it work?
=================

To start the process, type:

`./collect_rx_args.py /PATH/TO/RULESET ../data`

where the `/PATH/TO/RULESET` is the source dir for your rule set, `../data` is the destination - which is the part of this package. To be sure, before the run of this tool, remove the content of `../data` directory.

What does it do?
================

The scripts reads all *.conf files in the source directory, tries to parse them and builds the AST (Abstract Syntax Tree) in memory (note, that it doesn't write the parsed data into your disk). When it's done, then iterates over the rules (still in memory), and checks if it uses `@rx` operator. It it does, then writes it into a file. The name of file will be the `id` of the rule, and a suffix: `_N` where `N` starts from 1.

Consider, there is a `chained` rule, and more parts uses `@rx`. In this case, you will get many files with name of `id` and with name `_1`, `_2`, ... For the real example, if you use CRS (CoreRuleSet), then the rule with `id` 9001184 contains 3 `@rx` rule, then you will get 3 files:

```bash
9001184_1.txt
9001184_2.txt
9001184_3.txt
```

All files contains the regular expression in one line. All line are terminates with the EOL, then the new line with EOF.

# How to collect the subjects for patterns from CRS regression tests?

In this directory, you can find a tool named `collect_subjects_from_crstests.py`.

With helps of the script, you can collect the test strings for `@rx` operators from the CRS tests files.

Install
=======

This tool needs only Python YAML library - install it through your package manager or from pypi.

How does it work?
=================

To start the process, type:

`./collect_subjects_from_crstests.py /PATH/TO/RULESET/tests/regression/.../9NNNN.yaml ../subjects`

where the `/PATH/TO/RULESET/tests/regression/.../9NNNN.yaml` is the yaml test file of your rule set, `../subjects` is the destination - which is the part of this package. To be sure, before the run of this tool, remove the content of `../subjects` directory.

Examples:

`./collect_subjects_from_crstests.py /pat/to/source/of/owasp-modsecurity-crs/tests/regression/tests/REQUEST-942-APPLICATION-ATTACK-SQLI/942210.yaml ../subjects`

What does it do?
================

Actually this tool is in very initial state. I only tested it with 942210 to collect the patterns. The result will so many subject for the tests:

```bash
942210-1.txt
...
942210-81.txt
```

All files contains a subject which used the `ftw` tool for the regression tests. You can use to check the pattern with this command:

```bash
for s in `ls -1 subjects/*.txt`; do src/pcre4msc2 data/942210_1.txt ${s}; done
for s in `ls -1 subjects/*.txt`; do src/pcre4msc2 -j data/942210_1.txt ${s}; done
for s in `ls -1 subjects/*.txt`; do src/pcre4msc3 data/942210_1.txt ${s}; done
```


