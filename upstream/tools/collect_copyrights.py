#!/usr/bin/python
# coding=utf-8-unix

import re
import sys

regex1 = re.compile(r"Copyright \(C\) ([0-9]{4}-[0-9]{4}) (.+)$")
regex2 = re.compile(r"Copyright \(C\) (([0-9]{4})(, [0-9]{4})*) (.+)$")

copyrights = {}
errors = False

for line in sys.stdin:
    m1 = regex1.search(line)
    m2 = regex2.search(line)
    if m1 is None and m2 is None:
        sys.stderr.write("Could not parse '%s'\n" % line.strip())
        errors = True
    if m1:
        first, last = [int(y) for y in m1.groups()[0].split("-")]
        years = range(first, last + 1)
        owner = m1.groups()[-1].strip()
    elif m2:
        years = [int(y) for y in m2.groups()[0].split(", ")]
        owner = m2.groups()[-1].strip()
    s = copyrights.get(owner, set())
    for y in years:
        s.add(y)
    copyrights[owner] = s

if errors:
    exit(1)

for owner in copyrights:
    s = copyrights[owner]
    l = [y for y in s]
    l.sort()
    copyrights[owner] = l

copyrights = [
    (years, owner)
    for owner, years in copyrights.items()
    # some false positives
    if years[0] >= 2000 and "Monte Carlo Methods in Finance" not in owner
]
copyrights.sort()

lines = []
last_year = None
for years, owner in copyrights:
    # use blank lines to separate years
    if last_year is not None and years[0] != last_year:
        lines.append("")
    last_year = years[0]
    lines.append("Copyright (C) %s %s" % (", ".join([str(y) for y in years]), owner))

print(
    """
%s

QuantLib includes code taken from Peter Jäckel's book "Monte Carlo
Methods in Finance".

QuantLib includes software developed by the University of Chicago,
as Operator of Argonne National Laboratory.

QuantLib includes a set of numbers provided by Stephen Joe and Frances
Kuo under a BSD-style license.


Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

    Neither the names of the copyright holders nor the names of the QuantLib
    Group and its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE."""
    % ("\n".join(lines))
)
