#!/bin/bash

# execute this script from the root QuantLib directory

find ./ql -name *.[ch]pp \
| grep -v '/qldefines\.hpp$' | grep -v '/auto_link\.hpp$' | grep -v 'config' \
| grep -v '/mathconstants\.hpp$' | grep -v '/quantlib\.hpp$' \
| grep -v '/all.hpp$' | grep -v '/primitivepolynomials\.hpp$' \
| sort | xargs -n 1 ./tools/check_inclusions.py

