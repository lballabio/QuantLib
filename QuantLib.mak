# Microsoft Developer Studio Generated NMAKE File, Based on QuantLib.dsp
!IF "$(CFG)" == ""
CFG=QuantLib - Win32 OnTheEdgeDebug
!MESSAGE No configuration specified. Defaulting to QuantLib - Win32 OnTheEdgeDebug.
!ENDIF 

!IF "$(CFG)" != "QuantLib - Win32 Release" && "$(CFG)" != "QuantLib - Win32 Debug" && "$(CFG)" != "QuantLib - Win32 OnTheEdgeDebug" && "$(CFG)" != "QuantLib - Win32 OnTheEdgeRelease"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QuantLib.mak" CFG="QuantLib - Win32 OnTheEdgeDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QuantLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "QuantLib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\bsmeuropeanoption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoption.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\everestoption.obj"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\mceuropeanpricer.obj"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\pagodaoption.obj"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\piecewiseconstantforwards.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\plainbasketoption.obj"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360italian.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\westerncalendar.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\thirty360italian.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\avgpriceasianpathpricer.obj" \
	"$(INTDIR)\avgstrikeasianpathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\controlvariatedpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricasianpathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\knuthrandomgenerator.obj" \
	"$(INTDIR)\lecuyerrandomgenerator.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmeuropeanoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\bsmoption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\everestoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\mceuropeanpricer.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\pagodaoption.obj" \
	"$(INTDIR)\plainbasketoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseconstantforwards.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

".\lib\Win32\VisualStudio\QuantLib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\bsmeuropeanoption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoption.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\everestoption.obj"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\mceuropeanpricer.obj"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\pagodaoption.obj"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\piecewiseconstantforwards.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\plainbasketoption.obj"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360italian.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\westerncalendar.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\thirty360italian.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\avgpriceasianpathpricer.obj" \
	"$(INTDIR)\avgstrikeasianpathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\controlvariatedpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricasianpathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\knuthrandomgenerator.obj" \
	"$(INTDIR)\lecuyerrandomgenerator.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmeuropeanoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\bsmoption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\everestoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\mceuropeanpricer.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\pagodaoption.obj" \
	"$(INTDIR)\plainbasketoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseconstantforwards.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

".\lib\Win32\VisualStudio\QuantLib_d.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

OUTDIR=.\OnTheEdgeDebug
INTDIR=.\OnTheEdgeDebug

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\bsmeuropeanoption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoption.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\everestoption.obj"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\mceuropeanpricer.obj"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\pagodaoption.obj"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\piecewiseconstantforwards.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\plainbasketoption.obj"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360italian.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\westerncalendar.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\thirty360italian.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\avgpriceasianpathpricer.obj" \
	"$(INTDIR)\avgstrikeasianpathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\controlvariatedpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricasianpathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\knuthrandomgenerator.obj" \
	"$(INTDIR)\lecuyerrandomgenerator.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmeuropeanoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\bsmoption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\everestoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\mceuropeanpricer.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\pagodaoption.obj" \
	"$(INTDIR)\plainbasketoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseconstantforwards.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

".\lib\Win32\VisualStudio\QuantLib_d.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

OUTDIR=.\OnTheEdgeRelease
INTDIR=.\OnTheEdgeRelease

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\bsmeuropeanoption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoption.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\everestoption.obj"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\mceuropeanpricer.obj"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\pagodaoption.obj"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\piecewiseconstantforwards.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\plainbasketoption.obj"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360italian.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\westerncalendar.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\thirty360italian.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\avgpriceasianpathpricer.obj" \
	"$(INTDIR)\avgstrikeasianpathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\controlvariatedpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricasianpathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\knuthrandomgenerator.obj" \
	"$(INTDIR)\lecuyerrandomgenerator.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmeuropeanoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\bsmoption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\everestoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\mceuropeanpricer.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\pagodaoption.obj" \
	"$(INTDIR)\plainbasketoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseconstantforwards.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

".\lib\Win32\VisualStudio\QuantLib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("QuantLib.dep")
!INCLUDE "QuantLib.dep"
!ELSE 
!MESSAGE Warning: cannot find "QuantLib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QuantLib - Win32 Release" || "$(CFG)" == "QuantLib - Win32 Debug" || "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug" || "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"
SOURCE=.\Sources\Calendars\frankfurt.cpp

"$(INTDIR)\frankfurt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\helsinki.cpp

"$(INTDIR)\helsinki.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\london.cpp

"$(INTDIR)\london.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\milan.cpp

"$(INTDIR)\milan.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\newyork.cpp

"$(INTDIR)\newyork.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\target.cpp

"$(INTDIR)\target.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\wellington.cpp

"$(INTDIR)\wellington.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\westerncalendar.cpp

"$(INTDIR)\westerncalendar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Calendars\zurich.cpp

"$(INTDIR)\zurich.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\DayCounters\actualactual.cpp

"$(INTDIR)\actualactual.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\DayCounters\thirty360.cpp

"$(INTDIR)\thirty360.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\DayCounters\thirty360italian.cpp

"$(INTDIR)\thirty360italian.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\FiniteDifferences\bsmoperator.cpp

"$(INTDIR)\bsmoperator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\FiniteDifferences\tridiagonaloperator.cpp

"$(INTDIR)\tridiagonaloperator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\FiniteDifferences\valueatcenter.cpp

"$(INTDIR)\valueatcenter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Instruments\simpleswap.cpp

"$(INTDIR)\simpleswap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Instruments\stock.cpp

"$(INTDIR)\stock.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Instruments\swap.cpp

"$(INTDIR)\swap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Math\matrix.cpp

"$(INTDIR)\matrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Math\multivariateaccumulator.cpp

"$(INTDIR)\multivariateaccumulator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Math\normaldistribution.cpp

"$(INTDIR)\normaldistribution.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Math\statistics.cpp

"$(INTDIR)\statistics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Math\symmetricschurdecomposition.cpp

"$(INTDIR)\symmetricschurdecomposition.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\avgpriceasianpathpricer.cpp

"$(INTDIR)\avgpriceasianpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\avgstrikeasianpathpricer.cpp

"$(INTDIR)\avgstrikeasianpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\basketpathpricer.cpp

"$(INTDIR)\basketpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\controlvariatedpathpricer.cpp

"$(INTDIR)\controlvariatedpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\europeanpathpricer.cpp

"$(INTDIR)\europeanpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\everestpathpricer.cpp

"$(INTDIR)\everestpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\geometricasianpathpricer.cpp

"$(INTDIR)\geometricasianpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\getcovariance.cpp

"$(INTDIR)\getcovariance.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\himalayapathpricer.cpp

"$(INTDIR)\himalayapathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\knuthrandomgenerator.cpp

"$(INTDIR)\knuthrandomgenerator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\lecuyerrandomgenerator.cpp

"$(INTDIR)\lecuyerrandomgenerator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\MonteCarlo\pagodapathpricer.cpp

"$(INTDIR)\pagodapathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\averagepriceasian.cpp

"$(INTDIR)\averagepriceasian.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\averagestrikeasian.cpp

"$(INTDIR)\averagestrikeasian.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\barrieroption.cpp

"$(INTDIR)\barrieroption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\bermudanoption.cpp

"$(INTDIR)\bermudanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\binaryoption.cpp

"$(INTDIR)\binaryoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\bsmeuropeanoption.cpp

"$(INTDIR)\bsmeuropeanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\bsmnumericaloption.cpp

"$(INTDIR)\bsmnumericaloption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\bsmoption.cpp

"$(INTDIR)\bsmoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\cliquetoption.cpp

"$(INTDIR)\cliquetoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\dividendamericanoption.cpp

"$(INTDIR)\dividendamericanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\dividendeuropeanoption.cpp

"$(INTDIR)\dividendeuropeanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\dividendoption.cpp

"$(INTDIR)\dividendoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\dividendshoutoption.cpp

"$(INTDIR)\dividendshoutoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\everestoption.cpp

"$(INTDIR)\everestoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\finitedifferenceeuropean.cpp

"$(INTDIR)\finitedifferenceeuropean.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\himalaya.cpp

"$(INTDIR)\himalaya.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\mceuropeanpricer.cpp

"$(INTDIR)\mceuropeanpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\multiperiodoption.cpp

"$(INTDIR)\multiperiodoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\pagodaoption.cpp

"$(INTDIR)\pagodaoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\plainbasketoption.cpp

"$(INTDIR)\plainbasketoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Pricers\stepconditionoption.cpp

"$(INTDIR)\stepconditionoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\bisection.cpp

"$(INTDIR)\bisection.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\brent.cpp

"$(INTDIR)\brent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\falseposition.cpp

"$(INTDIR)\falseposition.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\newton.cpp

"$(INTDIR)\newton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\newtonsafe.cpp

"$(INTDIR)\newtonsafe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\ridder.cpp

"$(INTDIR)\ridder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Solvers1D\secant.cpp

"$(INTDIR)\secant.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\TermStructures\piecewiseconstantforwards.cpp

"$(INTDIR)\piecewiseconstantforwards.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\TermStructures\piecewiseflatforward.cpp

"$(INTDIR)\piecewiseflatforward.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\TermStructures\ratehelpers.cpp

"$(INTDIR)\ratehelpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Indexes\xibor.cpp

"$(INTDIR)\xibor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\Indexes\xibormanager.cpp

"$(INTDIR)\xibormanager.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\CashFlows\cashflowvectors.cpp

"$(INTDIR)\cashflowvectors.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\CashFlows\floatingratecoupon.cpp

"$(INTDIR)\floatingratecoupon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\calendar.cpp

"$(INTDIR)\calendar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\dataformatters.cpp

"$(INTDIR)\dataformatters.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\date.cpp

"$(INTDIR)\date.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\scheduler.cpp

"$(INTDIR)\scheduler.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\Sources\solver1d.cpp

"$(INTDIR)\solver1d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

