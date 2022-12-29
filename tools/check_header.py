#!/usr/bin/python -u

import multiprocessing as mp
import os
import sys


def check(header):
    print("Checking %s" % header, file=sys.stderr)
    command = "g++ -std=c++14 -c -Wno-unknown-pragmas -Wall -Werror -I. %s -o /dev/null" % header
    code = os.system(command)
    if code != 0:
        print("Errors while checking %s" % header, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    headers = [line.strip() for line in sys.stdin.readlines()]
    with mp.Pool(processes=mp.cpu_count()) as pool:
        errors = sum(pool.map(check, headers))
    sys.exit(errors)
