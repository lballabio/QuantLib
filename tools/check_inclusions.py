#!/usr/bin/python3

import sys, re

if len(sys.argv) != 2:
    print(f"Usage: {sys.argv[0]} <file>")
    sys.exit()

filename = sys.argv[1]
print(f"Checking {filename}")

regex = re.compile("^#include.*<(.*)>")
ql_headers = []
boost_headers = []
std_headers = []
experimental_headers = []
source = open(filename)
for n, line in enumerate(source):
    match = regex.search(line)
    if match:
        (header,) = match.groups()
        line_num = n + 1
        if header.startswith("ql/"):
            ql_headers.append((header, line_num))
            if header.startswith("ql/experimental") and "internal" not in line:
                experimental_headers.append((header, line_num))
        elif header.startswith("boost/"):
            boost_headers.append((header, line_num))
        else:
            std_headers.append((header, line_num))
source.close()

# At least one QuantLib header muct be included (which, hopefully,
# ultimately leads to including ql/qldefines.hpp)
if not ql_headers:
    print(f"./{filename}:1: error: no QuantLib header included")
    sys.exit(1)

if "ql/experimental" not in filename:
    # files in core library can't include stuff in experimental
    if experimental_headers:
        for f, n in experimental_headers:
            print(f"./{filename}:{n}: error: experimental header '{f}' included")
        sys.exit(1)

for f, n in ql_headers:
    if f.endswith("/all.hpp"):
        print(f"./{filename}:{n}: error: generated header '{f}' included")
        sys.exit(1)

# All Boost headers must be included after QuantLib ones
last_ql_header = max([n for _, n in ql_headers])

for _, n in boost_headers:
    if n < last_ql_header:
        print(
            f"./{filename}:{n}: error: Boost header included before last QuantLib header"
        )
        sys.exit(1)


# All standard headers must be included after QuantLib and Boost ones
if boost_headers:
    last_boost_header = max([n for _, n in boost_headers])

for _, n in std_headers:
    if n < last_ql_header:
        print(
            f"./{filename}:{n}: error: standard header included before last QuantLib header"
        )
        sys.exit(1)
    if boost_headers and n < last_boost_header:
        print(
            f"./{filename}:{n}: error: standard header included before last Boost header"
        )
        sys.exit(1)
