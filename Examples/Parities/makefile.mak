
#
# makefile for Parities.cpp under Borland C++
#
# $Id$
# $Log$
# Revision 1.5  2001/08/29 13:08:33  nando
# little tweaks to avoid gcc warnings
#
# Revision 1.4  2001/08/23 11:24:37  nando
# try/catch in examples
#
# Revision 1.3  2001/08/23 09:42:08  nando
# improved Borland examples makefiles
#
# Revision 1.2  2001/08/23 09:41:13  nando
# improved Borland examples makefiles
#
# Revision 1.1  2001/08/22 17:57:54  nando
# Examples compiles under borland
# added borland makefile
#

.autodepend
#.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
QL_INCLUDE_DIR = "$(QL_DIR)\Include"
QL_LIB_DIR     = "$(QL_DIR)\lib\Win32\Borland"

#Warning W8026 :
#Warning W8027 :
#Warning W8012 :
#Warning W8057 : Parameter 'argc' is never used in function main(int,char * *)

# Options
CC_OPTS = -w-8026 -w-8027 -w-8012 -w-8057 \
    -I$(QL_INCLUDE_DIR)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif

# Primary target:
Parities$(_D).exe: Parities.cpp
    bcc32 $(CC_OPTS) -L$(QL_LIB_DIR) -oParities$(_D).obj -eParities$(_D).exe Parities.cpp QuantLib$(_D).lib

# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.tds   del /q *.tds
    if exist *.exe   del /q *.exe
