# Microsoft Developer Studio Generated NMAKE File, Based on HedgingError.dsp
!IF "$(CFG)" == ""
CFG=HedgingError - Win32 Debug
!MESSAGE No configuration specified. Defaulting to HedgingError - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "HedgingError - Win32 Release" && "$(CFG)" != "HedgingError - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HedgingError.mak" CFG="HedgingError - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HedgingError - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HedgingError - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "HedgingError - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"


CLEAN :
	-@erase "$(INTDIR)\HedgingError.obj"
	-@erase "$(INTDIR)\HedgingError.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\HedgingError.bsc"
	-@erase "$(OUTDIR)\HedgingError.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\HedgingError.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HedgingError.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\HedgingError.sbr"

"$(OUTDIR)\HedgingError.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\HedgingError.pdb" /machine:I386 /out:"$(OUTDIR)\HedgingError.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\HedgingError.obj"

"$(OUTDIR)\HedgingError.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HedgingError - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"


CLEAN :
	-@erase "$(INTDIR)\HedgingError.obj"
	-@erase "$(INTDIR)\HedgingError.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\HedgingError.bsc"
	-@erase "$(OUTDIR)\HedgingError.exe"
	-@erase "$(OUTDIR)\HedgingError.ilk"
	-@erase "$(OUTDIR)\HedgingError.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\HedgingError.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HedgingError.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\HedgingError.sbr"

"$(OUTDIR)\HedgingError.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\HedgingError.pdb" /debug /machine:I386 /out:"$(OUTDIR)\HedgingError.exe" /pdbtype:sept /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\HedgingError.obj"

"$(OUTDIR)\HedgingError.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("HedgingError.dep")
!INCLUDE "HedgingError.dep"
!ELSE 
!MESSAGE Warning: cannot find "HedgingError.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "HedgingError - Win32 Release" || "$(CFG)" == "HedgingError - Win32 Debug"
SOURCE=.\HedgingError.cpp

"$(INTDIR)\HedgingError.obj"	"$(INTDIR)\HedgingError.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

