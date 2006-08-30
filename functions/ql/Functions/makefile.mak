
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
QL_INCLUDE_DIR = "$(QL_DIR)"
INCLUDE_DIR    = ..\..
OUTPUT_DIR     = ..\..\..\lib

# Object files
OBJS = \
    "calendars.obj$(_mt)$(_D)" \
    "daycounters.obj$(_mt)$(_D)" \
    "mathf.obj$(_mt)$(_D)" \
    "vols.obj$(_mt)$(_D)"

# Tools to be used
CC        = bcc32
TLIB      = tlib



# Options
CC_OPTS        = -vi- -q -c -I$(INCLUDE_DIR) -I$(QL_INCLUDE_DIR) -w-8070

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

TLIB_OPTS    = /P32
!ifdef _DEBUG
TLIB_OPTS    = /P64
!endif

# Generic rules
.cpp.obj:
    $(CC) $(CC_OPTS) $<
.cpp.obj$(_mt)$(_D):
    $(CC) $(CC_OPTS) -o$@ $<

# Primary target:
# static library
$(OUTPUT_DIR)\QuantLibFunctions-bcb$(_mt)$(_D)-$(VERSION_STRING).lib:: $(OUTPUT_DIR) $(OBJS)
    if exist $(OUTPUT_DIR)\QuantLibFunctions-bcb$(_mt)$(_D)-$(VERSION_STRING).lib \
         del $(OUTPUT_DIR)\QuantLibFunctions-bcb$(_mt)$(_D)-$(VERSION_STRING).lib
    $(TLIB) $(TLIB_OPTS) \
           "$(OUTPUT_DIR)\QuantLibFunctions-bcb$(_mt)$(_D)-$(VERSION_STRING).lib" /a $(OBJS)

$(OUTPUT_DIR):
    if not exist ..\..\..\lib md ..\..\..\lib

# Clean up
clean::
    if exist *.obj* del /q *.obj*
    if exist *.lib  del /q *.lib
