#!/bin/bash

# execute this script from the root QuantLib directory

find ./ql -name *.[ch]pp \
| grep -v '/qldefines\.hpp$' | grep -v '/auto_link\.hpp$' | grep -v 'config' \
| xargs -n 1 ./dev_tools/check_inclusions.py

