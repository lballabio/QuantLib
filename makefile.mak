
#
# main makefile for QuantLib under Borland C++
#
# $Id$
# $Source$
# $Log$
# Revision 1.14  2001/05/16 09:57:26  lballabio
# Added indexes and piecewise flat forward curve
#
# Revision 1.13  2001/05/16 08:35:45  nando
# reverted "make python" to old style, not ready for distutils with borland compiler
#
# Revision 1.12  2001/05/15 16:10:01  nando
# updated all PyWrap depencencies to PyQuantLibWrap
#

.autodepend
.silent

# Directories
!if "$(QL_DIR)" == ""
!message Set the QL_DIR environment variable to the absolute
!message path of your QuantLib installation before running $(MAKE).
!message
!error terminated
!endif

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

# QuantLib library
install::
    if exist "$(QL_DIR)\Include\ql" rmdir /S /Q "$(QL_DIR)\Include\ql"
    xcopy Include\ql\*.hpp "$(QL_DIR)\Include\ql" /S /I
    if exist "$(QL_DIR)\lib\Win32\VisualStudio" rmdir /S /Q "$(QL_DIR)\lib\Win32\VisualStudio"
    xcopy lib\Win32\VisualStudio\*.lib "$(QL_DIR)\lib\Win32\VisualStudio" /S /I
    if exist "$(QL_DIR)\lib\Win32\Borland" rmdir /S /Q "$(QL_DIR)\lib\Win32\Borland"
    xcopy lib\Win32\Borland\*.lib "$(QL_DIR)\lib\Win32\Borland" /S /I

# Python module
python::
    cd Python
    $(MAKE) Python
    # python setup.py build --compiler
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
