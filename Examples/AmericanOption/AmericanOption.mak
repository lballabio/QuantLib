# Microsoft Developer Studio Generated NMAKE File, Based on AmericanOption.dsp
!IF "$(CFG)" == ""
CFG=AmericanOption - Win32 Debug SingleThread
!MESSAGE No configuration specified. Defaulting to AmericanOption - Win32 Debug SingleThread.
!ENDIF 

!IF "$(CFG)" != "AmericanOption - Win32 Release" && "$(CFG)" != "AmericanOption - Win32 Debug" && "$(CFG)" != "AmericanOption - Win32 Release MTDLL" && "$(CFG)" != "AmericanOption - Win32 Debug MTDLL" && "$(CFG)" != "AmericanOption - Win32 Release SingleThread" && "$(CFG)" != "AmericanOption - Win32 Debug SingleThread"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "AmericanOption.mak" CFG="AmericanOption - Win32 Debug SingleThread"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "AmericanOption - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "AmericanOption - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "AmericanOption - Win32 Release MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "AmericanOption - Win32 Debug MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "AmericanOption - Win32 Release SingleThread" (based on "Win32 (x86) Console Application")
!MESSAGE "AmericanOption - Win32 Debug SingleThread" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "AmericanOption - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release" "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AmericanOption.obj"
	-@erase "$(INTDIR)\AmericanOption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\AmericanOption.bsc"
	-@erase "$(OUTDIR)\AmericanOption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\AmericanOption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AmericanOption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AmericanOption.sbr"

"$(OUTDIR)\AmericanOption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\AmericanOption.pdb" /machine:I386 /out:"$(OUTDIR)\AmericanOption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AmericanOption.obj" \
	"..\..\lib\QuantLib-vc6-mt-s-0_3_7.lib"

"$(OUTDIR)\AmericanOption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug" "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AmericanOption.obj"
	-@erase "$(INTDIR)\AmericanOption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AmericanOption.bsc"
	-@erase "$(OUTDIR)\AmericanOption.exe"
	-@erase "$(OUTDIR)\AmericanOption.ilk"
	-@erase "$(OUTDIR)\AmericanOption.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\AmericanOption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AmericanOption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AmericanOption.sbr"

"$(OUTDIR)\AmericanOption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\AmericanOption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\AmericanOption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AmericanOption.obj" \
	"..\..\lib\QuantLib-vc6-mt-sgd-0_3_7.lib"

"$(OUTDIR)\AmericanOption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Release MTDLL"

OUTDIR=.\build\ReleaseMTDLL
INTDIR=.\build\ReleaseMTDLL
# Begin Custom Macros
OutDir=.\build\ReleaseMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release MTDLL" "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AmericanOption.obj"
	-@erase "$(INTDIR)\AmericanOption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\AmericanOption.bsc"
	-@erase "$(OUTDIR)\AmericanOption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\AmericanOption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AmericanOption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AmericanOption.sbr"

"$(OUTDIR)\AmericanOption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\AmericanOption.pdb" /machine:I386 /out:"$(OUTDIR)\AmericanOption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AmericanOption.obj" \
	"..\..\lib\QuantLib-vc6-mt-0_3_7.lib"

"$(OUTDIR)\AmericanOption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Debug MTDLL"

OUTDIR=.\build\DebugMTDLL
INTDIR=.\build\DebugMTDLL
# Begin Custom Macros
OutDir=.\build\DebugMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug MTDLL" "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AmericanOption.obj"
	-@erase "$(INTDIR)\AmericanOption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AmericanOption.bsc"
	-@erase "$(OUTDIR)\AmericanOption.exe"
	-@erase "$(OUTDIR)\AmericanOption.ilk"
	-@erase "$(OUTDIR)\AmericanOption.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\AmericanOption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AmericanOption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AmericanOption.sbr"

"$(OUTDIR)\AmericanOption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\AmericanOption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\AmericanOption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AmericanOption.obj" \
	"..\..\lib\QuantLib-vc6-mt-gd-0_3_7.lib"

"$(OUTDIR)\AmericanOption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Release SingleThread"

OUTDIR=.\build\ReleaseST
INTDIR=.\build\ReleaseST
# Begin Custom Macros
OutDir=.\build\ReleaseST
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release SingleThread" "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AmericanOption.obj"
	-@erase "$(INTDIR)\AmericanOption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\AmericanOption.bsc"
	-@erase "$(OUTDIR)\AmericanOption.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\AmericanOption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AmericanOption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AmericanOption.sbr"

"$(OUTDIR)\AmericanOption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\AmericanOption.pdb" /machine:I386 /out:"$(OUTDIR)\AmericanOption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AmericanOption.obj" \
	"..\..\lib\QuantLib-vc6-s-0_3_7.lib"

"$(OUTDIR)\AmericanOption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Debug SingleThread"

OUTDIR=.\build\DebugST
INTDIR=.\build\DebugST
# Begin Custom Macros
OutDir=.\build\DebugST
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug SingleThread" "$(OUTDIR)\AmericanOption.exe" "$(OUTDIR)\AmericanOption.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\AmericanOption.obj"
	-@erase "$(INTDIR)\AmericanOption.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\AmericanOption.bsc"
	-@erase "$(OUTDIR)\AmericanOption.exe"
	-@erase "$(OUTDIR)\AmericanOption.ilk"
	-@erase "$(OUTDIR)\AmericanOption.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /Gi /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\AmericanOption.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\AmericanOption.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\AmericanOption.sbr"

"$(OUTDIR)\AmericanOption.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\AmericanOption.pdb" /debug /machine:I386 /out:"$(OUTDIR)\AmericanOption.exe" /libpath:"..\..\lib" 
LINK32_OBJS= \
	"$(INTDIR)\AmericanOption.obj" \
	"..\..\lib\QuantLib-vc6-sgd-0_3_7.lib"

"$(OUTDIR)\AmericanOption.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("AmericanOption.dep")
!INCLUDE "AmericanOption.dep"
!ELSE 
!MESSAGE Warning: cannot find "AmericanOption.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "AmericanOption - Win32 Release" || "$(CFG)" == "AmericanOption - Win32 Debug" || "$(CFG)" == "AmericanOption - Win32 Release MTDLL" || "$(CFG)" == "AmericanOption - Win32 Debug MTDLL" || "$(CFG)" == "AmericanOption - Win32 Release SingleThread" || "$(CFG)" == "AmericanOption - Win32 Debug SingleThread"
SOURCE=.\AmericanOption.cpp

"$(INTDIR)\AmericanOption.obj"	"$(INTDIR)\AmericanOption.sbr" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "AmericanOption - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\Examples\AmericanOption"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\Examples\AmericanOption"

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\Examples\AmericanOption"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\Examples\AmericanOption"

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Release MTDLL"

"QuantLib - Win32 Release MTDLL" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" 
   cd ".\Examples\AmericanOption"

"QuantLib - Win32 Release MTDLLCLEAN" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" RECURSE=1 CLEAN 
   cd ".\Examples\AmericanOption"

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Debug MTDLL"

"QuantLib - Win32 Debug MTDLL" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" 
   cd ".\Examples\AmericanOption"

"QuantLib - Win32 Debug MTDLLCLEAN" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" RECURSE=1 CLEAN 
   cd ".\Examples\AmericanOption"

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Release SingleThread"

"QuantLib - Win32 Release SingleThread" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release SingleThread" 
   cd ".\Examples\AmericanOption"

"QuantLib - Win32 Release SingleThreadCLEAN" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release SingleThread" RECURSE=1 CLEAN 
   cd ".\Examples\AmericanOption"

!ELSEIF  "$(CFG)" == "AmericanOption - Win32 Debug SingleThread"

"QuantLib - Win32 Debug SingleThread" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug SingleThread" 
   cd ".\Examples\AmericanOption"

"QuantLib - Win32 Debug SingleThreadCLEAN" : 
   cd "\Branches\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug SingleThread" RECURSE=1 CLEAN 
   cd ".\Examples\AmericanOption"

!ENDIF 


!ENDIF 

