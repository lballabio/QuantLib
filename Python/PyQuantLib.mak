# Microsoft Developer Studio Generated NMAKE File, Based on PyQuantLib.dsp
!IF "$(CFG)" == ""
CFG=PyQuantLib - Win32 Release
!MESSAGE No configuration specified. Defaulting to PyQuantLib - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "PyQuantLib - Win32 Release" && "$(CFG)" != "PyQuantLib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PyQuantLib.mak" CFG="PyQuantLib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PyQuantLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PyQuantLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PyQuantLib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

!IF "$(RECURSE)" == "0" 

ALL : ".\QuantLibc.dll"

!ELSE 

ALL : "QuantLib - Win32 Release" ".\QuantLibc.dll"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\quantlib_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\QuantLibc.exp"
	-@erase "$(OUTDIR)\QuantLibc.lib"
	-@erase ".\QuantLibc.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /GX /Od /I "$(QL_DIR)\Include" /I "$(PYTHON_HOME)\Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\PyQuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PyQuantLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\QuantLibc.pdb" /machine:I386 /out:".\QuantLibc.dll" /implib:"$(OUTDIR)\QuantLibc.lib" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio" /libpath:"$(PYTHON_HOME)\libs" /export:initQuantLibc 
LINK32_OBJS= \
	"$(INTDIR)\quantlib_wrap.obj" \
	"..\lib\Win32\VisualStudio\QuantLib.lib"

".\QuantLibc.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "PyQuantLib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

!IF "$(RECURSE)" == "0" 

ALL : ".\QuantLibc_d.dll"

!ELSE 

ALL : "QuantLib - Win32 Debug" ".\QuantLibc_d.dll"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\quantlib_wrap.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\QuantLibc_d.exp"
	-@erase "$(OUTDIR)\QuantLibc_d.lib"
	-@erase "$(OUTDIR)\QuantLibc_d.pdb"
	-@erase ".\QuantLibc_d.dll"
	-@erase ".\QuantLibc_d.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /GX /ZI /Od /I "$(QL_DIR)\Include" /I "$(PYTHON_HOME)\Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Fp"$(INTDIR)\PyQuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\PyQuantLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\QuantLibc_d.pdb" /debug /machine:I386 /out:".\QuantLibc_d.dll" /implib:"$(OUTDIR)\QuantLibc_d.lib" /libpath:"$(QL_DIR)\lib\Win32\VisualStudio" /libpath:"$(PYTHON_HOME)\libs" /export:initQuantLibc 
LINK32_OBJS= \
	"$(INTDIR)\quantlib_wrap.obj" \
	"..\lib\Win32\VisualStudio\QuantLib_d.lib"

".\QuantLibc_d.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("PyQuantLib.dep")
!INCLUDE "PyQuantLib.dep"
!ELSE 
!MESSAGE Warning: cannot find "PyQuantLib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "PyQuantLib - Win32 Release" || "$(CFG)" == "PyQuantLib - Win32 Debug"
SOURCE=quantlib_wrap.cpp

"$(INTDIR)\quantlib_wrap.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "PyQuantLib - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Backed_up\_projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\Python"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Backed_up\_projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\Python"

!ELSEIF  "$(CFG)" == "PyQuantLib - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Backed_up\_projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\Python"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Backed_up\_projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\Python"

!ENDIF 


!ENDIF 

