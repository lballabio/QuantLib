
.autodepend
#.silent

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
    "beijing.obj$(_mt)$(_D)" \
    "bratislava.obj$(_mt)$(_D)" \
    "budapest.obj$(_mt)$(_D)" \
    "copenhagen.obj$(_mt)$(_D)" \
    "germany.obj$(_mt)$(_D)" \
    "helsinki.obj$(_mt)$(_D)" \
    "hongkong.obj$(_mt)$(_D)" \
    "italy.obj$(_mt)$(_D)" \
    "johannesburg.obj$(_mt)$(_D)" \
    "jointcalendar.obj$(_mt)$(_D)" \
    "oslo.obj$(_mt)$(_D)" \
    "prague.obj$(_mt)$(_D)" \
    "riyadh.obj$(_mt)$(_D)" \
    "seoul.obj$(_mt)$(_D)" \
    "singapore.obj$(_mt)$(_D)" \
    "stockholm.obj$(_mt)$(_D)" \
    "sydney.obj$(_mt)$(_D)" \
    "taiwan.obj$(_mt)$(_D)" \
    "target.obj$(_mt)$(_D)" \
    "tokyo.obj$(_mt)$(_D)" \
    "toronto.obj$(_mt)$(_D)" \
    "unitedkingdom.obj$(_mt)$(_D)" \
    "unitedstates.obj$(_mt)$(_D)" \
    "warsaw.obj$(_mt)$(_D)" \
    "wellington.obj$(_mt)$(_D)" \
    "zurich.obj$(_mt)$(_D)"


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
Calendars$(_mt)$(_D).lib:: $(OBJS)
    if exist Calendars$(_mt)$(_D).lib     del Calendars$(_mt)$(_D).lib
    $(TLIB) $(TLIB_OPTS) "Calendars$(_mt)$(_D).lib" /a $(OBJS)





# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
