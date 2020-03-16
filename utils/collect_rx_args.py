#!/usr/bin/env python3

import yaml
import sys
import re
import os, os.path
import msc_pyparser
from msc_pyparser import MSCUtils as u


class Check(object):
    def __init__(self, data, destdir):
        self.data = data
        self.destdir = destdir
        self.current_ruleid = 0
        self.last_ruleid = 0
        self.curr_lineno = 0
        self.chained = False
        self.chainlevel = 0

    def collectrx(self):
        self.chained = False
        patterns = []
        for d in self.data:
            if d['type'].lower() != "secrule":
                continue
            if d['operator'] == "@rx":
                patterns.append(d['operator_argument'])
            if "actions" in d:
                act_idx = 0
                if self.chained == True:
                    self.chained = False
                while act_idx < len(d['actions']):
                    a = d['actions'][act_idx]

                    self.curr_lineno = a['lineno']
                    if a['act_name'] == "id":
                        self.current_ruleid = int(a['act_arg'])

                    if a['act_name'] == "chain":
                        self.chained = True
                        self.chainlevel += 1

                    act_idx += 1

                # end of (chained) rule
                if self.chained == False:
                    if len(patterns) > 0:
                        pi = 1
                        print("Writing patterns from rule %d" % (self.current_ruleid))
                        for p in patterns:
                            with open("%s/%d_%d.txt" % (self.destdir, self.current_ruleid, pi), "w") as pfile:
                                pfile.write(p)
                                pi += 1
                    self.current_ruleid = 0
                    patterns = []

if len(sys.argv) < 3:
    print("Argument missing!")
    print("Use: %s /path/to/CRS/rules /path/to/export/regexes" % (sys.argv[0]))
    sys.exit(-1)

srcobj = sys.argv[1]
dstobj = sys.argv[2]

dt = u.getpathtype(dstobj)
if dt == u.UNKNOWN:
    print("Unknown dest path!")
    sys.exit(-1)
if dt == u.IS_FILE:
    print("Dest path is file!")
    sys.exit(-1)

st = u.getpathtype(srcobj)
if st == u.UNKNOWN:
    print("Unknown source path!")
    sys.exit()

configs = []
if st == u.IS_DIR:
    for f in os.listdir(srcobj):
        fp = os.path.join(srcobj, f)
        if os.path.isfile(fp) and os.path.basename(fp)[-5:] == ".conf":
            configs.append(fp)
if st == u.IS_FILE:
    configs.append(srcobj)

configs.sort()

for c in configs:
    print("Parsing CRS config: %s" % c)
    cname = os.path.basename(c)
    dname = cname.replace(".conf", ".yaml")

    try:
        with open(c) as file:
            data = file.read()
    except:
        print("Exception catched - ", sys.exc_info())
        sys.exit(-1)

    try:
        mparser = msc_pyparser.MSCParser()
        mparser.parser.parse(data)
    except:
        print(sys.exc_info()[1])
        sys.exit(-1)

    try:
        chk = Check(mparser.configlines, dstobj)
        chk.collectrx()
    except:
        print(sys.exc_info()[1])
        sys.exit(-1)
