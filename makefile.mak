
# main makefile for QuantLib under Borland C++
#
# $Id$
# $Source$
# $Log$
# Revision 1.22  2001/08/23 09:41:13  nando
# improved Borland examples makefiles
#
# Revision 1.21  2001/08/22 17:57:40  nando
# Examples compiles under borland
# added borland makefile
#
# Revision 1.20  2001/05/28 13:17:21  nando
# I'm confused, but it seems like we don't need to mkdir
#
# Revision 1.19  2001/05/28 13:09:55  nando
# R019-branch-merge3 merged into trunk
#
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
    cd ..\Examples
    $(MAKE) -DQL_DIR="..\.."
    cd ..

# the installation directive requires the QL_DIR environment variable to point
# to the installed version of QuantLib
install::
    if exist "$(QL_DIR)\Include\ql" rmdir /S /Q "$(QL_DIR)\Include\ql"
    xcopy Include\ql\*.hpp "$(QL_DIR)\Include\ql" /S /I

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
    cd ..\Examples
    $(MAKE) clean
    cd ..\Docs
    $(MAKE) clean
    cd ..
