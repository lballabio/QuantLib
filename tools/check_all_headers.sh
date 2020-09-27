#!/bin/bash

# execute this script from the root QuantLib directory

find ./ql -name *.hpp \
| grep -v '/all\.hpp$' | grep -v '/auto_link\.hpp$' | grep -v 'config' \
| sort | ./tools/check_header.py

