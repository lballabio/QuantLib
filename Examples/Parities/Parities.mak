# Microsoft Developer Studio Generated NMAKE File, Based on Parities.dsp
!IF "$(CFG)" == ""
CFG=Parities - Win32 OnTheEdgeDebug
!MESSAGE No configuration specified. Defaulting to Parities - Win32 OnTheEdgeDebug.
!ENDIF 

!IF "$(CFG)" != "Parities - Win32 Release" && "$(CFG)" != "Parities - Win32 Debug" && "$(CFG)" != "Parities - Win32 OnTheEdgeRelease" && "$(CFG)" != "Parities - Win32 OnTheEdgeDebug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Parities.mak" CFG="Parities - Win32 OnTheEdgeDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Parities - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "Parities - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "Parities - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Console Application")
!MESSAGE "Parities - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Parities - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\Parities.exe" "$(OUTDIR)\Parities.bsc"


CLEAN :
	-@erase "$(INTDIR)\Parities.obj"
	-@erase "$(INTDIR)\Parities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Parities.bsc"
	-@erase "$(OUTDIR)\Parities.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "$(QL_DIR)" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Parities.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Parities.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Parities.sbr"

"$(OUTDIR)\Parities.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\Parities.pdb" /machine:I386 /out:"$(OUTDIR)\Parities.exe" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\Parities.obj"

"$(OUTDIR)\Parities.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Parities - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\Parities.exe" "$(OUTDIR)\Parities.bsc"


CLEAN :
	-@erase "$(INTDIR)\Parities.obj"
	-@erase "$(INTDIR)\Parities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Parities.bsc"
	-@erase "$(OUTDIR)\Parities.exe"
	-@erase "$(OUTDIR)\Parities.ilk"
	-@erase "$(OUTDIR)\Parities.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QL_DIR)" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Parities.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Parities.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Parities.sbr"

"$(OUTDIR)\Parities.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\Parities.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Parities.exe" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\Parities.obj"

"$(OUTDIR)\Parities.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Parities - Win32 OnTheEdgeRelease"

OUTDIR=.\OnTheEdgeRelease
INTDIR=.\OnTheEdgeRelease
# Begin Custom Macros
OutDir=.\OnTheEdgeRelease
# End Custom Macros

ALL : "$(OUTDIR)\Parities.exe" "$(OUTDIR)\Parities.bsc"


CLEAN :
	-@erase "$(INTDIR)\Parities.obj"
	-@erase "$(INTDIR)\Parities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\Parities.bsc"
	-@erase "$(OUTDIR)\Parities.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "..\..\\" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Parities.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Parities.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Parities.sbr"

"$(OUTDIR)\Parities.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\Parities.pdb" /machine:I386 /out:"$(OUTDIR)\Parities.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\Parities.obj"

"$(OUTDIR)\Parities.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Parities - Win32 OnTheEdgeDebug"

OUTDIR=.\OnTheEdgeDebug
INTDIR=.\OnTheEdgeDebug
# Begin Custom Macros
OutDir=.\OnTheEdgeDebug
# End Custom Macros

ALL : "$(OUTDIR)\Parities.exe" "$(OUTDIR)\Parities.bsc"


CLEAN :
	-@erase "$(INTDIR)\Parities.obj"
	-@erase "$(INTDIR)\Parities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\Parities.bsc"
	-@erase "$(OUTDIR)\Parities.exe"
	-@erase "$(OUTDIR)\Parities.ilk"
	-@erase "$(OUTDIR)\Parities.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\\" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\Parities.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\Parities.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\Parities.sbr"

"$(OUTDIR)\Parities.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\Parities.pdb" /debug /machine:I386 /out:"$(OUTDIR)\Parities.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\Parities.obj"

"$(OUTDIR)\Parities.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

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


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("Parities.dep")
!INCLUDE "Parities.dep"
!ELSE 
!MESSAGE Warning: cannot find "Parities.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "Parities - Win32 Release" || "$(CFG)" == "Parities - Win32 Debug" || "$(CFG)" == "Parities - Win32 OnTheEdgeRelease" || "$(CFG)" == "Parities - Win32 OnTheEdgeDebug"
SOURCE=.\Parities.cpp

"$(INTDIR)\Parities.obj"	"$(INTDIR)\Parities.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

