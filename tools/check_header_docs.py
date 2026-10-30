#!/usr/bin/python -u

"""Check that header Doxygen file names match their actual file names."""

import os
import re
import sys
from typing import List

regex = re.compile(r"\\file +([a-z_]+\.hpp)")


def check(header: str) -> int:
    """Return whether a header contains an incorrect Doxygen file name."""
    with open(header, encoding="utf-8") as header_file:
        lines = header_file.readlines()
    for line in lines:
        match = regex.search(line)
        if match:
            documented = match.groups()[0]
            basename = os.path.basename(header)
            if documented == basename:
                return 0
            print(
                f"Wrong file name ({documented}) in Doxygen \\file directive in {header}",
                file=sys.stderr,
            )
            return 1

    return 0


if __name__ == "__main__":
    headers: List[str] = [line.strip() for line in sys.stdin.readlines()]
    errors = sum(check(h) for h in headers)
    sys.exit(errors)
