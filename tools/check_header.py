#!/usr/bin/python

import os, sys, tempfile

if len(sys.argv) != 2:
    print 'Usage: %s <header file>' % sys.argv[0]
    sys.exit()

header = sys.argv[1]

main = r"""
#include <%s>
int main() {
    return 0;
}
""" % header

fd,fname = tempfile.mkstemp(suffix = '.cpp', dir = '.', text = True)
os.write(fd,main)
os.close(fd)

print 'Checking %s' % header
command = 'g++ -c -I. %s -o /dev/null' % fname
os.system(command)

os.unlink(fname)

