#!/usr/bin/python -u

"""Compile QuantLib headers individually to verify self-contained includes.

Header paths come from the repository file list on standard input, and the
compiler command comes from the build environment's ``CXX`` setting.  The
command is split into arguments and run without a shell, so neither input is
interpreted as shell syntax.  This supports compiler wrappers such as
``CXX="ccache g++"`` but intentionally does not support pipes, redirection, or
other shell expressions in ``CXX``.
"""

import multiprocessing as mp
import os
import shlex
import shutil
# Compiling headers requires invoking the compiler configured by the build.
import subprocess  # nosec B404
import sys
from typing import List


def check(header: str) -> int:
    """Compile one header as a translation unit and return its error count."""
    cxx = os.environ.get("CXX", "g++")
    source_file = header + ".cpp"
    object_file = header + ".o"
    command = shlex.split(cxx) + [
        "-std=c++17",
        "-c",
        "-Wno-unknown-pragmas",
        "-Wall",
        "-Werror",
        "-I.",
        source_file,
        "-o",
        object_file,
    ]
    shutil.copy(header, source_file)
    print(" ".join(shlex.quote(argument) for argument in command), file=sys.stderr)
    try:
        # Bandit B603 flags every subprocess call, even with shell=False.  CXX is
        # supplied by the build configuration and header paths come from the
        # repository file list; passing both as arguments prevents shell injection.
        code = subprocess.run(command, check=False).returncode  # nosec B603
    finally:
        for filename in (source_file, object_file):
            try:
                os.remove(filename)
            except FileNotFoundError:
                pass
    if code != 0:
        print(f"Errors while checking {header}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    headers: List[str] = [line.strip() for line in sys.stdin.readlines()]
    with mp.Pool(processes=mp.cpu_count()) as pool:
        errors = sum(pool.map(check, headers))
    sys.exit(errors)
