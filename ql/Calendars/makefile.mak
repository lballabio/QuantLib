
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

# Object files
OBJS = frankfurt.obj$(_D) \
       helsinki.obj$(_D) \
       johannesburg.obj$(_D) \
       london.obj$(_D) \
       milan.obj$(_D) \
       newyork.obj$(_D) \
       target.obj$(_D) \
       tokyo.obj$(_D) \
       toronto.obj$(_D) \
       sydney.obj$(_D) \
       wellington.obj$(_D) \
       zurich.obj$(_D)

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
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
Calendars$(_D).lib:: $(OBJS)
    if exist Calendars$(_D).lib     del Calendars$(_D).lib
    $(TLIB) $(TLIB_OPTS) Calendars$(_D).lib /a $(OBJS)


# Clean up
clean::
    if exist *.obj         del /q *.obj
    if exist *.obj$(_D)    del /q *.obj
    if exist *.lib   del /q *.lib

