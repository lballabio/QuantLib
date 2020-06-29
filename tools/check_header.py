#!/usr/bin/python

import os
import sys
import tempfile

errors = 0

for line in sys.stdin.readlines():
    header = line.strip()

    main = (
        r"""
#include <%s>
int main() {
    return 0;
}
"""
        % header
    )

    fd, fname = tempfile.mkstemp(suffix=".cpp", dir=".", text=True)
    os.write(fd, main)
    os.close(fd)

    print("Checking %s" % header)
    command = "g++ -c -Wno-unknown-pragmas -Wno-deprecated-declarations -I. %s -o /dev/null" % fname
    code = os.system(command)
    if code != 0:
        errors += 1

    os.unlink(fname)

sys.exit(errors)
