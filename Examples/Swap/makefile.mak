
#
# makefile for swapvaluation.cpp under Borland C++
#
# $Id$

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
QL_INCLUDE_DIR = "$(QL_DIR)"
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
SwapValuation$(_D).exe: swapvaluation.cpp
    bcc32 $(CC_OPTS) -L$(QL_LIB_DIR) -oSwapValuation$(_D).obj -eSwapValuation$(_D).exe swapvaluation.cpp QuantLib$(_D).lib

# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.tds   del /q *.tds
    if exist *.exe   del /q *.exe
