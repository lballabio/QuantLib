
# makefile for DiscreteHedging.cpp under Borland C++

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
QL_INCLUDE_DIR   = "$(QL_DIR)"
QL_LIB_DIR       = "$(QL_DIR)\lib\Win32\Borland"

#Warning W8057 : Parameter 'argc' is never used in function main(int,char * *)

# Options
CC_OPTS = -vi- -w-8057 \
    -I$(QL_INCLUDE_DIR)

!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

# Primary target:
DiscreteHedging$(_D).exe: DiscreteHedging.cpp
    bcc32 $(CC_OPTS) -L$(QL_LIB_DIR) -o"DiscreteHedging$(_D).obj" -eDiscreteHedging$(_D).exe DiscreteHedging.cpp QuantLib$(_D).lib





# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.tds   del /q *.tds
    if exist *.exe   del /q *.exe
