#!/bin/bash

echo Checking $1
clang-tidy -quiet -fix $1 -- -std=c++03 -I$PWD

