# Microsoft Developer Studio Generated NMAKE File, Based on BermudanSwaption.dsp
!IF "$(CFG)" == ""
CFG=BermudanSwaption - Win32 OnTheEdgeDebug
!MESSAGE No configuration specified. Defaulting to BermudanSwaption - Win32 OnTheEdgeDebug.
!ENDIF 

!IF "$(CFG)" != "BermudanSwaption - Win32 Release" && "$(CFG)" != "BermudanSwaption - Win32 Debug" && "$(CFG)" != "BermudanSwaption - Win32 OnTheEdgeRelease" && "$(CFG)" != "BermudanSwaption - Win32 OnTheEdgeDebug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BermudanSwaption.mak" CFG="BermudanSwaption - Win32 OnTheEdgeDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BermudanSwaption - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "BermudanSwaption - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"


CLEAN :
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "$(QL_DIR)" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\BermudanSwaption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BermudanSwaption.sbr"

"$(OUTDIR)\BermudanSwaption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"


CLEAN :
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"
	-@erase "$(OUTDIR)\BermudanSwaption.ilk"
	-@erase "$(OUTDIR)\BermudanSwaption.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QL_DIR)" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\BermudanSwaption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BermudanSwaption.sbr"

"$(OUTDIR)\BermudanSwaption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 OnTheEdgeRelease"

OUTDIR=.\build\OnTheEdgeRelease
INTDIR=.\build\OnTheEdgeRelease
# Begin Custom Macros
OutDir=.\build\OnTheEdgeRelease
# End Custom Macros

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"


CLEAN :
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "..\..\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\BermudanSwaption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BermudanSwaption.sbr"

"$(OUTDIR)\BermudanSwaption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 OnTheEdgeDebug"

OUTDIR=.\build\OnTheEdgeDebug
INTDIR=.\build\OnTheEdgeDebug
# Begin Custom Macros
OutDir=.\build\OnTheEdgeDebug
# End Custom Macros

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"


CLEAN :
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"
	-@erase "$(OUTDIR)\BermudanSwaption.ilk"
	-@erase "$(OUTDIR)\BermudanSwaption.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\BermudanSwaption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\BermudanSwaption.sbr"

"$(OUTDIR)\BermudanSwaption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("BermudanSwaption.dep")
!INCLUDE "BermudanSwaption.dep"
!ELSE 
!MESSAGE Warning: cannot find "BermudanSwaption.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "BermudanSwaption - Win32 Release" || "$(CFG)" == "BermudanSwaption - Win32 Debug" || "$(CFG)" == "BermudanSwaption - Win32 OnTheEdgeRelease" || "$(CFG)" == "BermudanSwaption - Win32 OnTheEdgeDebug"
SOURCE=.\BermudanSwaption.cpp

"$(INTDIR)\BermudanSwaption.obj"	"$(INTDIR)\BermudanSwaption.sbr" : $(SOURCE) "$(INTDIR)"



!ENDIF 

