# Microsoft Developer Studio Generated NMAKE File, Based on QuantLibFunctions.dsp
!IF "$(CFG)" == ""
CFG=QuantLibFunctions - Win32 Debug SingleThread
!MESSAGE No configuration specified. Defaulting to QuantLibFunctions - Win32 Debug SingleThread.
!ENDIF 

!IF "$(CFG)" != "QuantLibFunctions - Win32 Release" && "$(CFG)" != "QuantLibFunctions - Win32 Debug" && "$(CFG)" != "QuantLibFunctions - Win32 Release MTDLL" && "$(CFG)" != "QuantLibFunctions - Win32 Debug MTDLL" && "$(CFG)" != "QuantLibFunctions - Win32 Release SingleThread" && "$(CFG)" != "QuantLibFunctions - Win32 Debug SingleThread"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QuantLibFunctions.mak" CFG="QuantLibFunctions - Win32 Debug SingleThread"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QuantLibFunctions - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLibFunctions - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLibFunctions - Win32 Release MTDLL" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLibFunctions - Win32 Debug MTDLL" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLibFunctions - Win32 Release SingleThread" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLibFunctions - Win32 Debug SingleThread" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "QuantLibFunctions - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\lib\QuantLibFunctions-vc6-mt-s-0_3_8.lib"

!ELSE 

ALL : "QuantLib - Win32 Release" "..\..\..\lib\QuantLibFunctions-vc6-mt-s-0_3_8.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "..\..\..\lib\QuantLibFunctions-vc6-mt-s-0_3_8.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /GR /GX /O2 /I "..\.." /I "..\..\.." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLibFunctions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLibFunctions.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"..\..\..\lib\QuantLibFunctions-vc6-mt-s-0_3_8.lib" 
LIB32_OBJS= \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"..\..\..\lib\QuantLib-vc6-mt-s-0_3_8.lib"

"..\..\..\lib\QuantLibFunctions-vc6-mt-s-0_3_8.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\lib\QuantLibFunctions-vc6-mt-sgd-0_3_8.lib"

!ELSE 

ALL : "QuantLib - Win32 Debug" "..\..\..\lib\QuantLibFunctions-vc6-mt-sgd-0_3_8.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "..\..\..\lib\QuantLibFunctions-vc6-mt-sgd-0_3_8.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\.." /I "..\..\.." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLibFunctions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLibFunctions.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"..\..\..\lib\QuantLibFunctions-vc6-mt-sgd-0_3_8.lib" 
LIB32_OBJS= \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"..\..\..\lib\QuantLib-vc6-mt-sgd-0_3_8.lib"

"..\..\..\lib\QuantLibFunctions-vc6-mt-sgd-0_3_8.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Release MTDLL"

OUTDIR=.\build\ReleaseMTDLL
INTDIR=.\build\ReleaseMTDLL

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\lib\QuantLibFunctions-vc6-mt-0_3_8.lib"

!ELSE 

ALL : "QuantLib - Win32 Release MTDLL" "..\..\..\lib\QuantLibFunctions-vc6-mt-0_3_8.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "..\..\..\lib\QuantLibFunctions-vc6-mt-0_3_8.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /O2 /I "..\.." /I "..\..\.." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLibFunctions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLibFunctions.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"..\..\..\lib\QuantLibFunctions-vc6-mt-0_3_8.lib" 
LIB32_OBJS= \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"..\..\..\lib\QuantLib-vc6-mt-0_3_8.lib"

"..\..\..\lib\QuantLibFunctions-vc6-mt-0_3_8.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Debug MTDLL"

OUTDIR=.\build\DebugMTDLL
INTDIR=.\build\DebugMTDLL

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\lib\QuantLibFunctions-vc6-mt-gd-0_3_8.lib"

!ELSE 

ALL : "QuantLib - Win32 Debug MTDLL" "..\..\..\lib\QuantLibFunctions-vc6-mt-gd-0_3_8.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "..\..\..\lib\QuantLibFunctions-vc6-mt-gd-0_3_8.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\.." /I "..\..\.." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLibFunctions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLibFunctions.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"..\..\..\lib\QuantLibFunctions-vc6-mt-gd-0_3_8.lib" 
LIB32_OBJS= \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"..\..\..\lib\QuantLib-vc6-mt-gd-0_3_8.lib"

"..\..\..\lib\QuantLibFunctions-vc6-mt-gd-0_3_8.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Release SingleThread"

OUTDIR=.\build\ReleaseST
INTDIR=.\build\ReleaseST

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\lib\QuantLibFunctions-vc6-s-0_3_8.lib"

!ELSE 

ALL : "QuantLib - Win32 Release SingleThread" "..\..\..\lib\QuantLibFunctions-vc6-s-0_3_8.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "..\..\..\lib\QuantLibFunctions-vc6-s-0_3_8.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /GR /GX /O2 /I "..\.." /I "..\..\.." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLibFunctions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLibFunctions.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"..\..\..\lib\QuantLibFunctions-vc6-s-0_3_8.lib" 
LIB32_OBJS= \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"..\..\..\lib\QuantLib-vc6-s-0_3_8.lib"

"..\..\..\lib\QuantLibFunctions-vc6-s-0_3_8.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Debug SingleThread"

OUTDIR=.\build\DebugST
INTDIR=.\build\DebugST

!IF "$(RECURSE)" == "0" 

ALL : "..\..\..\lib\QuantLibFunctions-vc6-sgd-0_3_8.lib"

!ELSE 

ALL : "QuantLib - Win32 Debug SingleThread" "..\..\..\lib\QuantLibFunctions-vc6-sgd-0_3_8.lib"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "..\..\..\lib\QuantLibFunctions-vc6-sgd-0_3_8.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /GR /GX /ZI /Od /I "..\.." /I "..\..\.." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLibFunctions.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLibFunctions.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"..\..\..\lib\QuantLibFunctions-vc6-sgd-0_3_8.lib" 
LIB32_OBJS= \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"..\..\..\lib\QuantLib-vc6-sgd-0_3_8.lib"

"..\..\..\lib\QuantLibFunctions-vc6-sgd-0_3_8.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("QuantLibFunctions.dep")
!INCLUDE "QuantLibFunctions.dep"
!ELSE 
!MESSAGE Warning: cannot find "QuantLibFunctions.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QuantLibFunctions - Win32 Release" || "$(CFG)" == "QuantLibFunctions - Win32 Debug" || "$(CFG)" == "QuantLibFunctions - Win32 Release MTDLL" || "$(CFG)" == "QuantLibFunctions - Win32 Debug MTDLL" || "$(CFG)" == "QuantLibFunctions - Win32 Release SingleThread" || "$(CFG)" == "QuantLibFunctions - Win32 Debug SingleThread"
SOURCE=.\calendars.cpp

"$(INTDIR)\calendars.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\daycounters.cpp

"$(INTDIR)\daycounters.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mathf.cpp

"$(INTDIR)\mathf.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\vols.cpp

"$(INTDIR)\vols.obj" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "QuantLibFunctions - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Release MTDLL"

"QuantLib - Win32 Release MTDLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

"QuantLib - Win32 Release MTDLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" RECURSE=1 CLEAN 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Debug MTDLL"

"QuantLib - Win32 Debug MTDLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

"QuantLib - Win32 Debug MTDLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" RECURSE=1 CLEAN 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Release SingleThread"

"QuantLib - Win32 Release SingleThread" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release SingleThread" 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

"QuantLib - Win32 Release SingleThreadCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release SingleThread" RECURSE=1 CLEAN 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

!ELSEIF  "$(CFG)" == "QuantLibFunctions - Win32 Debug SingleThread"

"QuantLib - Win32 Debug SingleThread" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug SingleThread" 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

"QuantLib - Win32 Debug SingleThreadCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug SingleThread" RECURSE=1 CLEAN 
   cd ".\FUNCTIONS\QL\FUNCTIONS"

!ENDIF 


!ENDIF 

