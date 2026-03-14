#!/usr/bin/python -u

import multiprocessing as mp
import os
import sys
import shutil


def check(header):
    cxx = os.environ.get("CXX", "g++")
    source_file = header + ".cpp"
    shutil.copy(header, source_file)
    object_file = header + ".o"
    command = f"{cxx} -std=c++17 -c -Wno-unknown-pragmas -Wall -Werror -I. {source_file} -o {object_file}"
    print(command, file=sys.stderr)
    code = os.system(command)
    try:
        os.remove(source_file)
        os.remove(object_file)
    except:
        pass
    if code != 0:
        print(f"Errors while checking {header}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    headers = [line.strip() for line in sys.stdin.readlines()]
    with mp.Pool(processes=mp.cpu_count()) as pool:
        errors = sum(pool.map(check, headers))
    sys.exit(errors)
