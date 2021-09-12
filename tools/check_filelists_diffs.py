#!/usr/bin/python

import sys

inputs = [
    ("ql.dist.diff", "Some Makefile.am"),
    ("test-suite.dist.diff", "test-suite/Makefile.am"),
    #("ql.cmake.diff", "ql/CMakeLists.txt"),
    ("test-suite.cmake.diff", "test-suite/CMakeLists.txt"),
    ("ql.vcx.diff", "QuantLib.vcxproj"),
    ("ql.vcx.filters.diff", "QuantLib.vcxproj.filters"),
    ("test-suite.vcx.diff", "test-suite/testsuite.vcxproj"),
    ("test-suite.vcx.filters.diff", "test-suite/testsuite.vcxproj.filters"),
]

result = 0


def format(line):
    filename = line[2:].strip()
    if filename.endswith(".hpp"):
        return "header file %s" % filename
    elif filename.endswith(".cpp"):
        return "source file %s" % filename
    else:
        return "file %s" % filename


CYAN = "\033[96m"
RED = "\033[91m"
GREEN = "\033[92m"
BOLD = "\033[1m"
RESET = "\033[0m"

print(BOLD + CYAN + "\n=============================== RESULTS ================================\n" + RESET)

for diffs, target in inputs:
    with open(diffs) as f:
        for line in f:
            if line.startswith("< "):
                print(RED + "%s contains extra %s" % (target, format(line)) + RESET)
                result = 1
            if line.startswith("> "):
                print(RED + "%s doesn't contain %s" % (target, format(line)) + RESET)
                result = 1

if result == 0:
    print(GREEN + "All clear." + RESET)

sys.exit(result)
