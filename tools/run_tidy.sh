#!/bin/bash

echo Checking $1
clang-tidy-9 -quiet -fix $1 -- -std=c++03 -I$PWD

