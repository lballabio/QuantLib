
# $Id$
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
    cd Sources
    $(MAKE)
    cd ..

# Python module
python::
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

# Documentation
docs-all:
    cd Docs
    $(MAKE) all
    cd ..

docs-html:
    cd Docs
    $(MAKE) html
    cd ..

docs-pdf:
    cd Docs
    $(MAKE) pdf
    cd ..

docs-ps:
    cd Docs
    $(MAKE) ps
    cd ..


# Clean up
clean::
    cd Sources
    $(MAKE) clean
    cd ..
    cd Python
    $(MAKE) clean
    cd ..
    cd Docs
    $(MAKE) clean
    cd ..
