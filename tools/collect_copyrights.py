#!/usr/bin/python

import re, sys

regex1 = re.compile(r"Copyright \(C\) ([0-9]{4}-[0-9]{4}) (.+)$")
regex2 = re.compile(r"Copyright \(C\) (([0-9]{4})(, [0-9]{4})*) (.+)$")

copyrights = {}

for line in sys.stdin:
    m1 = regex1.search(line)
    m2 = regex2.search(line)
    if m1 is None and m2 is None:
        print line
        continue
    if m1:
        first, last = [ int(y) for y in m1.groups()[0].split('-') ]
        years = range(first, last+1)
        owner = m1.groups()[-1].strip()
    elif m2:
        years = [ int(y) for y in m2.groups()[0].split(', ') ]
        owner = m2.groups()[-1].strip()
    s = copyrights.get(owner,set())
    for y in years:
        s.add(y)
    copyrights[owner] = s

for owner in copyrights:
    s = copyrights[owner]
    l = [ y for y in s ]
    l.sort()
    copyrights[owner] = l

copyrights = [ (years,owner) for owner,years in copyrights.items() ]
copyrights.sort()

print 60*'-'
for years, owner in copyrights:
    print "Copyright (C)", ', '.join([str(y) for y in years]), owner

