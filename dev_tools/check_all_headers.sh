#!/bin/bash

# execute this script from the root QuantLib directory

find ./ql -name *.hpp \
| grep -v '/core\.hpp$' | grep -v '/all\.hpp$' | grep -v 'config' \
| xargs -n 1 ./dev_tools/check_header.py

