# Microsoft Developer Studio Generated NMAKE File, Based on DiscreteHedging.dsp
!IF "$(CFG)" == ""
CFG=DiscreteHedging - Win32 Debug DLL
!MESSAGE No configuration specified. Defaulting to DiscreteHedging - Win32 Debug DLL.
!ENDIF 

!IF "$(CFG)" != "DiscreteHedging - Win32 Release" && "$(CFG)" != "DiscreteHedging - Win32 Debug" && "$(CFG)" != "DiscreteHedging - Win32 Release DLL" && "$(CFG)" != "DiscreteHedging - Win32 Debug DLL"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "DiscreteHedging.mak" CFG="DiscreteHedging - Win32 Debug DLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DiscreteHedging - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "DiscreteHedging - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "DiscreteHedging - Win32 Release DLL" (based on "Win32 (x86) Console Application")
!MESSAGE "DiscreteHedging - Win32 Debug DLL" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "DiscreteHedging - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release" "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DiscreteHedging.obj"
	-@erase "$(INTDIR)\DiscreteHedging.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DiscreteHedging.bsc"
	-@erase "$(OUTDIR)\DiscreteHedging.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\DiscreteHedging.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DiscreteHedging.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DiscreteHedging.sbr"

"$(OUTDIR)\DiscreteHedging.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\DiscreteHedging.pdb" /machine:I386 /out:"$(OUTDIR)\DiscreteHedging.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\DiscreteHedging.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)\DiscreteHedging.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DiscreteHedging - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug" "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DiscreteHedging.obj"
	-@erase "$(INTDIR)\DiscreteHedging.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DiscreteHedging.bsc"
	-@erase "$(OUTDIR)\DiscreteHedging.exe"
	-@erase "$(OUTDIR)\DiscreteHedging.ilk"
	-@erase "$(OUTDIR)\DiscreteHedging.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\DiscreteHedging.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DiscreteHedging.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DiscreteHedging.sbr"

"$(OUTDIR)\DiscreteHedging.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\DiscreteHedging.pdb" /debug /machine:I386 /out:"$(OUTDIR)\DiscreteHedging.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\DiscreteHedging.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)\DiscreteHedging.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DiscreteHedging - Win32 Release DLL"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release DLL" "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release DLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DiscreteHedging.obj"
	-@erase "$(INTDIR)\DiscreteHedging.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\DiscreteHedging.bsc"
	-@erase "$(OUTDIR)\DiscreteHedging.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /Ob2 /I "..\.." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\DiscreteHedging.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DiscreteHedging.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DiscreteHedging.sbr"

"$(OUTDIR)\DiscreteHedging.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\DiscreteHedging.pdb" /machine:I386 /out:"$(OUTDIR)\DiscreteHedging.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\DiscreteHedging.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)\DiscreteHedging.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "DiscreteHedging - Win32 Debug DLL"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug DLL" "$(OUTDIR)\DiscreteHedging.exe" "$(OUTDIR)\DiscreteHedging.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug DLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\DiscreteHedging.obj"
	-@erase "$(INTDIR)\DiscreteHedging.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\DiscreteHedging.bsc"
	-@erase "$(OUTDIR)\DiscreteHedging.exe"
	-@erase "$(OUTDIR)\DiscreteHedging.ilk"
	-@erase "$(OUTDIR)\DiscreteHedging.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\.." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\DiscreteHedging.pch" /YX"quantlib.hpp" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\DiscreteHedging.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\DiscreteHedging.sbr"

"$(OUTDIR)\DiscreteHedging.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\DiscreteHedging.pdb" /debug /machine:I386 /out:"$(OUTDIR)\DiscreteHedging.exe" /libpath:"..\..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\DiscreteHedging.obj" \
	"..\..\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)\DiscreteHedging.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("DiscreteHedging.dep")
!INCLUDE "DiscreteHedging.dep"
!ELSE 
!MESSAGE Warning: cannot find "DiscreteHedging.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "DiscreteHedging - Win32 Release" || "$(CFG)" == "DiscreteHedging - Win32 Debug" || "$(CFG)" == "DiscreteHedging - Win32 Release DLL" || "$(CFG)" == "DiscreteHedging - Win32 Debug DLL"
SOURCE=.\DiscreteHedging.cpp

"$(INTDIR)\DiscreteHedging.obj"	"$(INTDIR)\DiscreteHedging.sbr" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "DiscreteHedging - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\Examples\DiscreteHedging"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\Examples\DiscreteHedging"

!ELSEIF  "$(CFG)" == "DiscreteHedging - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\Examples\DiscreteHedging"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\Examples\DiscreteHedging"

!ELSEIF  "$(CFG)" == "DiscreteHedging - Win32 Release DLL"

"QuantLib - Win32 Release DLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release DLL" 
   cd ".\Examples\DiscreteHedging"

"QuantLib - Win32 Release DLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release DLL" RECURSE=1 CLEAN 
   cd ".\Examples\DiscreteHedging"

!ELSEIF  "$(CFG)" == "DiscreteHedging - Win32 Debug DLL"

"QuantLib - Win32 Debug DLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug DLL" 
   cd ".\Examples\DiscreteHedging"

"QuantLib - Win32 Debug DLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug DLL" RECURSE=1 CLEAN 
   cd ".\Examples\DiscreteHedging"

!ENDIF 


!ENDIF 

