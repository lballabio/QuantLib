
#
# makefile for QuantLib Examples under Borland C++
#
# $Id$
# $Source$
# $Log$
# Revision 1.1  2001/08/22 17:57:49  nando
# Examples compiles under borland
# added borland makefile
#

.autodepend
.silent

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
