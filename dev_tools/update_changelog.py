#!/usr/bin/python
# run this from the QuantLib base directory

import os, string

options = [
    "-i",                         # show revision numbers
    "-f ChangeLog.txt",           # output file
    "--authors=dev_tools/developers",    # developer e-mails
    "--break-before-msg=2",       # blank line between header and log
    "--linelen=78"                # maximum length of an output line
]

os.system('svn2cl %s' % string.join(options))

