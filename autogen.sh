#!/bin/sh

# Run this to generate all the auto-generated files needed by the GNU
# configure program

autoreconf --force --install

echo
echo Run the command './configure --help' for information on options
echo that can change the behavior of the library.

