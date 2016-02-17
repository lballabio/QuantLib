#!/bin/bash

# execute this script from the root QuantLib directory

find ./ql -name *.hpp \
| grep -v '/all\.hpp$' | grep -v '/auto_link\.hpp$' \
| grep -v 'config' | grep -v '/mathconstants\.hpp$'\
| sort | xargs -n 1 ../dev_tools/check_header.py

