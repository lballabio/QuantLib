#!/usr/bin/python
# run this from the QuantLib base directory

import os, string

options = [
    "-r",                        # show revision numbers
    "-f ChangeLog.txt",          # output file
    "-W 180",                    # unify commits within 3 mins
    "-U dev_tools/developers",   # developer e-mails
    "-P",                        # no empty log messages
    "-S"                         # blank line between header and log
]

os.system('cvs2cl %s' % string.join(options))

