
# main makefile for QuantLib under Borland C++
#
# $Id$


.autodepend
.silent

# Options
!ifdef DEBUG
    MAKE = $(MAKE) -DDEBUG
!endif

# Primary target:
# QuantLib library
quantlib::
    cd ql
    $(MAKE)
    cd ..\Examples
    $(MAKE) -DQL_DIR="..\.."
    cd ..

# the installation directive requires the QL_DIR environment variable to
# point to the installed version of QuantLib
inst::
    if exist "$(QL_DIR)\ql" rmdir /S /Q "$(QL_DIR)\ql"
    xcopy ql\*.hpp "$(QL_DIR)\ql" /S /I

    if exist "$(QL_DIR)\lib\Win32\VisualStudio" rmdir /S /Q "$(QL_DIR)\lib\Win32\VisualStudio"
    xcopy lib\Win32\VisualStudio\*.lib "$(QL_DIR)\lib\Win32\VisualStudio" /S /I

    if exist "$(QL_DIR)\lib\Win32\Borland" rmdir /S /Q "$(QL_DIR)\lib\Win32\Borland"
    xcopy lib\Win32\Borland\*.lib "$(QL_DIR)\lib\Win32\Borland" /S /I


# Documentation
docs-all:
    cd Docs
    $(MAKE) all
    cd ..

docs-html:
    cd Docs
    $(MAKE) html
    cd ..

docs-html-online:
    cd Docs
    $(MAKE) html-online
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
    cd ql
    $(MAKE) clean
    cd ..\Examples
    $(MAKE) clean
    cd ..\Docs
    $(MAKE) clean
    cd ..
