
.autodepend
.silent

# Debug version
!ifdef DEBUG
    _D = _d
!endif

# Directories
INCLUDE_DIR    = ..\..
BCC_INCLUDE    = $(MAKEDIR)\..\include
SRCDIR         = "."
OBJDIR         = "..\..\build\Borland"

# Object files
OBJS = \
    $(OBJDIR)\budapest.obj$(_D) \
    $(OBJDIR)\frankfurt.obj$(_D) \
    $(OBJDIR)\helsinki.obj$(_D) \
    $(OBJDIR)\johannesburg.obj$(_D) \
    $(OBJDIR)\jointcalendar.obj$(_D) \
    $(OBJDIR)\london.obj$(_D) \
    $(OBJDIR)\milan.obj$(_D) \
    $(OBJDIR)\newyork.obj$(_D) \
    $(OBJDIR)\oslo.obj$(_D) \
    $(OBJDIR)\target.obj$(_D) \
    $(OBJDIR)\tokyo.obj$(_D) \
    $(OBJDIR)\toronto.obj$(_D) \
    $(OBJDIR)\stockholm.obj$(_D) \
    $(OBJDIR)\sydney.obj$(_D) \
    $(OBJDIR)\warsaw.obj$(_D) \
    $(OBJDIR)\wellington.obj$(_D) \
    $(OBJDIR)\zurich.obj$(_D)

# Tools to be used
CC        = bcc32
TLIB      = tlib



# Options
CC_OPTS        = -vi- -q -c -tWM \
    -I$(INCLUDE_DIR) \
    -I$(BCC_INCLUDE) \
    -n$(OBJDIR)

!ifdef DEBUG
CC_OPTS = $(CC_OPTS) -v -DQL_DEBUG
!endif
!ifdef SAFE
CC_OPTS = $(CC_OPTS) -DQL_EXTRA_SAFETY_CHECKS
!endif

TLIB_OPTS    = /P128
!ifdef DEBUG
TLIB_OPTS    = /P128
!endif

# Generic rules
{$(SRCDIR)}.cpp{$(OBJDIR)}.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
$(OBJDIR)\Calendars$(_D).lib:: $(OBJDIR) $(OBJS)
    if exist $(OBJDIR)\Calendars$(_D).lib     del $(OBJDIR)\Calendars$(_D).lib
    $(TLIB) $(TLIB_OPTS) $(OBJDIR)\Calendars$(_D).lib /a $(OBJS)

#create build dir
$(OBJDIR):
        @if not exist $(OBJDIR) (md $(OBJDIR))

# Clean up
clean::
    if exist $(OBJDIR)\*.obj         del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.obj$(_D)    del /q $(OBJDIR)\*.obj
    if exist $(OBJDIR)\*.lib         del /q $(OBJDIR)\*.lib
