
# $Source$

#
# main makefile for QuantLib under Borland C++
#

.autodepend
.silent

# Options
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif

# Primary target:
# QuantLib library
quantlib::
    cd Win
    $(MAKE) 
    cd ..

# Python module
python:: quantlib
    cd Python
    $(MAKE) Python
    cd ..

# Install Python module
python-install::
    cd Python
    $(MAKE) install
    cd ..

# Test Python module
python-test::
    cd Python
    $(MAKE) test
    cd ..

# Clean up
clean::
    cd Win
    $(MAKE) clean
    cd ..
    cd Python
    $(MAKE) clean
    cd ..
