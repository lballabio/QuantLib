
#
# makefile for QuantLib Examples under Borland C++
#
# $Id$

.autodepend
#.silent

# Options
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif

# Primary target:
# QuantLib Examples
examples::
    cd HedgingError
    $(MAKE)
    cd ..\Parities
    $(MAKE)
    cd ..

# Clean up
clean::
    cd HedgingError
    $(MAKE) clean
    cd ..\Parities
    $(MAKE) clean
    cd ..
