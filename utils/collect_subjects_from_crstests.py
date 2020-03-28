#!/usr/bin/env python3

import yaml
import sys
import urllib.parse
import os.path

if len(sys.argv) < 3:
    print("Argument missing!")
    print("Use: %s /path/to/crs/tests/RULE/test.yaml /path/to/export/dir" % (sys.argv[0]))
    sys.exit(-1)

fname = sys.argv[1]
outdir = sys.argv[2]

try:
    with open(fname) as file:
        if yaml.__version__ >= "5.1":
            data = yaml.load(file, Loader=yaml.FullLoader)
        else:
            data = yaml.load(file)
except:
    print("Exception catched - ", sys.exc_info())
    sys.exit(-1)

try:
    for t in data['tests']:
        ts = t['stages'][0]['stage']
        if 'log_contains' in ts['output']:
            if 'data' in ts['input']:
                oname = os.path.join(outdir, t['test_title'] + ".txt")
                try:
                    with open(oname, 'w') as ofile:
                        ofile.write(urllib.parse.unquote_plus(ts['input']['data']))
                except:
                    print("Exception catched - ", sys.exc_info())
                    sys.exit(-1)    

except:
    print("Exception catched - ", sys.exc_info())
    sys.exit(-1)