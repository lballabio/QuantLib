#!/bin/bash
#
# Run this from the main QuantLib directory

grep -r --include='*.[hc]pp' "^ *Copyright (C)" * | ./tools/collect_copyrights.py > LICENSE.TXT

