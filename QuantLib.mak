# Microsoft Developer Studio Generated NMAKE File, Based on QuantLib.dsp
!IF "$(CFG)" == ""
CFG=QuantLib - Win32 OnTheEdgeDebug
!MESSAGE No configuration specified. Defaulting to QuantLib - Win32 OnTheEdgeDebug.
!ENDIF 

!IF "$(CFG)" != "QuantLib - Win32 Release" && "$(CFG)" != "QuantLib - Win32 Debug" && "$(CFG)" != "QuantLib - Win32 OnTheEdgeRelease" && "$(CFG)" != "QuantLib - Win32 OnTheEdgeDebug"
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
!MESSAGE "QuantLib - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Static Library")
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

!IF  "$(CFG)" == "QuantLib - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib" "$(OUTDIR)\QuantLib.bsc"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagepriceasian.sbr"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.sbr"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.sbr"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basket.obj"
	-@erase "$(INTDIR)\basket.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\bermudanoption.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendamericanoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendoption.sbr"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.sbr"
	-@erase "$(INTDIR)\europeanengine.obj"
	-@erase "$(INTDIR)\europeanengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everest.obj"
	-@erase "$(INTDIR)\everest.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalaya.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.sbr"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multiperiodoption.sbr"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.sbr"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newton.sbr"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newtonsafe.sbr"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\newyork.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagoda.obj"
	-@erase "$(INTDIR)\pagoda.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\plainoption.obj"
	-@erase "$(INTDIR)\plainoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\segmentintegral.obj"
	-@erase "$(INTDIR)\segmentintegral.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\singleassetpathpricer.obj"
	-@erase "$(INTDIR)\singleassetpathpricer.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stepconditionoption.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\westerncalendar.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /Od /Ob2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\westerncalendar.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\plainoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\segmentintegral.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\avgpriceasianpathpricer.sbr" \
	"$(INTDIR)\avgstrikeasianpathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\controlvariatedpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricasianpathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\knuthrandomgenerator.sbr" \
	"$(INTDIR)\lecuyerrandomgenerator.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\singleassetpathpricer.sbr" \
	"$(INTDIR)\averagepriceasian.sbr" \
	"$(INTDIR)\averagestrikeasian.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\bermudanoption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\bsmnumericaloption.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendamericanoption.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\dividendoption.sbr" \
	"$(INTDIR)\dividendshoutoption.sbr" \
	"$(INTDIR)\europeanengine.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\finitedifferenceeuropean.sbr" \
	"$(INTDIR)\himalaya.sbr" \
	"$(INTDIR)\multiperiodoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\stepconditionoption.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\basket.sbr" \
	"$(INTDIR)\pagoda.sbr" \
	"$(INTDIR)\everest.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

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
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\plainoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\segmentintegral.obj" \
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
	"$(INTDIR)\singleassetpathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\europeanengine.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\basket.obj" \
	"$(INTDIR)\pagoda.obj" \
	"$(INTDIR)\everest.obj"

".\lib\Win32\VisualStudio\QuantLib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib" "$(OUTDIR)\QuantLib.bsc"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagepriceasian.sbr"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.sbr"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.sbr"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basket.obj"
	-@erase "$(INTDIR)\basket.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\bermudanoption.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendamericanoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendoption.sbr"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.sbr"
	-@erase "$(INTDIR)\europeanengine.obj"
	-@erase "$(INTDIR)\europeanengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everest.obj"
	-@erase "$(INTDIR)\everest.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalaya.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.sbr"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multiperiodoption.sbr"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.sbr"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newton.sbr"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newtonsafe.sbr"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\newyork.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagoda.obj"
	-@erase "$(INTDIR)\pagoda.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\plainoption.obj"
	-@erase "$(INTDIR)\plainoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\segmentintegral.obj"
	-@erase "$(INTDIR)\segmentintegral.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\singleassetpathpricer.obj"
	-@erase "$(INTDIR)\singleassetpathpricer.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stepconditionoption.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\westerncalendar.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\westerncalendar.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\plainoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\segmentintegral.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\avgpriceasianpathpricer.sbr" \
	"$(INTDIR)\avgstrikeasianpathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\controlvariatedpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricasianpathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\knuthrandomgenerator.sbr" \
	"$(INTDIR)\lecuyerrandomgenerator.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\singleassetpathpricer.sbr" \
	"$(INTDIR)\averagepriceasian.sbr" \
	"$(INTDIR)\averagestrikeasian.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\bermudanoption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\bsmnumericaloption.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendamericanoption.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\dividendoption.sbr" \
	"$(INTDIR)\dividendshoutoption.sbr" \
	"$(INTDIR)\europeanengine.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\finitedifferenceeuropean.sbr" \
	"$(INTDIR)\himalaya.sbr" \
	"$(INTDIR)\multiperiodoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\stepconditionoption.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\basket.sbr" \
	"$(INTDIR)\pagoda.sbr" \
	"$(INTDIR)\everest.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

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
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\plainoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\segmentintegral.obj" \
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
	"$(INTDIR)\singleassetpathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\europeanengine.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\basket.obj" \
	"$(INTDIR)\pagoda.obj" \
	"$(INTDIR)\everest.obj"

".\lib\Win32\VisualStudio\QuantLib_d.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

OUTDIR=.\build\OnTheEdgeRelease
INTDIR=.\build\OnTheEdgeRelease
# Begin Custom Macros
OutDir=.\build\OnTheEdgeRelease
# End Custom Macros

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib" "$(OUTDIR)\QuantLib.bsc"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagepriceasian.sbr"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.sbr"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.sbr"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basket.obj"
	-@erase "$(INTDIR)\basket.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\bermudanoption.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendamericanoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendoption.sbr"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.sbr"
	-@erase "$(INTDIR)\europeanengine.obj"
	-@erase "$(INTDIR)\europeanengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everest.obj"
	-@erase "$(INTDIR)\everest.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalaya.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.sbr"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multiperiodoption.sbr"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.sbr"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newton.sbr"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newtonsafe.sbr"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\newyork.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagoda.obj"
	-@erase "$(INTDIR)\pagoda.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\plainoption.obj"
	-@erase "$(INTDIR)\plainoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\segmentintegral.obj"
	-@erase "$(INTDIR)\segmentintegral.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\singleassetpathpricer.obj"
	-@erase "$(INTDIR)\singleassetpathpricer.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stepconditionoption.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\westerncalendar.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /Od /Ob2 /I ".\\" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\westerncalendar.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\plainoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\segmentintegral.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\avgpriceasianpathpricer.sbr" \
	"$(INTDIR)\avgstrikeasianpathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\controlvariatedpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricasianpathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\knuthrandomgenerator.sbr" \
	"$(INTDIR)\lecuyerrandomgenerator.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\singleassetpathpricer.sbr" \
	"$(INTDIR)\averagepriceasian.sbr" \
	"$(INTDIR)\averagestrikeasian.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\bermudanoption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\bsmnumericaloption.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendamericanoption.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\dividendoption.sbr" \
	"$(INTDIR)\dividendshoutoption.sbr" \
	"$(INTDIR)\europeanengine.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\finitedifferenceeuropean.sbr" \
	"$(INTDIR)\himalaya.sbr" \
	"$(INTDIR)\multiperiodoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\stepconditionoption.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\basket.sbr" \
	"$(INTDIR)\pagoda.sbr" \
	"$(INTDIR)\everest.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

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
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\plainoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\segmentintegral.obj" \
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
	"$(INTDIR)\singleassetpathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\europeanengine.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\basket.obj" \
	"$(INTDIR)\pagoda.obj" \
	"$(INTDIR)\everest.obj"

".\lib\Win32\VisualStudio\QuantLib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

OUTDIR=.\build\OnTheEdgeDebug
INTDIR=.\build\OnTheEdgeDebug
# Begin Custom Macros
OutDir=.\build\OnTheEdgeDebug
# End Custom Macros

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib" "$(OUTDIR)\QuantLib.bsc"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\averagepriceasian.obj"
	-@erase "$(INTDIR)\averagepriceasian.sbr"
	-@erase "$(INTDIR)\averagestrikeasian.obj"
	-@erase "$(INTDIR)\averagestrikeasian.sbr"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.obj"
	-@erase "$(INTDIR)\avgpriceasianpathpricer.sbr"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.obj"
	-@erase "$(INTDIR)\avgstrikeasianpathpricer.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basket.obj"
	-@erase "$(INTDIR)\basket.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\bermudanoption.obj"
	-@erase "$(INTDIR)\bermudanoption.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmnumericaloption.obj"
	-@erase "$(INTDIR)\bsmnumericaloption.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\controlvariatedpathpricer.obj"
	-@erase "$(INTDIR)\controlvariatedpathpricer.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\dividendamericanoption.obj"
	-@erase "$(INTDIR)\dividendamericanoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendoption.obj"
	-@erase "$(INTDIR)\dividendoption.sbr"
	-@erase "$(INTDIR)\dividendshoutoption.obj"
	-@erase "$(INTDIR)\dividendshoutoption.sbr"
	-@erase "$(INTDIR)\europeanengine.obj"
	-@erase "$(INTDIR)\europeanengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everest.obj"
	-@erase "$(INTDIR)\everest.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\finitedifferenceeuropean.obj"
	-@erase "$(INTDIR)\finitedifferenceeuropean.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\geometricasianpathpricer.obj"
	-@erase "$(INTDIR)\geometricasianpathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalaya.obj"
	-@erase "$(INTDIR)\himalaya.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\knuthrandomgenerator.obj"
	-@erase "$(INTDIR)\knuthrandomgenerator.sbr"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.obj"
	-@erase "$(INTDIR)\lecuyerrandomgenerator.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\multiperiodoption.obj"
	-@erase "$(INTDIR)\multiperiodoption.sbr"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.sbr"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newton.sbr"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newtonsafe.sbr"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\newyork.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagoda.obj"
	-@erase "$(INTDIR)\pagoda.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\plainoption.obj"
	-@erase "$(INTDIR)\plainoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\segmentintegral.obj"
	-@erase "$(INTDIR)\segmentintegral.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\singleassetpathpricer.obj"
	-@erase "$(INTDIR)\singleassetpathpricer.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\stepconditionoption.obj"
	-@erase "$(INTDIR)\stepconditionoption.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\westerncalendar.obj"
	-@erase "$(INTDIR)\westerncalendar.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\\" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\westerncalendar.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\plainoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\segmentintegral.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\avgpriceasianpathpricer.sbr" \
	"$(INTDIR)\avgstrikeasianpathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\controlvariatedpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricasianpathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\knuthrandomgenerator.sbr" \
	"$(INTDIR)\lecuyerrandomgenerator.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\singleassetpathpricer.sbr" \
	"$(INTDIR)\averagepriceasian.sbr" \
	"$(INTDIR)\averagestrikeasian.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\bermudanoption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\bsmnumericaloption.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendamericanoption.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\dividendoption.sbr" \
	"$(INTDIR)\dividendshoutoption.sbr" \
	"$(INTDIR)\europeanengine.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\finitedifferenceeuropean.sbr" \
	"$(INTDIR)\himalaya.sbr" \
	"$(INTDIR)\multiperiodoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\stepconditionoption.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\basket.sbr" \
	"$(INTDIR)\pagoda.sbr" \
	"$(INTDIR)\everest.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

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
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\plainoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\segmentintegral.obj" \
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
	"$(INTDIR)\singleassetpathpricer.obj" \
	"$(INTDIR)\averagepriceasian.obj" \
	"$(INTDIR)\averagestrikeasian.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\bermudanoption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\bsmnumericaloption.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendamericanoption.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\dividendoption.obj" \
	"$(INTDIR)\dividendshoutoption.obj" \
	"$(INTDIR)\europeanengine.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\finitedifferenceeuropean.obj" \
	"$(INTDIR)\himalaya.obj" \
	"$(INTDIR)\multiperiodoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\stepconditionoption.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\basket.obj" \
	"$(INTDIR)\pagoda.obj" \
	"$(INTDIR)\everest.obj"

".\lib\Win32\VisualStudio\QuantLib_d.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
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
!IF EXISTS("QuantLib.dep")
!INCLUDE "QuantLib.dep"
!ELSE 
!MESSAGE Warning: cannot find "QuantLib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QuantLib - Win32 Release" || "$(CFG)" == "QuantLib - Win32 Debug" || "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease" || "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"
SOURCE=.\ql\Calendars\frankfurt.cpp

"$(INTDIR)\frankfurt.obj"	"$(INTDIR)\frankfurt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\helsinki.cpp

"$(INTDIR)\helsinki.obj"	"$(INTDIR)\helsinki.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\london.cpp

"$(INTDIR)\london.obj"	"$(INTDIR)\london.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\milan.cpp

"$(INTDIR)\milan.obj"	"$(INTDIR)\milan.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\newyork.cpp

"$(INTDIR)\newyork.obj"	"$(INTDIR)\newyork.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\target.cpp

"$(INTDIR)\target.obj"	"$(INTDIR)\target.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\wellington.cpp

"$(INTDIR)\wellington.obj"	"$(INTDIR)\wellington.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\westerncalendar.cpp

"$(INTDIR)\westerncalendar.obj"	"$(INTDIR)\westerncalendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\zurich.cpp

"$(INTDIR)\zurich.obj"	"$(INTDIR)\zurich.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\DayCounters\actualactual.cpp

"$(INTDIR)\actualactual.obj"	"$(INTDIR)\actualactual.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\DayCounters\thirty360.cpp

"$(INTDIR)\thirty360.obj"	"$(INTDIR)\thirty360.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\bsmoperator.cpp

"$(INTDIR)\bsmoperator.obj"	"$(INTDIR)\bsmoperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\tridiagonaloperator.cpp

"$(INTDIR)\tridiagonaloperator.obj"	"$(INTDIR)\tridiagonaloperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\valueatcenter.cpp

"$(INTDIR)\valueatcenter.obj"	"$(INTDIR)\valueatcenter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\plainoption.cpp

"$(INTDIR)\plainoption.obj"	"$(INTDIR)\plainoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\simpleswap.cpp

"$(INTDIR)\simpleswap.obj"	"$(INTDIR)\simpleswap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\stock.cpp

"$(INTDIR)\stock.obj"	"$(INTDIR)\stock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\swap.cpp

"$(INTDIR)\swap.obj"	"$(INTDIR)\swap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\matrix.cpp

"$(INTDIR)\matrix.obj"	"$(INTDIR)\matrix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\multivariateaccumulator.cpp

"$(INTDIR)\multivariateaccumulator.obj"	"$(INTDIR)\multivariateaccumulator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\normaldistribution.cpp

"$(INTDIR)\normaldistribution.obj"	"$(INTDIR)\normaldistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\segmentintegral.cpp

"$(INTDIR)\segmentintegral.obj"	"$(INTDIR)\segmentintegral.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\statistics.cpp

"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\symmetricschurdecomposition.cpp

"$(INTDIR)\symmetricschurdecomposition.obj"	"$(INTDIR)\symmetricschurdecomposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\avgpriceasianpathpricer.cpp

"$(INTDIR)\avgpriceasianpathpricer.obj"	"$(INTDIR)\avgpriceasianpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\avgstrikeasianpathpricer.cpp

"$(INTDIR)\avgstrikeasianpathpricer.obj"	"$(INTDIR)\avgstrikeasianpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\basketpathpricer.cpp

"$(INTDIR)\basketpathpricer.obj"	"$(INTDIR)\basketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\controlvariatedpathpricer.cpp

"$(INTDIR)\controlvariatedpathpricer.obj"	"$(INTDIR)\controlvariatedpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\europeanpathpricer.cpp

"$(INTDIR)\europeanpathpricer.obj"	"$(INTDIR)\europeanpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\everestpathpricer.cpp

"$(INTDIR)\everestpathpricer.obj"	"$(INTDIR)\everestpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\geometricasianpathpricer.cpp

"$(INTDIR)\geometricasianpathpricer.obj"	"$(INTDIR)\geometricasianpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\getcovariance.cpp

"$(INTDIR)\getcovariance.obj"	"$(INTDIR)\getcovariance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\himalayapathpricer.cpp

"$(INTDIR)\himalayapathpricer.obj"	"$(INTDIR)\himalayapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\knuthrandomgenerator.cpp

"$(INTDIR)\knuthrandomgenerator.obj"	"$(INTDIR)\knuthrandomgenerator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\lecuyerrandomgenerator.cpp

"$(INTDIR)\lecuyerrandomgenerator.obj"	"$(INTDIR)\lecuyerrandomgenerator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\pagodapathpricer.cpp

"$(INTDIR)\pagodapathpricer.obj"	"$(INTDIR)\pagodapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\singleassetpathpricer.cpp

"$(INTDIR)\singleassetpathpricer.obj"	"$(INTDIR)\singleassetpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\averagepriceasian.cpp

"$(INTDIR)\averagepriceasian.obj"	"$(INTDIR)\averagepriceasian.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\averagestrikeasian.cpp

"$(INTDIR)\averagestrikeasian.obj"	"$(INTDIR)\averagestrikeasian.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\barrieroption.cpp

"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\basket.cpp

"$(INTDIR)\basket.obj"	"$(INTDIR)\basket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\bermudanoption.cpp

"$(INTDIR)\bermudanoption.obj"	"$(INTDIR)\bermudanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\binaryoption.cpp

"$(INTDIR)\binaryoption.obj"	"$(INTDIR)\binaryoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\bsmnumericaloption.cpp

"$(INTDIR)\bsmnumericaloption.obj"	"$(INTDIR)\bsmnumericaloption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\cliquetoption.cpp

"$(INTDIR)\cliquetoption.obj"	"$(INTDIR)\cliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\dividendamericanoption.cpp

"$(INTDIR)\dividendamericanoption.obj"	"$(INTDIR)\dividendamericanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\dividendeuropeanoption.cpp

"$(INTDIR)\dividendeuropeanoption.obj"	"$(INTDIR)\dividendeuropeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\dividendoption.cpp

"$(INTDIR)\dividendoption.obj"	"$(INTDIR)\dividendoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\dividendshoutoption.cpp

"$(INTDIR)\dividendshoutoption.obj"	"$(INTDIR)\dividendshoutoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\europeanengine.cpp

"$(INTDIR)\europeanengine.obj"	"$(INTDIR)\europeanengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\europeanoption.cpp

"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\everest.cpp

"$(INTDIR)\everest.obj"	"$(INTDIR)\everest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\finitedifferenceeuropean.cpp

"$(INTDIR)\finitedifferenceeuropean.obj"	"$(INTDIR)\finitedifferenceeuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\himalaya.cpp

"$(INTDIR)\himalaya.obj"	"$(INTDIR)\himalaya.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mceuropean.cpp

"$(INTDIR)\mceuropean.obj"	"$(INTDIR)\mceuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\multiperiodoption.cpp

"$(INTDIR)\multiperiodoption.obj"	"$(INTDIR)\multiperiodoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\pagoda.cpp

"$(INTDIR)\pagoda.obj"	"$(INTDIR)\pagoda.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\singleassetoption.cpp

"$(INTDIR)\singleassetoption.obj"	"$(INTDIR)\singleassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\stepconditionoption.cpp

"$(INTDIR)\stepconditionoption.obj"	"$(INTDIR)\stepconditionoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\bisection.cpp

"$(INTDIR)\bisection.obj"	"$(INTDIR)\bisection.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\brent.cpp

"$(INTDIR)\brent.obj"	"$(INTDIR)\brent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\falseposition.cpp

"$(INTDIR)\falseposition.obj"	"$(INTDIR)\falseposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\newton.cpp

"$(INTDIR)\newton.obj"	"$(INTDIR)\newton.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\newtonsafe.cpp

"$(INTDIR)\newtonsafe.obj"	"$(INTDIR)\newtonsafe.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\ridder.cpp

"$(INTDIR)\ridder.obj"	"$(INTDIR)\ridder.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Solvers1D\secant.cpp

"$(INTDIR)\secant.obj"	"$(INTDIR)\secant.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\piecewiseflatforward.cpp

"$(INTDIR)\piecewiseflatforward.obj"	"$(INTDIR)\piecewiseflatforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\ratehelpers.cpp

"$(INTDIR)\ratehelpers.obj"	"$(INTDIR)\ratehelpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Indexes\xibor.cpp

"$(INTDIR)\xibor.obj"	"$(INTDIR)\xibor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Indexes\xibormanager.cpp

"$(INTDIR)\xibormanager.obj"	"$(INTDIR)\xibormanager.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\CashFlows\cashflowvectors.cpp

"$(INTDIR)\cashflowvectors.obj"	"$(INTDIR)\cashflowvectors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\CashFlows\floatingratecoupon.cpp

"$(INTDIR)\floatingratecoupon.obj"	"$(INTDIR)\floatingratecoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\calendar.cpp

"$(INTDIR)\calendar.obj"	"$(INTDIR)\calendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\dataformatters.cpp

"$(INTDIR)\dataformatters.obj"	"$(INTDIR)\dataformatters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\date.cpp

"$(INTDIR)\date.obj"	"$(INTDIR)\date.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\option.cpp

"$(INTDIR)\option.obj"	"$(INTDIR)\option.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\scheduler.cpp

"$(INTDIR)\scheduler.obj"	"$(INTDIR)\scheduler.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\solver1d.cpp

"$(INTDIR)\solver1d.obj"	"$(INTDIR)\solver1d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

