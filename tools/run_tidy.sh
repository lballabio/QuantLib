#!/bin/bash

echo Checking $1
clang-tidy-10 -quiet -fix $1 -- -std=c++11 -I$PWD

