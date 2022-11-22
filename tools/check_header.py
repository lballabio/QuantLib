#!/usr/bin/python -u

import os
import sys

errors = 0

for line in sys.stdin.readlines():
    header = line.strip()

    print("Checking %s" % header)
    command = "g++ -c -Wno-unknown-pragmas -Wall -Werror -I. %s -o /dev/null" % header
    code = os.system(command)
    if code != 0:
        print("Errors while checking %s" % header)
        errors += 1

sys.exit(errors)
