# Microsoft Developer Studio Generated NMAKE File, Based on testsuite.dsp
!IF "$(CFG)" == ""
CFG=testsuite - Win32 Debug MTDLL
!MESSAGE No configuration specified. Defaulting to testsuite - Win32 Debug MTDLL.
!ENDIF 

!IF "$(CFG)" != "testsuite - Win32 Release" && "$(CFG)" != "testsuite - Win32 Debug" && "$(CFG)" != "testsuite - Win32 Release MTDLL" && "$(CFG)" != "testsuite - Win32 Debug MTDLL"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak" CFG="testsuite - Win32 Debug MTDLL"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testsuite - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Release MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug MTDLL" (based on "Win32 (x86) Console Application")
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

!IF  "$(CFG)" == "testsuite - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release" "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\americanoption.obj"
	-@erase "$(INTDIR)\americanoption.sbr"
	-@erase "$(INTDIR)\asianoptions.obj"
	-@erase "$(INTDIR)\asianoptions.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\calendars.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\covariance.obj"
	-@erase "$(INTDIR)\covariance.sbr"
	-@erase "$(INTDIR)\dates.obj"
	-@erase "$(INTDIR)\dates.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\digitaloption.obj"
	-@erase "$(INTDIR)\digitaloption.sbr"
	-@erase "$(INTDIR)\distributions.obj"
	-@erase "$(INTDIR)\distributions.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\instruments.obj"
	-@erase "$(INTDIR)\instruments.sbr"
	-@erase "$(INTDIR)\integrals.obj"
	-@erase "$(INTDIR)\integrals.sbr"
	-@erase "$(INTDIR)\interpolations.obj"
	-@erase "$(INTDIR)\interpolations.sbr"
	-@erase "$(INTDIR)\jumpdiffusion.obj"
	-@erase "$(INTDIR)\jumpdiffusion.sbr"
	-@erase "$(INTDIR)\lowdiscrepancysequences.obj"
	-@erase "$(INTDIR)\lowdiscrepancysequences.sbr"
	-@erase "$(INTDIR)\marketelements.obj"
	-@erase "$(INTDIR)\marketelements.sbr"
	-@erase "$(INTDIR)\matrices.obj"
	-@erase "$(INTDIR)\matrices.sbr"
	-@erase "$(INTDIR)\mersennetwister.obj"
	-@erase "$(INTDIR)\mersennetwister.sbr"
	-@erase "$(INTDIR)\old_pricers.obj"
	-@erase "$(INTDIR)\old_pricers.sbr"
	-@erase "$(INTDIR)\operators.obj"
	-@erase "$(INTDIR)\operators.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\qltestlistener.obj"
	-@erase "$(INTDIR)\qltestlistener.sbr"
	-@erase "$(INTDIR)\quantlibtestsuite.obj"
	-@erase "$(INTDIR)\quantlibtestsuite.sbr"
	-@erase "$(INTDIR)\riskstats.obj"
	-@erase "$(INTDIR)\riskstats.sbr"
	-@erase "$(INTDIR)\solvers.obj"
	-@erase "$(INTDIR)\solvers.sbr"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\stats.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\termstructures.obj"
	-@erase "$(INTDIR)\termstructures.sbr"
	-@erase "$(INTDIR)\utilities.obj"
	-@erase "$(INTDIR)\utilities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testsuite.bsc"
	-@erase "$(OUTDIR)\testsuite.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I ".." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\testsuite.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testsuite.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\americanoption.sbr" \
	"$(INTDIR)\asianoptions.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\calendars.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\covariance.sbr" \
	"$(INTDIR)\dates.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\digitaloption.sbr" \
	"$(INTDIR)\distributions.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\instruments.sbr" \
	"$(INTDIR)\integrals.sbr" \
	"$(INTDIR)\interpolations.sbr" \
	"$(INTDIR)\jumpdiffusion.sbr" \
	"$(INTDIR)\lowdiscrepancysequences.sbr" \
	"$(INTDIR)\marketelements.sbr" \
	"$(INTDIR)\matrices.sbr" \
	"$(INTDIR)\mersennetwister.sbr" \
	"$(INTDIR)\old_pricers.sbr" \
	"$(INTDIR)\operators.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\qltestlistener.sbr" \
	"$(INTDIR)\quantlibtestsuite.sbr" \
	"$(INTDIR)\riskstats.sbr" \
	"$(INTDIR)\solvers.sbr" \
	"$(INTDIR)\stats.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\termstructures.sbr" \
	"$(INTDIR)\utilities.sbr"

"$(OUTDIR)\testsuite.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=cppunit.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\testsuite.pdb" /machine:I386 /out:"$(OUTDIR)\testsuite.exe" /libpath:"..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\americanoption.obj" \
	"$(INTDIR)\asianoptions.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\covariance.obj" \
	"$(INTDIR)\dates.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\digitaloption.obj" \
	"$(INTDIR)\distributions.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\instruments.obj" \
	"$(INTDIR)\integrals.obj" \
	"$(INTDIR)\interpolations.obj" \
	"$(INTDIR)\jumpdiffusion.obj" \
	"$(INTDIR)\lowdiscrepancysequences.obj" \
	"$(INTDIR)\marketelements.obj" \
	"$(INTDIR)\matrices.obj" \
	"$(INTDIR)\mersennetwister.obj" \
	"$(INTDIR)\old_pricers.obj" \
	"$(INTDIR)\operators.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\qltestlistener.obj" \
	"$(INTDIR)\quantlibtestsuite.obj" \
	"$(INTDIR)\riskstats.obj" \
	"$(INTDIR)\solvers.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\termstructures.obj" \
	"$(INTDIR)\utilities.obj" \
	"..\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)\testsuite.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug" "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\americanoption.obj"
	-@erase "$(INTDIR)\americanoption.sbr"
	-@erase "$(INTDIR)\asianoptions.obj"
	-@erase "$(INTDIR)\asianoptions.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\calendars.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\covariance.obj"
	-@erase "$(INTDIR)\covariance.sbr"
	-@erase "$(INTDIR)\dates.obj"
	-@erase "$(INTDIR)\dates.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\digitaloption.obj"
	-@erase "$(INTDIR)\digitaloption.sbr"
	-@erase "$(INTDIR)\distributions.obj"
	-@erase "$(INTDIR)\distributions.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\instruments.obj"
	-@erase "$(INTDIR)\instruments.sbr"
	-@erase "$(INTDIR)\integrals.obj"
	-@erase "$(INTDIR)\integrals.sbr"
	-@erase "$(INTDIR)\interpolations.obj"
	-@erase "$(INTDIR)\interpolations.sbr"
	-@erase "$(INTDIR)\jumpdiffusion.obj"
	-@erase "$(INTDIR)\jumpdiffusion.sbr"
	-@erase "$(INTDIR)\lowdiscrepancysequences.obj"
	-@erase "$(INTDIR)\lowdiscrepancysequences.sbr"
	-@erase "$(INTDIR)\marketelements.obj"
	-@erase "$(INTDIR)\marketelements.sbr"
	-@erase "$(INTDIR)\matrices.obj"
	-@erase "$(INTDIR)\matrices.sbr"
	-@erase "$(INTDIR)\mersennetwister.obj"
	-@erase "$(INTDIR)\mersennetwister.sbr"
	-@erase "$(INTDIR)\old_pricers.obj"
	-@erase "$(INTDIR)\old_pricers.sbr"
	-@erase "$(INTDIR)\operators.obj"
	-@erase "$(INTDIR)\operators.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\qltestlistener.obj"
	-@erase "$(INTDIR)\qltestlistener.sbr"
	-@erase "$(INTDIR)\quantlibtestsuite.obj"
	-@erase "$(INTDIR)\quantlibtestsuite.sbr"
	-@erase "$(INTDIR)\riskstats.obj"
	-@erase "$(INTDIR)\riskstats.sbr"
	-@erase "$(INTDIR)\solvers.obj"
	-@erase "$(INTDIR)\solvers.sbr"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\stats.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\termstructures.obj"
	-@erase "$(INTDIR)\termstructures.sbr"
	-@erase "$(INTDIR)\utilities.obj"
	-@erase "$(INTDIR)\utilities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testsuite.bsc"
	-@erase "$(OUTDIR)\testsuite.exe"
	-@erase "$(OUTDIR)\testsuite.ilk"
	-@erase "$(OUTDIR)\testsuite.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\testsuite.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testsuite.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\americanoption.sbr" \
	"$(INTDIR)\asianoptions.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\calendars.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\covariance.sbr" \
	"$(INTDIR)\dates.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\digitaloption.sbr" \
	"$(INTDIR)\distributions.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\instruments.sbr" \
	"$(INTDIR)\integrals.sbr" \
	"$(INTDIR)\interpolations.sbr" \
	"$(INTDIR)\jumpdiffusion.sbr" \
	"$(INTDIR)\lowdiscrepancysequences.sbr" \
	"$(INTDIR)\marketelements.sbr" \
	"$(INTDIR)\matrices.sbr" \
	"$(INTDIR)\mersennetwister.sbr" \
	"$(INTDIR)\old_pricers.sbr" \
	"$(INTDIR)\operators.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\qltestlistener.sbr" \
	"$(INTDIR)\quantlibtestsuite.sbr" \
	"$(INTDIR)\riskstats.sbr" \
	"$(INTDIR)\solvers.sbr" \
	"$(INTDIR)\stats.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\termstructures.sbr" \
	"$(INTDIR)\utilities.sbr"

"$(OUTDIR)\testsuite.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=cppunitd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\testsuite.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testsuite.exe" /pdbtype:sept /libpath:"..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\americanoption.obj" \
	"$(INTDIR)\asianoptions.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\covariance.obj" \
	"$(INTDIR)\dates.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\digitaloption.obj" \
	"$(INTDIR)\distributions.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\instruments.obj" \
	"$(INTDIR)\integrals.obj" \
	"$(INTDIR)\interpolations.obj" \
	"$(INTDIR)\jumpdiffusion.obj" \
	"$(INTDIR)\lowdiscrepancysequences.obj" \
	"$(INTDIR)\marketelements.obj" \
	"$(INTDIR)\matrices.obj" \
	"$(INTDIR)\mersennetwister.obj" \
	"$(INTDIR)\old_pricers.obj" \
	"$(INTDIR)\operators.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\qltestlistener.obj" \
	"$(INTDIR)\quantlibtestsuite.obj" \
	"$(INTDIR)\riskstats.obj" \
	"$(INTDIR)\solvers.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\termstructures.obj" \
	"$(INTDIR)\utilities.obj" \
	"..\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)\testsuite.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "testsuite - Win32 Release MTDLL"

OUTDIR=.\build\ReleaseMTDLL
INTDIR=.\build\ReleaseMTDLL
# Begin Custom Macros
OutDir=.\build\ReleaseMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ELSE 

ALL : "QuantLib - Win32 Release MTDLL" "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Release MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\americanoption.obj"
	-@erase "$(INTDIR)\americanoption.sbr"
	-@erase "$(INTDIR)\asianoptions.obj"
	-@erase "$(INTDIR)\asianoptions.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\calendars.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\covariance.obj"
	-@erase "$(INTDIR)\covariance.sbr"
	-@erase "$(INTDIR)\dates.obj"
	-@erase "$(INTDIR)\dates.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\digitaloption.obj"
	-@erase "$(INTDIR)\digitaloption.sbr"
	-@erase "$(INTDIR)\distributions.obj"
	-@erase "$(INTDIR)\distributions.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\instruments.obj"
	-@erase "$(INTDIR)\instruments.sbr"
	-@erase "$(INTDIR)\integrals.obj"
	-@erase "$(INTDIR)\integrals.sbr"
	-@erase "$(INTDIR)\interpolations.obj"
	-@erase "$(INTDIR)\interpolations.sbr"
	-@erase "$(INTDIR)\jumpdiffusion.obj"
	-@erase "$(INTDIR)\jumpdiffusion.sbr"
	-@erase "$(INTDIR)\lowdiscrepancysequences.obj"
	-@erase "$(INTDIR)\lowdiscrepancysequences.sbr"
	-@erase "$(INTDIR)\marketelements.obj"
	-@erase "$(INTDIR)\marketelements.sbr"
	-@erase "$(INTDIR)\matrices.obj"
	-@erase "$(INTDIR)\matrices.sbr"
	-@erase "$(INTDIR)\mersennetwister.obj"
	-@erase "$(INTDIR)\mersennetwister.sbr"
	-@erase "$(INTDIR)\old_pricers.obj"
	-@erase "$(INTDIR)\old_pricers.sbr"
	-@erase "$(INTDIR)\operators.obj"
	-@erase "$(INTDIR)\operators.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\qltestlistener.obj"
	-@erase "$(INTDIR)\qltestlistener.sbr"
	-@erase "$(INTDIR)\quantlibtestsuite.obj"
	-@erase "$(INTDIR)\quantlibtestsuite.sbr"
	-@erase "$(INTDIR)\riskstats.obj"
	-@erase "$(INTDIR)\riskstats.sbr"
	-@erase "$(INTDIR)\solvers.obj"
	-@erase "$(INTDIR)\solvers.sbr"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\stats.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\termstructures.obj"
	-@erase "$(INTDIR)\termstructures.sbr"
	-@erase "$(INTDIR)\utilities.obj"
	-@erase "$(INTDIR)\utilities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\testsuite.bsc"
	-@erase "$(OUTDIR)\testsuite.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I ".." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\testsuite.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testsuite.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\americanoption.sbr" \
	"$(INTDIR)\asianoptions.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\calendars.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\covariance.sbr" \
	"$(INTDIR)\dates.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\digitaloption.sbr" \
	"$(INTDIR)\distributions.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\instruments.sbr" \
	"$(INTDIR)\integrals.sbr" \
	"$(INTDIR)\interpolations.sbr" \
	"$(INTDIR)\jumpdiffusion.sbr" \
	"$(INTDIR)\lowdiscrepancysequences.sbr" \
	"$(INTDIR)\marketelements.sbr" \
	"$(INTDIR)\matrices.sbr" \
	"$(INTDIR)\mersennetwister.sbr" \
	"$(INTDIR)\old_pricers.sbr" \
	"$(INTDIR)\operators.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\qltestlistener.sbr" \
	"$(INTDIR)\quantlibtestsuite.sbr" \
	"$(INTDIR)\riskstats.sbr" \
	"$(INTDIR)\solvers.sbr" \
	"$(INTDIR)\stats.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\termstructures.sbr" \
	"$(INTDIR)\utilities.sbr"

"$(OUTDIR)\testsuite.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=cppunit.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\testsuite.pdb" /machine:I386 /out:"$(OUTDIR)\testsuite.exe" /libpath:"..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\americanoption.obj" \
	"$(INTDIR)\asianoptions.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\covariance.obj" \
	"$(INTDIR)\dates.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\digitaloption.obj" \
	"$(INTDIR)\distributions.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\instruments.obj" \
	"$(INTDIR)\integrals.obj" \
	"$(INTDIR)\interpolations.obj" \
	"$(INTDIR)\jumpdiffusion.obj" \
	"$(INTDIR)\lowdiscrepancysequences.obj" \
	"$(INTDIR)\marketelements.obj" \
	"$(INTDIR)\matrices.obj" \
	"$(INTDIR)\mersennetwister.obj" \
	"$(INTDIR)\old_pricers.obj" \
	"$(INTDIR)\operators.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\qltestlistener.obj" \
	"$(INTDIR)\quantlibtestsuite.obj" \
	"$(INTDIR)\riskstats.obj" \
	"$(INTDIR)\solvers.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\termstructures.obj" \
	"$(INTDIR)\utilities.obj" \
	"..\lib\Win32\VisualStudio\QuantLib_MTDLL.lib"

"$(OUTDIR)\testsuite.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug MTDLL"

OUTDIR=.\build\DebugMTDLL
INTDIR=.\build\DebugMTDLL
# Begin Custom Macros
OutDir=.\build\DebugMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ELSE 

ALL : "QuantLib - Win32 Debug MTDLL" "$(OUTDIR)\testsuite.exe" "$(OUTDIR)\testsuite.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLib - Win32 Debug MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\americanoption.obj"
	-@erase "$(INTDIR)\americanoption.sbr"
	-@erase "$(INTDIR)\asianoptions.obj"
	-@erase "$(INTDIR)\asianoptions.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\calendars.obj"
	-@erase "$(INTDIR)\calendars.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\covariance.obj"
	-@erase "$(INTDIR)\covariance.sbr"
	-@erase "$(INTDIR)\dates.obj"
	-@erase "$(INTDIR)\dates.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\digitaloption.obj"
	-@erase "$(INTDIR)\digitaloption.sbr"
	-@erase "$(INTDIR)\distributions.obj"
	-@erase "$(INTDIR)\distributions.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\instruments.obj"
	-@erase "$(INTDIR)\instruments.sbr"
	-@erase "$(INTDIR)\integrals.obj"
	-@erase "$(INTDIR)\integrals.sbr"
	-@erase "$(INTDIR)\interpolations.obj"
	-@erase "$(INTDIR)\interpolations.sbr"
	-@erase "$(INTDIR)\jumpdiffusion.obj"
	-@erase "$(INTDIR)\jumpdiffusion.sbr"
	-@erase "$(INTDIR)\lowdiscrepancysequences.obj"
	-@erase "$(INTDIR)\lowdiscrepancysequences.sbr"
	-@erase "$(INTDIR)\marketelements.obj"
	-@erase "$(INTDIR)\marketelements.sbr"
	-@erase "$(INTDIR)\matrices.obj"
	-@erase "$(INTDIR)\matrices.sbr"
	-@erase "$(INTDIR)\mersennetwister.obj"
	-@erase "$(INTDIR)\mersennetwister.sbr"
	-@erase "$(INTDIR)\old_pricers.obj"
	-@erase "$(INTDIR)\old_pricers.sbr"
	-@erase "$(INTDIR)\operators.obj"
	-@erase "$(INTDIR)\operators.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\qltestlistener.obj"
	-@erase "$(INTDIR)\qltestlistener.sbr"
	-@erase "$(INTDIR)\quantlibtestsuite.obj"
	-@erase "$(INTDIR)\quantlibtestsuite.sbr"
	-@erase "$(INTDIR)\riskstats.obj"
	-@erase "$(INTDIR)\riskstats.sbr"
	-@erase "$(INTDIR)\solvers.obj"
	-@erase "$(INTDIR)\solvers.sbr"
	-@erase "$(INTDIR)\stats.obj"
	-@erase "$(INTDIR)\stats.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\termstructures.obj"
	-@erase "$(INTDIR)\termstructures.sbr"
	-@erase "$(INTDIR)\utilities.obj"
	-@erase "$(INTDIR)\utilities.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\testsuite.bsc"
	-@erase "$(OUTDIR)\testsuite.exe"
	-@erase "$(OUTDIR)\testsuite.ilk"
	-@erase "$(OUTDIR)\testsuite.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\testsuite.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\testsuite.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\americanoption.sbr" \
	"$(INTDIR)\asianoptions.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\calendars.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\covariance.sbr" \
	"$(INTDIR)\dates.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\digitaloption.sbr" \
	"$(INTDIR)\distributions.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\instruments.sbr" \
	"$(INTDIR)\integrals.sbr" \
	"$(INTDIR)\interpolations.sbr" \
	"$(INTDIR)\jumpdiffusion.sbr" \
	"$(INTDIR)\lowdiscrepancysequences.sbr" \
	"$(INTDIR)\marketelements.sbr" \
	"$(INTDIR)\matrices.sbr" \
	"$(INTDIR)\mersennetwister.sbr" \
	"$(INTDIR)\old_pricers.sbr" \
	"$(INTDIR)\operators.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\qltestlistener.sbr" \
	"$(INTDIR)\quantlibtestsuite.sbr" \
	"$(INTDIR)\riskstats.sbr" \
	"$(INTDIR)\solvers.sbr" \
	"$(INTDIR)\stats.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\termstructures.sbr" \
	"$(INTDIR)\utilities.sbr"

"$(OUTDIR)\testsuite.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
LINK32_FLAGS=cppunitd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\testsuite.pdb" /debug /machine:I386 /out:"$(OUTDIR)\testsuite.exe" /pdbtype:sept /libpath:"..\lib\Win32\VisualStudio\\" 
LINK32_OBJS= \
	"$(INTDIR)\americanoption.obj" \
	"$(INTDIR)\asianoptions.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\calendars.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\covariance.obj" \
	"$(INTDIR)\dates.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\digitaloption.obj" \
	"$(INTDIR)\distributions.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\instruments.obj" \
	"$(INTDIR)\integrals.obj" \
	"$(INTDIR)\interpolations.obj" \
	"$(INTDIR)\jumpdiffusion.obj" \
	"$(INTDIR)\lowdiscrepancysequences.obj" \
	"$(INTDIR)\marketelements.obj" \
	"$(INTDIR)\matrices.obj" \
	"$(INTDIR)\mersennetwister.obj" \
	"$(INTDIR)\old_pricers.obj" \
	"$(INTDIR)\operators.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\qltestlistener.obj" \
	"$(INTDIR)\quantlibtestsuite.obj" \
	"$(INTDIR)\riskstats.obj" \
	"$(INTDIR)\solvers.obj" \
	"$(INTDIR)\stats.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\termstructures.obj" \
	"$(INTDIR)\utilities.obj" \
	"..\lib\Win32\VisualStudio\QuantLib_MTDLL_d.lib"

"$(OUTDIR)\testsuite.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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
!IF EXISTS("testsuite.dep")
!INCLUDE "testsuite.dep"
!ELSE 
!MESSAGE Warning: cannot find "testsuite.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "testsuite - Win32 Release" || "$(CFG)" == "testsuite - Win32 Debug" || "$(CFG)" == "testsuite - Win32 Release MTDLL" || "$(CFG)" == "testsuite - Win32 Debug MTDLL"
SOURCE=.\americanoption.cpp

"$(INTDIR)\americanoption.obj"	"$(INTDIR)\americanoption.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\asianoptions.cpp

"$(INTDIR)\asianoptions.obj"	"$(INTDIR)\asianoptions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\barrieroption.cpp

"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\basketoption.cpp

"$(INTDIR)\basketoption.obj"	"$(INTDIR)\basketoption.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\calendars.cpp

"$(INTDIR)\calendars.obj"	"$(INTDIR)\calendars.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\capfloor.cpp

"$(INTDIR)\capfloor.obj"	"$(INTDIR)\capfloor.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\compoundforward.cpp

"$(INTDIR)\compoundforward.obj"	"$(INTDIR)\compoundforward.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\covariance.cpp

"$(INTDIR)\covariance.obj"	"$(INTDIR)\covariance.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\dates.cpp

"$(INTDIR)\dates.obj"	"$(INTDIR)\dates.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\daycounters.cpp

"$(INTDIR)\daycounters.obj"	"$(INTDIR)\daycounters.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\digitaloption.cpp

"$(INTDIR)\digitaloption.obj"	"$(INTDIR)\digitaloption.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\distributions.cpp

"$(INTDIR)\distributions.obj"	"$(INTDIR)\distributions.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\europeanoption.cpp

"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\factorial.cpp

"$(INTDIR)\factorial.obj"	"$(INTDIR)\factorial.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\instruments.cpp

"$(INTDIR)\instruments.obj"	"$(INTDIR)\instruments.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\integrals.cpp

"$(INTDIR)\integrals.obj"	"$(INTDIR)\integrals.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\interpolations.cpp

"$(INTDIR)\interpolations.obj"	"$(INTDIR)\interpolations.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\jumpdiffusion.cpp

"$(INTDIR)\jumpdiffusion.obj"	"$(INTDIR)\jumpdiffusion.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\lowdiscrepancysequences.cpp

"$(INTDIR)\lowdiscrepancysequences.obj"	"$(INTDIR)\lowdiscrepancysequences.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\marketelements.cpp

"$(INTDIR)\marketelements.obj"	"$(INTDIR)\marketelements.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\matrices.cpp

"$(INTDIR)\matrices.obj"	"$(INTDIR)\matrices.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\mersennetwister.cpp

"$(INTDIR)\mersennetwister.obj"	"$(INTDIR)\mersennetwister.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\old_pricers.cpp

"$(INTDIR)\old_pricers.obj"	"$(INTDIR)\old_pricers.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\operators.cpp

"$(INTDIR)\operators.obj"	"$(INTDIR)\operators.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\piecewiseflatforward.cpp

"$(INTDIR)\piecewiseflatforward.obj"	"$(INTDIR)\piecewiseflatforward.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\qltestlistener.cpp

"$(INTDIR)\qltestlistener.obj"	"$(INTDIR)\qltestlistener.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\quantlibtestsuite.cpp

"$(INTDIR)\quantlibtestsuite.obj"	"$(INTDIR)\quantlibtestsuite.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\riskstats.cpp

"$(INTDIR)\riskstats.obj"	"$(INTDIR)\riskstats.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\solvers.cpp

"$(INTDIR)\solvers.obj"	"$(INTDIR)\solvers.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\stats.cpp

"$(INTDIR)\stats.obj"	"$(INTDIR)\stats.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\swap.cpp

"$(INTDIR)\swap.obj"	"$(INTDIR)\swap.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\swaption.cpp

"$(INTDIR)\swaption.obj"	"$(INTDIR)\swaption.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\termstructures.cpp

"$(INTDIR)\termstructures.obj"	"$(INTDIR)\termstructures.sbr" : $(SOURCE) "$(INTDIR)"


SOURCE=.\utilities.cpp

"$(INTDIR)\utilities.obj"	"$(INTDIR)\utilities.sbr" : $(SOURCE) "$(INTDIR)"


!IF  "$(CFG)" == "testsuite - Win32 Release"

"QuantLib - Win32 Release" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" 
   cd ".\test-suite"

"QuantLib - Win32 ReleaseCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release" RECURSE=1 CLEAN 
   cd ".\test-suite"

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug"

"QuantLib - Win32 Debug" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" 
   cd ".\test-suite"

"QuantLib - Win32 DebugCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug" RECURSE=1 CLEAN 
   cd ".\test-suite"

!ELSEIF  "$(CFG)" == "testsuite - Win32 Release MTDLL"

"QuantLib - Win32 Release MTDLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" 
   cd ".\test-suite"

"QuantLib - Win32 Release MTDLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Release MTDLL" RECURSE=1 CLEAN 
   cd ".\test-suite"

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug MTDLL"

"QuantLib - Win32 Debug MTDLL" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" 
   cd ".\test-suite"

"QuantLib - Win32 Debug MTDLLCLEAN" : 
   cd "\Projects\QuantLib"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLib.mak CFG="QuantLib - Win32 Debug MTDLL" RECURSE=1 CLEAN 
   cd ".\test-suite"

!ENDIF 


!ENDIF 

