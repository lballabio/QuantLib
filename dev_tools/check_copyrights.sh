#!/bin/bash
#
# Run this from the main QuantLib directory

grep -r --include='*.[hc]pp' "Copyright (C)" * | ./dev_tools/collect_copyrights.py

