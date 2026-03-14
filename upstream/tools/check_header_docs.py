#!/usr/bin/python -u

import multiprocessing as mp
import sys
import os
import re

regex = re.compile(r"\\file +([a-z_]+\.hpp)")


def check(header):
    with open(header) as f:
        lines = f.readlines()
    for l in lines:
        m = regex.search(l)
        if m:
            documented = m.groups()[0]
            basename = os.path.basename(header)
            if documented == basename:
                return 0
            else:
                print(
                    f"Wrong file name ({documented}) in Doxygen \\file directive in {header}",
                    file=sys.stderr,
                )
                return 1

    return 0


if __name__ == "__main__":
    headers = [line.strip() for line in sys.stdin.readlines()]
    errors = sum(check(h) for h in headers)
    sys.exit(errors)
