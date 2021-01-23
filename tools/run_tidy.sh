#!/bin/bash

echo Checking $1
clang-tidy-10 -quiet -fix $1 -- -std=c++11 -I$PWD -DQL_USE_STD_SHARED_PTR=1 -DQL_USE_STD_FUNCTION=1 -DQL_USE_STD_TUPLE=1

