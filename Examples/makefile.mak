
#
# makefile for QuantLib Examples under Borland C++
#
# $Id$

.autodepend
.silent

# Options
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif

# Primary target:
# QuantLib Examples
examples::
    cd DiscreteHedging
    $(MAKE)
    cd ..\Parities
    $(MAKE)
    cd ..\Swap
    $(MAKE)
    cd ..

# Clean up
clean::
    cd DiscreteHedging
    $(MAKE) clean
    cd ..\Parities
    $(MAKE) clean
    cd ..\Swap
    $(MAKE) clean
    cd ..
