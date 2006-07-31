#!/usr/bin/python
# run this from the QuantLib base directory

import os, string

options = [
    "-r",                         # show revision numbers
    "-f ChangeLog.txt",           # output file
    "-W 180",                     # unify commits within 3 mins
    "-U dev_tools/developers",    # developer e-mails
    "-P",                         # no empty log messages
    "--no-hide-branch-additions", # show files added on branch
    "-S"                          # blank line between header and log
]

try:
    branch = None
    tag = open('CVS/Tag')
    for line in tag:
        line = line.strip()
        if line.startswith('T'):
            branch = line[1:]
except:
    pass

if branch:
    options.append('-F %s' % branch) # only show messages on or before branch

os.system('cvs2cl %s' % string.join(options))

