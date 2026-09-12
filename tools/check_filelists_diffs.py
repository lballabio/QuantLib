#!/usr/bin/python

"""Report inconsistencies between generated and maintained source-file lists."""

import sys
from typing import Tuple

INPUTS: Tuple[Tuple[str, str], ...] = (
    ("ql.dist.diff", "ql/Makefile.am"),
    ("test-suite.dist.diff", "test-suite/Makefile.am"),
    ("ql.cmake.diff", "ql/CMakeLists.txt"),
    ("test-suite.cmake.diff", "test-suite/CMakeLists.txt"),
    ("ql.vcx.diff", "QuantLib.vcxproj"),
    ("ql.vcx.filters.diff", "QuantLib.vcxproj.filters"),
    ("test-suite.vcx.diff", "test-suite/testsuite.vcxproj"),
    ("test-suite.vcx.filters.diff", "test-suite/testsuite.vcxproj.filters"),
)


def describe_file(line: str) -> str:
    """Describe the source file referenced by a diff output line."""
    filename = line[2:].strip()
    if filename.endswith(".hpp"):
        return f"header file {filename}"
    if filename.endswith(".cpp"):
        return f"source file {filename}"
    return f"file {filename}"


CYAN = "\033[96m"
RED = "\033[91m"
GREEN = "\033[92m"
BOLD = "\033[1m"
RESET = "\033[0m"


def main() -> int:
    """Print file-list differences and return a process exit status."""
    result = 0
    heading = "\n=============================== RESULTS ================================\n"
    print(BOLD + CYAN + heading + RESET)

    for diffs, target in INPUTS:
        with open(diffs, encoding="utf-8") as diff_file:
            for line in diff_file:
                if line.startswith("< "):
                    print(RED + f"{target} contains extra {describe_file(line)}" + RESET)
                    result = 1
                if line.startswith("> "):
                    print(RED + f"{target} doesn't contain {describe_file(line)}" + RESET)
                    result = 1

    if result == 0:
        print(GREEN + "All clear." + RESET)
    return result


if __name__ == "__main__":
    sys.exit(main())
