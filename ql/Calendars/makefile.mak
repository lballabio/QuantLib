
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
BOOST_INCLUDE_DIR   = "$(BOOST_INCLUDE_DIR)"
INCLUDE_DIR    = ..\..

# Object files
OBJS = \
    budapest.obj$(_D) \
    copenhagen.obj$(_D) \
    frankfurt.obj$(_D) \
    helsinki.obj$(_D) \
    johannesburg.obj$(_D) \
    jointcalendar.obj$(_D) \
    london.obj$(_D) \
    milan.obj$(_D) \
    newyork.obj$(_D) \
    oslo.obj$(_D) \
    target.obj$(_D) \
    tokyo.obj$(_D) \
    toronto.obj$(_D) \
    stockholm.obj$(_D) \
    sydney.obj$(_D) \
    warsaw.obj$(_D) \
    wellington.obj$(_D) \
    zurich.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib



# Options
CC_OPTS        = -vi- -q -c -tWM \
    -I$(BOOST_INCLUDE_DIR) \
    -I$(INCLUDE_DIR)

!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!else
CC_OPTS = $(CC_OPTS) -O2
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
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
    if exist *.lib         del /q *.lib
