
# main makefile for QuantLib under Borland C++

.autodepend
#.silent

# MAKE Options
!ifdef __MT__
    MAKE = $(MAKE) -D__MT__
!endif
!ifdef _RTLDLL
    MAKE = $(MAKE) -D_RTLDLL
!endif
!ifdef _DEBUG
    MAKE = $(MAKE) -D_DEBUG
!endif
!ifdef SAFE
    MAKE = $(MAKE) -DSAFE
!endif

# Primary target:
# QuantLib library
quantlib::
    cd ql
    $(MAKE)
    cd ..

# check
check: tests
    cd test-suite
    $(MAKE) -DQL_DIR=".." check
    cd ..

# test
test: tests

# tests
tests:: functions quantlib
    cd test-suite
    $(MAKE) -DQL_DIR=".."
    cd ..

# functions
functions:: quantlib
    cd functions\ql\Functions
    $(MAKE) -DQL_DIR="..\..\.."
    cd ..\..\..

#all
all:: examples tests
    cd test-suite
    $(MAKE) -DQL_DIR=".." check
    cd ..

# Examples
examples:: quantlib functions
    cd Examples
    $(MAKE) -DQL_DIR="..\.."
    cd ..

# the installation directive requires the QL_DIR environment variable to
# point to the installed version of QuantLib
install : inst
inst:: quantlib
    if exist "$(QL_DIR)\ql" rmdir /S /Q "$(QL_DIR)\ql"
    xcopy ql\*.hpp "$(QL_DIR)\ql" /S /I
    xcopy ql\*.h   "$(QL_DIR)\ql" /S /I

    if exist "$(QL_DIR)\lib" rmdir /S /Q "$(QL_DIR)\lib"
    xcopy lib\*.lib "$(QL_DIR)\lib" /S /I
    xcopy lib\*.pdb "$(QL_DIR)\lib" /S /I


# Documentation
docs-all:
    cd Docs
    $(MAKE) all
    cd ..

docs-html:
    cd Docs
    $(MAKE) html
    cd ..

docs-htmlhelp:
    cd Docs
    $(MAKE) htmlhelp
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
    cd ..\functions\ql\Functions
    $(MAKE) clean
    cd ..\..\..\test-suite
    $(MAKE) clean
    cd ..\Examples
    $(MAKE) clean
    cd ..\Docs
    $(MAKE) clean
    cd ..
