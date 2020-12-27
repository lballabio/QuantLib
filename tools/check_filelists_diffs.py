#!/usr/bin/python

import sys

inputs = [
    ("ql.cmake.diff", "ql/CMakeLists.txt"),
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


for diffs, target in inputs:
    with open(diffs) as f:
        for line in f:
            if line.startswith("< "):
                print("%s contains extra %s" % (target, format(line)))
                result = 1
            if line.startswith("> "):
                print("%s doesn't contain %s" % (target, format(line)))
                result = 1

sys.exit(result)
