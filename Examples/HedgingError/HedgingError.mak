# Microsoft Developer Studio Generated NMAKE File, Based on HedgingError.dsp
!IF "$(CFG)" == ""
CFG=HedgingError - Win32 OnTheEdgeDebug
!MESSAGE No configuration specified. Defaulting to HedgingError - Win32 OnTheEdgeDebug.
!ENDIF 

!IF "$(CFG)" != "HedgingError - Win32 Release" && "$(CFG)" != "HedgingError - Win32 Debug" && "$(CFG)" != "HedgingError - Win32 OnTheEdgeRelease" && "$(CFG)" != "HedgingError - Win32 OnTheEdgeDebug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "HedgingError.mak" CFG="HedgingError - Win32 OnTheEdgeDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "HedgingError - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "HedgingError - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "HedgingError - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Console Application")
!MESSAGE "HedgingError - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "HedgingError - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release" "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\HedgingError.obj"
	-@erase "$(INTDIR)\HedgingError.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\HedgingError.bsc"
	-@erase "$(OUTDIR)\HedgingError.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "$(QL_DIR)\Include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\HedgingError.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HedgingError.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\HedgingError.sbr"

"$(OUTDIR)\HedgingError.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\HedgingError.pdb" /machine:I386 /out:"$(OUTDIR)\HedgingError.exe" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\HedgingError.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib.lib"

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

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug" "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
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

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QL_DIR)\Include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\HedgingError.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HedgingError.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\HedgingError.sbr"

"$(OUTDIR)\HedgingError.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\HedgingError.pdb" /debug /machine:I386 /out:"$(OUTDIR)\HedgingError.exe" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\HedgingError.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)\HedgingError.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HedgingError - Win32 OnTheEdgeRelease"

OUTDIR=.\OnTheEdgeRelease
INTDIR=.\OnTheEdgeRelease
# Begin Custom Macros
OutDir=.\OnTheEdgeRelease
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ELSE 

ALL : "QuantLib - Win32 OnTheEdgeRelease" "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 OnTheEdgeReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\HedgingError.obj"
	-@erase "$(INTDIR)\HedgingError.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\HedgingError.bsc"
	-@erase "$(OUTDIR)\HedgingError.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "..\..\Include" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\HedgingError.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
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
	"$(INTDIR)\HedgingError.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)\HedgingError.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "HedgingError - Win32 OnTheEdgeDebug"

OUTDIR=.\OnTheEdgeDebug
INTDIR=.\OnTheEdgeDebug
# Begin Custom Macros
OutDir=.\OnTheEdgeDebug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ELSE 

ALL : "QuantLib - Win32 OnTheEdgeDebug" "$(OUTDIR)\HedgingError.exe" "$(OUTDIR)\HedgingError.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 OnTheEdgeDebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
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

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\Include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\HedgingError.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\HedgingError.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\HedgingError.sbr"

"$(OUTDIR)\HedgingError.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\HedgingError.pdb" /debug /machine:I386 /out:"$(OUTDIR)\HedgingError.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\HedgingError.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)\HedgingError.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("HedgingError.dep")
!INCLUDE "HedgingError.dep"
!ELSE 
!MESSAGE Warning: cannot find "HedgingError.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "HedgingError - Win32 Release" || "$(CFG)" == "HedgingError - Win32 Debug" || "$(CFG)" == "HedgingError - Win32 OnTheEdgeRelease" || "$(CFG)" == "HedgingError - Win32 OnTheEdgeDebug"
SOURCE=.\HedgingError.cpp

"$(INTDIR)\HedgingError.obj"	"$(INTDIR)\HedgingError.sbr" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "HedgingError - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\Examples\HedgingError"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\Examples\HedgingError"

!ELSEIF  "$(CFG)" == "HedgingError - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\Examples\HedgingError"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\Examples\HedgingError"

!ELSEIF  "$(CFG)" == "HedgingError - Win32 OnTheEdgeRelease"

"QuantLib - Win32 OnTheEdgeRelease" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 OnTheEdgeRelease" 
   cd ".\Examples\HedgingError"

"QuantLib - Win32 OnTheEdgeReleaseCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 OnTheEdgeRelease" RECURSE=1 CLEAN 
   cd ".\Examples\HedgingError"

!ELSEIF  "$(CFG)" == "HedgingError - Win32 OnTheEdgeDebug"

"QuantLib - Win32 OnTheEdgeDebug" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 OnTheEdgeDebug" 
   cd ".\Examples\HedgingError"

"QuantLib - Win32 OnTheEdgeDebugCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 OnTheEdgeDebug" RECURSE=1 CLEAN 
   cd ".\Examples\HedgingError"

!ENDIF 


!ENDIF 

