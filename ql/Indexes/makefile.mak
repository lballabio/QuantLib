
.autodepend
.silent

!ifdef _DEBUG
!ifndef _RTLDLL
    _D = -sd
!else
    _D = -d
!endif
!else
!ifndef _RTLDLL
    _D = -s
!endif
!endif

!ifdef __MT__
    _mt = -mt
!endif

# Directories
INCLUDE_DIR    = ..\..

# Object files
OBJS = \
    "xibor.obj$(_mt)$(_D)" \
    "xibormanager.obj$(_mt)$(_D)"

# Tools to be used
CC        = bcc32
TLIB      = tlib



# Options
CC_OPTS        = -vi- -q -c -I$(INCLUDE_DIR)

!ifdef _DEBUG
CC_OPTS = $(CC_OPTS) -v -D_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2 -DNDEBUG
!endif

!ifdef _RTLDLL
    CC_OPTS = $(CC_OPTS) -D_RTLDLL
!endif

!ifdef __MT__
    CC_OPTS = $(CC_OPTS) -tWM
!endif

!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

TLIB_OPTS    = /P128
!ifdef _DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
Indexes$(_mt)$(_D).lib:: $(OBJS)
    if exist Indexes$(_mt)$(_D).lib     del Indexes$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "Indexes$(_mt)$(_D).lib" /a $(OBJS)





# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
