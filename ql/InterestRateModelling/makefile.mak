
# $Id$

.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..
BCC_INCLUDE    = $(MAKEDIR)\..\include
BCC_LIBS       = $(MAKEDIR)\..\lib

# Object files
OBJS = \
       calibrationhelper.obj$(_D) \
       model.obj$(_D) \
       onefactormodel.obj$(_D) \
       timefunction.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib

# Options
CC_OPTS        = -vi- -q -c -tWM -n$(OUTPUT_DIR) \
    -w-8026 -w-8027 -w-8012 \
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE)
!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif

TLIB_OPTS    = /P128
!ifdef DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj_d:
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
IRModelling$(_D).lib:: $(OBJS)
    if exist IRModelling$(_D).lib     del IRModelling$(_D).lib
    $(TLIB) $(TLIB_OPTS) IRModelling$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj   del /q *.obj
    if exist *.lib   del /q *.lib

