#!/bin/sh

# Run this to generate all the auto-generated files needed by the GNU
# configure program

# this is present in gsl
#libtoolize --automake

aclocal
# cppunit approach
#aclocal || \
#    aclocal -I /usr/share/aclocal

# this is absent in gsl
autoheader

automake --add-missing

autoconf

