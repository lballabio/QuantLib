#!/bin/bash
# run this from the QuantLib base directory

OPTIONS="-r"                               # show revision numbers
OPTIONS="$OPTIONS -f ChangeLog.txt"        # output file
OPTIONS="$OPTIONS -W 180"                  # unify commits within 3 mins
OPTIONS="$OPTIONS -U dev_tools/developers" # developer e-mails
OPTIONS="$OPTIONS -P"                      # no empty log messages
OPTIONS="$OPTIONS -S"                      # blank line between header and log

cvs2cl $OPTIONS 

