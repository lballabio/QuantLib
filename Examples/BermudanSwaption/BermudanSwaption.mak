# Microsoft Developer Studio Generated NMAKE File, Based on BermudanSwaption.dsp
!IF "$(CFG)" == ""
CFG=BermudanSwaption - Win32 Debug SingleThread
!MESSAGE No configuration specified. Defaulting to BermudanSwaption - Win32 Debug SingleThread.
!ENDIF 

!IF "$(CFG)" != "BermudanSwaption - Win32 Release" && "$(CFG)" != "BermudanSwaption - Win32 Debug" && "$(CFG)" != "BermudanSwaption - Win32 Release MTDLL" && "$(CFG)" != "BermudanSwaption - Win32 Debug MTDLL" && "$(CFG)" != "BermudanSwaption - Win32 Release SingleThread" && "$(CFG)" != "BermudanSwaption - Win32 Debug SingleThread"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "BermudanSwaption.mak" CFG="BermudanSwaption - Win32 Debug SingleThread"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "BermudanSwaption - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 Release MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 Debug MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 Release SingleThread" (based on "Win32 (x86) Console Application")
!MESSAGE "BermudanSwaption - Win32 Debug SingleThread" (based on "Win32 (x86) Console Application")
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

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release" "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj" \
	"..\..\lib\QuantLib-vc6-mt-s-0_3_9.lib"

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

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug" "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
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
CPP_PROJ=/nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj" \
	"..\..\lib\QuantLib-vc6-mt-sgd-0_3_9.lib"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Release MTDLL"

OUTDIR=.\build\ReleaseMTDLL
INTDIR=.\build\ReleaseMTDLL
# Begin Custom Macros
OutDir=.\build\ReleaseMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release MTDLL" "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj" \
	"..\..\lib\QuantLib-vc6-mt-0_3_9.lib"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Debug MTDLL"

OUTDIR=.\build\DebugMTDLL
INTDIR=.\build\DebugMTDLL
# Begin Custom Macros
OutDir=.\build\DebugMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug MTDLL" "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
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
CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj" \
	"..\..\lib\QuantLib-vc6-mt-gd-0_3_9.lib"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Release SingleThread"

OUTDIR=.\build\ReleaseST
INTDIR=.\build\ReleaseST
# Begin Custom Macros
OutDir=.\build\ReleaseST
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release SingleThread" "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\BermudanSwaption.obj"
	-@erase "$(INTDIR)\BermudanSwaption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\BermudanSwaption.bsc"
	-@erase "$(OUTDIR)\BermudanSwaption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj" \
	"..\..\lib\QuantLib-vc6-s-0_3_9.lib"

"$(OUTDIR)\BermudanSwaption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Debug SingleThread"

OUTDIR=.\build\DebugST
INTDIR=.\build\DebugST
# Begin Custom Macros
OutDir=.\build\DebugST
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug SingleThread" "$(OUTDIR)\BermudanSwaption.exe" "$(OUTDIR)\BermudanSwaption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
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
CPP_PROJ=/nologo /MLd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\BermudanSwaption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\BermudanSwaption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\BermudanSwaption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\BermudanSwaption.obj" \
	"..\..\lib\QuantLib-vc6-sgd-0_3_9.lib"

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


!IF "$(CFG)" == "BermudanSwaption - Win32 Release" || "$(CFG)" == "BermudanSwaption - Win32 Debug" || "$(CFG)" == "BermudanSwaption - Win32 Release MTDLL" || "$(CFG)" == "BermudanSwaption - Win32 Debug MTDLL" || "$(CFG)" == "BermudanSwaption - Win32 Release SingleThread" || "$(CFG)" == "BermudanSwaption - Win32 Debug SingleThread"
SOURCE=.\BermudanSwaption.cpp

"$(INTDIR)\BermudanSwaption.obj"	"$(INTDIR)\BermudanSwaption.sbr" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "BermudanSwaption - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\Examples\BermudanSwaption"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\Examples\BermudanSwaption"

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\Examples\BermudanSwaption"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\Examples\BermudanSwaption"

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Release MTDLL"

"QuantLib - Win32 Release MTDLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" 
   cd ".\Examples\BermudanSwaption"

"QuantLib - Win32 Release MTDLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" RECURSE=1 CLEAN 
   cd ".\Examples\BermudanSwaption"

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Debug MTDLL"

"QuantLib - Win32 Debug MTDLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" 
   cd ".\Examples\BermudanSwaption"

"QuantLib - Win32 Debug MTDLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" RECURSE=1 CLEAN 
   cd ".\Examples\BermudanSwaption"

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Release SingleThread"

"QuantLib - Win32 Release SingleThread" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release SingleThread" 
   cd ".\Examples\BermudanSwaption"

"QuantLib - Win32 Release SingleThreadCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release SingleThread" RECURSE=1 CLEAN 
   cd ".\Examples\BermudanSwaption"

!ELSEIF  "$(CFG)" == "BermudanSwaption - Win32 Debug SingleThread"

"QuantLib - Win32 Debug SingleThread" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug SingleThread" 
   cd ".\Examples\BermudanSwaption"

"QuantLib - Win32 Debug SingleThreadCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug SingleThread" RECURSE=1 CLEAN 
   cd ".\Examples\BermudanSwaption"

!ENDIF 


!ENDIF 

