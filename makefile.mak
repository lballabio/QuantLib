
# main makefile for QuantLib under Borland C++
#
# $Id$
# $Source$
# $Log$
# Revision 1.18  2001/05/25 16:33:58  nando
# improved install directive
#
# Revision 1.17  2001/05/24 16:06:17  nando
# commented out QL_DIR requirement
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

# QuantLib library
install::
    if exist "$(QL_DIR)\Include\ql" rmdir /S /Q "$(QL_DIR)\Include\ql"
    mkdir "$(QL_DIR)\Include\qk"
    xcopy Include\ql\*.hpp "$(QL_DIR)\Include\ql" /S /I

    if exist "$(QL_DIR)\lib\Win32\VisualStudio" rmdir /S /Q "$(QL_DIR)\lib\Win32\VisualStudio"
    mkdir "$(QL_DIR)\lib\Win32\VisualStudio"
    xcopy lib\Win32\VisualStudio\*.lib "$(QL_DIR)\lib\Win32\VisualStudio" /S /I

    if exist "$(QL_DIR)\lib\Win32\Borland" rmdir /S /Q "$(QL_DIR)\lib\Win32\Borland"
    mkdir "$(QL_DIR)\lib\Win32\Borland"
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
    cd ..\Docs
    $(MAKE) clean
    cd ..
