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
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\arithmeticapopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticapopathpricer.sbr"
	-@erase "$(INTDIR)\arithmeticasopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticasopathpricer.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\calibrationhelper.obj"
	-@erase "$(INTDIR)\calibrationhelper.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\capfloorpricer.obj"
	-@erase "$(INTDIR)\capfloorpricer.sbr"
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\europeananalyticalengine.obj"
	-@erase "$(INTDIR)\europeananalyticalengine.sbr"
	-@erase "$(INTDIR)\europeanbinomialengine.obj"
	-@erase "$(INTDIR)\europeanbinomialengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
	-@erase "$(INTDIR)\fddividendeuropeanoption.obj"
	-@erase "$(INTDIR)\fddividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\fddividendoption.obj"
	-@erase "$(INTDIR)\fddividendoption.sbr"
	-@erase "$(INTDIR)\fddividendshoutoption.obj"
	-@erase "$(INTDIR)\fddividendshoutoption.sbr"
	-@erase "$(INTDIR)\fdeuropean.obj"
	-@erase "$(INTDIR)\fdeuropean.sbr"
	-@erase "$(INTDIR)\fdmultiperiodoption.obj"
	-@erase "$(INTDIR)\fdmultiperiodoption.sbr"
	-@erase "$(INTDIR)\fdstepconditionoption.obj"
	-@erase "$(INTDIR)\fdstepconditionoption.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\geometricapopathpricer.obj"
	-@erase "$(INTDIR)\geometricapopathpricer.sbr"
	-@erase "$(INTDIR)\geometricasopathpricer.obj"
	-@erase "$(INTDIR)\geometricasopathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\jamshidianswaption.obj"
	-@erase "$(INTDIR)\jamshidianswaption.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\mathf.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\maxbasketpathpricer.obj"
	-@erase "$(INTDIR)\maxbasketpathpricer.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\mceverest.obj"
	-@erase "$(INTDIR)\mceverest.sbr"
	-@erase "$(INTDIR)\mchimalaya.obj"
	-@erase "$(INTDIR)\mchimalaya.sbr"
	-@erase "$(INTDIR)\mcmaxbasket.obj"
	-@erase "$(INTDIR)\mcmaxbasket.sbr"
	-@erase "$(INTDIR)\mcpagoda.obj"
	-@erase "$(INTDIR)\mcpagoda.sbr"
	-@erase "$(INTDIR)\mcperformanceoption.obj"
	-@erase "$(INTDIR)\mcperformanceoption.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
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
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.obj"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\swaptionpricer.obj"
	-@erase "$(INTDIR)\swaptionpricer.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloor.obj"
	-@erase "$(INTDIR)\treecapfloor.sbr"
	-@erase "$(INTDIR)\treeswaption.obj"
	-@erase "$(INTDIR)\treeswaption.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "$(INTDIR)\vols.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
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

CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /Od /Ob2 /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\arithmeticapopathpricer.sbr" \
	"$(INTDIR)\arithmeticasopathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\cliquetoptionpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricapopathpricer.sbr" \
	"$(INTDIR)\geometricasopathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\maxbasketpathpricer.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\performanceoptionpathpricer.sbr" \
	"$(INTDIR)\analyticalcapfloor.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\blackcapfloor.sbr" \
	"$(INTDIR)\blackswaption.sbr" \
	"$(INTDIR)\capfloorpricer.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendeuropeanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\jamshidianswaption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\swaptionpricer.sbr" \
	"$(INTDIR)\treecapfloor.sbr" \
	"$(INTDIR)\treeswaption.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\mathf.sbr" \
	"$(INTDIR)\vols.sbr" \
	"$(INTDIR)\caphelper.sbr" \
	"$(INTDIR)\swaptionhelper.sbr" \
	"$(INTDIR)\blackkarasinski.sbr" \
	"$(INTDIR)\coxingersollross.sbr" \
	"$(INTDIR)\extendedcoxingersollross.sbr" \
	"$(INTDIR)\hullwhite.sbr" \
	"$(INTDIR)\vasicek.sbr" \
	"$(INTDIR)\g2.sbr" \
	"$(INTDIR)\calibrationhelper.sbr" \
	"$(INTDIR)\model.sbr" \
	"$(INTDIR)\onefactormodel.sbr" \
	"$(INTDIR)\twofactormodel.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\europeananalyticalengine.sbr" \
	"$(INTDIR)\europeanbinomialengine.sbr" \
	"$(INTDIR)\quantovanillaanalyticengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\Win32\VisualStudio\QuantLib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\arithmeticapopathpricer.obj" \
	"$(INTDIR)\arithmeticasopathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\cliquetoptionpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricapopathpricer.obj" \
	"$(INTDIR)\geometricasopathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\maxbasketpathpricer.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\performanceoptionpathpricer.obj" \
	"$(INTDIR)\analyticalcapfloor.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\blackcapfloor.obj" \
	"$(INTDIR)\blackswaption.obj" \
	"$(INTDIR)\capfloorpricer.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendeuropeanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\jamshidianswaption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\swaptionpricer.obj" \
	"$(INTDIR)\treecapfloor.obj" \
	"$(INTDIR)\treeswaption.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"$(INTDIR)\caphelper.obj" \
	"$(INTDIR)\swaptionhelper.obj" \
	"$(INTDIR)\blackkarasinski.obj" \
	"$(INTDIR)\coxingersollross.obj" \
	"$(INTDIR)\extendedcoxingersollross.obj" \
	"$(INTDIR)\hullwhite.obj" \
	"$(INTDIR)\vasicek.obj" \
	"$(INTDIR)\g2.obj" \
	"$(INTDIR)\calibrationhelper.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\onefactormodel.obj" \
	"$(INTDIR)\twofactormodel.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\europeananalyticalengine.obj" \
	"$(INTDIR)\europeanbinomialengine.obj" \
	"$(INTDIR)\quantovanillaanalyticengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

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
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\arithmeticapopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticapopathpricer.sbr"
	-@erase "$(INTDIR)\arithmeticasopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticasopathpricer.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\calibrationhelper.obj"
	-@erase "$(INTDIR)\calibrationhelper.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\capfloorpricer.obj"
	-@erase "$(INTDIR)\capfloorpricer.sbr"
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\europeananalyticalengine.obj"
	-@erase "$(INTDIR)\europeananalyticalengine.sbr"
	-@erase "$(INTDIR)\europeanbinomialengine.obj"
	-@erase "$(INTDIR)\europeanbinomialengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
	-@erase "$(INTDIR)\fddividendeuropeanoption.obj"
	-@erase "$(INTDIR)\fddividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\fddividendoption.obj"
	-@erase "$(INTDIR)\fddividendoption.sbr"
	-@erase "$(INTDIR)\fddividendshoutoption.obj"
	-@erase "$(INTDIR)\fddividendshoutoption.sbr"
	-@erase "$(INTDIR)\fdeuropean.obj"
	-@erase "$(INTDIR)\fdeuropean.sbr"
	-@erase "$(INTDIR)\fdmultiperiodoption.obj"
	-@erase "$(INTDIR)\fdmultiperiodoption.sbr"
	-@erase "$(INTDIR)\fdstepconditionoption.obj"
	-@erase "$(INTDIR)\fdstepconditionoption.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\geometricapopathpricer.obj"
	-@erase "$(INTDIR)\geometricapopathpricer.sbr"
	-@erase "$(INTDIR)\geometricasopathpricer.obj"
	-@erase "$(INTDIR)\geometricasopathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\jamshidianswaption.obj"
	-@erase "$(INTDIR)\jamshidianswaption.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\mathf.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\maxbasketpathpricer.obj"
	-@erase "$(INTDIR)\maxbasketpathpricer.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\mceverest.obj"
	-@erase "$(INTDIR)\mceverest.sbr"
	-@erase "$(INTDIR)\mchimalaya.obj"
	-@erase "$(INTDIR)\mchimalaya.sbr"
	-@erase "$(INTDIR)\mcmaxbasket.obj"
	-@erase "$(INTDIR)\mcmaxbasket.sbr"
	-@erase "$(INTDIR)\mcpagoda.obj"
	-@erase "$(INTDIR)\mcpagoda.sbr"
	-@erase "$(INTDIR)\mcperformanceoption.obj"
	-@erase "$(INTDIR)\mcperformanceoption.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
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
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.obj"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\swaptionpricer.obj"
	-@erase "$(INTDIR)\swaptionpricer.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloor.obj"
	-@erase "$(INTDIR)\treecapfloor.sbr"
	-@erase "$(INTDIR)\treeswaption.obj"
	-@erase "$(INTDIR)\treeswaption.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "$(INTDIR)\vols.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.idb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.pdb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"lib\Win32\VisualStudio\QuantLib" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\arithmeticapopathpricer.sbr" \
	"$(INTDIR)\arithmeticasopathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\cliquetoptionpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricapopathpricer.sbr" \
	"$(INTDIR)\geometricasopathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\maxbasketpathpricer.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\performanceoptionpathpricer.sbr" \
	"$(INTDIR)\analyticalcapfloor.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\blackcapfloor.sbr" \
	"$(INTDIR)\blackswaption.sbr" \
	"$(INTDIR)\capfloorpricer.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendeuropeanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\jamshidianswaption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\swaptionpricer.sbr" \
	"$(INTDIR)\treecapfloor.sbr" \
	"$(INTDIR)\treeswaption.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\mathf.sbr" \
	"$(INTDIR)\vols.sbr" \
	"$(INTDIR)\caphelper.sbr" \
	"$(INTDIR)\swaptionhelper.sbr" \
	"$(INTDIR)\blackkarasinski.sbr" \
	"$(INTDIR)\coxingersollross.sbr" \
	"$(INTDIR)\extendedcoxingersollross.sbr" \
	"$(INTDIR)\hullwhite.sbr" \
	"$(INTDIR)\vasicek.sbr" \
	"$(INTDIR)\g2.sbr" \
	"$(INTDIR)\calibrationhelper.sbr" \
	"$(INTDIR)\model.sbr" \
	"$(INTDIR)\onefactormodel.sbr" \
	"$(INTDIR)\twofactormodel.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\europeananalyticalengine.sbr" \
	"$(INTDIR)\europeanbinomialengine.sbr" \
	"$(INTDIR)\quantovanillaanalyticengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\arithmeticapopathpricer.obj" \
	"$(INTDIR)\arithmeticasopathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\cliquetoptionpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricapopathpricer.obj" \
	"$(INTDIR)\geometricasopathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\maxbasketpathpricer.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\performanceoptionpathpricer.obj" \
	"$(INTDIR)\analyticalcapfloor.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\blackcapfloor.obj" \
	"$(INTDIR)\blackswaption.obj" \
	"$(INTDIR)\capfloorpricer.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendeuropeanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\jamshidianswaption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\swaptionpricer.obj" \
	"$(INTDIR)\treecapfloor.obj" \
	"$(INTDIR)\treeswaption.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"$(INTDIR)\caphelper.obj" \
	"$(INTDIR)\swaptionhelper.obj" \
	"$(INTDIR)\blackkarasinski.obj" \
	"$(INTDIR)\coxingersollross.obj" \
	"$(INTDIR)\extendedcoxingersollross.obj" \
	"$(INTDIR)\hullwhite.obj" \
	"$(INTDIR)\vasicek.obj" \
	"$(INTDIR)\g2.obj" \
	"$(INTDIR)\calibrationhelper.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\onefactormodel.obj" \
	"$(INTDIR)\twofactormodel.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\europeananalyticalengine.obj" \
	"$(INTDIR)\europeanbinomialengine.obj" \
	"$(INTDIR)\quantovanillaanalyticengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

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
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\arithmeticapopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticapopathpricer.sbr"
	-@erase "$(INTDIR)\arithmeticasopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticasopathpricer.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\basketpathpricer.sbr"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binaryoption.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calendar.sbr"
	-@erase "$(INTDIR)\calibrationhelper.obj"
	-@erase "$(INTDIR)\calibrationhelper.sbr"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloor.sbr"
	-@erase "$(INTDIR)\capfloorpricer.obj"
	-@erase "$(INTDIR)\capfloorpricer.sbr"
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\daycounters.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\europeananalyticalengine.obj"
	-@erase "$(INTDIR)\europeananalyticalengine.sbr"
	-@erase "$(INTDIR)\europeanbinomialengine.obj"
	-@erase "$(INTDIR)\europeanbinomialengine.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\europeanpathpricer.sbr"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\falseposition.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
	-@erase "$(INTDIR)\fddividendeuropeanoption.obj"
	-@erase "$(INTDIR)\fddividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\fddividendoption.obj"
	-@erase "$(INTDIR)\fddividendoption.sbr"
	-@erase "$(INTDIR)\fddividendshoutoption.obj"
	-@erase "$(INTDIR)\fddividendshoutoption.sbr"
	-@erase "$(INTDIR)\fdeuropean.obj"
	-@erase "$(INTDIR)\fdeuropean.sbr"
	-@erase "$(INTDIR)\fdmultiperiodoption.obj"
	-@erase "$(INTDIR)\fdmultiperiodoption.sbr"
	-@erase "$(INTDIR)\fdstepconditionoption.obj"
	-@erase "$(INTDIR)\fdstepconditionoption.sbr"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\floatingratecoupon.sbr"
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\frankfurt.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\geometricapopathpricer.obj"
	-@erase "$(INTDIR)\geometricapopathpricer.sbr"
	-@erase "$(INTDIR)\geometricasopathpricer.obj"
	-@erase "$(INTDIR)\geometricasopathpricer.sbr"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\getcovariance.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\himalayapathpricer.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\jamshidianswaption.obj"
	-@erase "$(INTDIR)\jamshidianswaption.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\london.sbr"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\mathf.sbr"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\matrix.sbr"
	-@erase "$(INTDIR)\maxbasketpathpricer.obj"
	-@erase "$(INTDIR)\maxbasketpathpricer.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceuropean.sbr"
	-@erase "$(INTDIR)\mceverest.obj"
	-@erase "$(INTDIR)\mceverest.sbr"
	-@erase "$(INTDIR)\mchimalaya.obj"
	-@erase "$(INTDIR)\mchimalaya.sbr"
	-@erase "$(INTDIR)\mcmaxbasket.obj"
	-@erase "$(INTDIR)\mcmaxbasket.sbr"
	-@erase "$(INTDIR)\mcpagoda.obj"
	-@erase "$(INTDIR)\mcpagoda.sbr"
	-@erase "$(INTDIR)\mcperformanceoption.obj"
	-@erase "$(INTDIR)\mcperformanceoption.sbr"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\milan.sbr"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
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
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\option.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.obj"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\ridder.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\secant.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\solver1d.sbr"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\statistics.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\swaptionpricer.obj"
	-@erase "$(INTDIR)\swaptionpricer.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloor.obj"
	-@erase "$(INTDIR)\treecapfloor.sbr"
	-@erase "$(INTDIR)\treeswaption.obj"
	-@erase "$(INTDIR)\treeswaption.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "$(INTDIR)\vols.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
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

CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /Od /Ob2 /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\floatingratecoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\matrix.sbr" \
	"$(INTDIR)\multivariateaccumulator.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\statistics.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\arithmeticapopathpricer.sbr" \
	"$(INTDIR)\arithmeticasopathpricer.sbr" \
	"$(INTDIR)\basketpathpricer.sbr" \
	"$(INTDIR)\cliquetoptionpathpricer.sbr" \
	"$(INTDIR)\europeanpathpricer.sbr" \
	"$(INTDIR)\everestpathpricer.sbr" \
	"$(INTDIR)\geometricapopathpricer.sbr" \
	"$(INTDIR)\geometricasopathpricer.sbr" \
	"$(INTDIR)\getcovariance.sbr" \
	"$(INTDIR)\himalayapathpricer.sbr" \
	"$(INTDIR)\maxbasketpathpricer.sbr" \
	"$(INTDIR)\pagodapathpricer.sbr" \
	"$(INTDIR)\performanceoptionpathpricer.sbr" \
	"$(INTDIR)\analyticalcapfloor.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\binaryoption.sbr" \
	"$(INTDIR)\blackcapfloor.sbr" \
	"$(INTDIR)\blackswaption.sbr" \
	"$(INTDIR)\capfloorpricer.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendeuropeanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\jamshidianswaption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceuropean.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\swaptionpricer.sbr" \
	"$(INTDIR)\treecapfloor.sbr" \
	"$(INTDIR)\treeswaption.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\bisection.sbr" \
	"$(INTDIR)\brent.sbr" \
	"$(INTDIR)\falseposition.sbr" \
	"$(INTDIR)\newton.sbr" \
	"$(INTDIR)\newtonsafe.sbr" \
	"$(INTDIR)\ridder.sbr" \
	"$(INTDIR)\secant.sbr" \
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\daycounters.sbr" \
	"$(INTDIR)\mathf.sbr" \
	"$(INTDIR)\vols.sbr" \
	"$(INTDIR)\caphelper.sbr" \
	"$(INTDIR)\swaptionhelper.sbr" \
	"$(INTDIR)\blackkarasinski.sbr" \
	"$(INTDIR)\coxingersollross.sbr" \
	"$(INTDIR)\extendedcoxingersollross.sbr" \
	"$(INTDIR)\hullwhite.sbr" \
	"$(INTDIR)\vasicek.sbr" \
	"$(INTDIR)\g2.sbr" \
	"$(INTDIR)\calibrationhelper.sbr" \
	"$(INTDIR)\model.sbr" \
	"$(INTDIR)\onefactormodel.sbr" \
	"$(INTDIR)\twofactormodel.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\europeananalyticalengine.sbr" \
	"$(INTDIR)\europeanbinomialengine.sbr" \
	"$(INTDIR)\quantovanillaanalyticengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\arithmeticapopathpricer.obj" \
	"$(INTDIR)\arithmeticasopathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\cliquetoptionpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricapopathpricer.obj" \
	"$(INTDIR)\geometricasopathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\maxbasketpathpricer.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\performanceoptionpathpricer.obj" \
	"$(INTDIR)\analyticalcapfloor.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\blackcapfloor.obj" \
	"$(INTDIR)\blackswaption.obj" \
	"$(INTDIR)\capfloorpricer.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendeuropeanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\jamshidianswaption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\swaptionpricer.obj" \
	"$(INTDIR)\treecapfloor.obj" \
	"$(INTDIR)\treeswaption.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"$(INTDIR)\caphelper.obj" \
	"$(INTDIR)\swaptionhelper.obj" \
	"$(INTDIR)\blackkarasinski.obj" \
	"$(INTDIR)\coxingersollross.obj" \
	"$(INTDIR)\extendedcoxingersollross.obj" \
	"$(INTDIR)\hullwhite.obj" \
	"$(INTDIR)\vasicek.obj" \
	"$(INTDIR)\g2.obj" \
	"$(INTDIR)\calibrationhelper.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\onefactormodel.obj" \
	"$(INTDIR)\twofactormodel.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\europeananalyticalengine.obj" \
	"$(INTDIR)\europeanbinomialengine.obj" \
	"$(INTDIR)\quantovanillaanalyticengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

".\lib\Win32\VisualStudio\QuantLib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

OUTDIR=.\build\OnTheEdgeDebug
INTDIR=.\build\OnTheEdgeDebug

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib"


CLEAN :
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\arithmeticapopathpricer.obj"
	-@erase "$(INTDIR)\arithmeticasopathpricer.obj"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\basketpathpricer.obj"
	-@erase "$(INTDIR)\binaryoption.obj"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\calendar.obj"
	-@erase "$(INTDIR)\calibrationhelper.obj"
	-@erase "$(INTDIR)\capfloor.obj"
	-@erase "$(INTDIR)\capfloorpricer.obj"
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoptionpathpricer.obj"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\daycounters.obj"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\europeananalyticalengine.obj"
	-@erase "$(INTDIR)\europeanbinomialengine.obj"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanpathpricer.obj"
	-@erase "$(INTDIR)\everestpathpricer.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\falseposition.obj"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendeuropeanoption.obj"
	-@erase "$(INTDIR)\fddividendoption.obj"
	-@erase "$(INTDIR)\fddividendshoutoption.obj"
	-@erase "$(INTDIR)\fdeuropean.obj"
	-@erase "$(INTDIR)\fdmultiperiodoption.obj"
	-@erase "$(INTDIR)\fdstepconditionoption.obj"
	-@erase "$(INTDIR)\floatingratecoupon.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\frankfurt.obj"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\geometricapopathpricer.obj"
	-@erase "$(INTDIR)\geometricasopathpricer.obj"
	-@erase "$(INTDIR)\getcovariance.obj"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\himalayapathpricer.obj"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\jamshidianswaption.obj"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\london.obj"
	-@erase "$(INTDIR)\mathf.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\maxbasketpathpricer.obj"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mceuropean.obj"
	-@erase "$(INTDIR)\mceverest.obj"
	-@erase "$(INTDIR)\mchimalaya.obj"
	-@erase "$(INTDIR)\mcmaxbasket.obj"
	-@erase "$(INTDIR)\mcpagoda.obj"
	-@erase "$(INTDIR)\mcperformanceoption.obj"
	-@erase "$(INTDIR)\milan.obj"
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\multivariateaccumulator.obj"
	-@erase "$(INTDIR)\newton.obj"
	-@erase "$(INTDIR)\newtonsafe.obj"
	-@erase "$(INTDIR)\newyork.obj"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\option.obj"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\quantovanillaanalyticengine.obj"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ridder.obj"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\secant.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\solver1d.obj"
	-@erase "$(INTDIR)\statistics.obj"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionpricer.obj"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\treecapfloor.obj"
	-@erase "$(INTDIR)\treeswaption.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vols.obj"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.idb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.pdb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"lib\Win32\VisualStudio\QuantLib" /FD /GZ /c 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\QuantLib.bsc" 
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib" 
LIB32_OBJS= \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\floatingratecoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\multivariateaccumulator.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\statistics.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\arithmeticapopathpricer.obj" \
	"$(INTDIR)\arithmeticasopathpricer.obj" \
	"$(INTDIR)\basketpathpricer.obj" \
	"$(INTDIR)\cliquetoptionpathpricer.obj" \
	"$(INTDIR)\europeanpathpricer.obj" \
	"$(INTDIR)\everestpathpricer.obj" \
	"$(INTDIR)\geometricapopathpricer.obj" \
	"$(INTDIR)\geometricasopathpricer.obj" \
	"$(INTDIR)\getcovariance.obj" \
	"$(INTDIR)\himalayapathpricer.obj" \
	"$(INTDIR)\maxbasketpathpricer.obj" \
	"$(INTDIR)\pagodapathpricer.obj" \
	"$(INTDIR)\performanceoptionpathpricer.obj" \
	"$(INTDIR)\analyticalcapfloor.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\binaryoption.obj" \
	"$(INTDIR)\blackcapfloor.obj" \
	"$(INTDIR)\blackswaption.obj" \
	"$(INTDIR)\capfloorpricer.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendeuropeanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\jamshidianswaption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceuropean.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\swaptionpricer.obj" \
	"$(INTDIR)\treecapfloor.obj" \
	"$(INTDIR)\treeswaption.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\bisection.obj" \
	"$(INTDIR)\brent.obj" \
	"$(INTDIR)\falseposition.obj" \
	"$(INTDIR)\newton.obj" \
	"$(INTDIR)\newtonsafe.obj" \
	"$(INTDIR)\ridder.obj" \
	"$(INTDIR)\secant.obj" \
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\daycounters.obj" \
	"$(INTDIR)\mathf.obj" \
	"$(INTDIR)\vols.obj" \
	"$(INTDIR)\caphelper.obj" \
	"$(INTDIR)\swaptionhelper.obj" \
	"$(INTDIR)\blackkarasinski.obj" \
	"$(INTDIR)\coxingersollross.obj" \
	"$(INTDIR)\extendedcoxingersollross.obj" \
	"$(INTDIR)\hullwhite.obj" \
	"$(INTDIR)\vasicek.obj" \
	"$(INTDIR)\g2.obj" \
	"$(INTDIR)\calibrationhelper.obj" \
	"$(INTDIR)\model.obj" \
	"$(INTDIR)\onefactormodel.obj" \
	"$(INTDIR)\twofactormodel.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\europeananalyticalengine.obj" \
	"$(INTDIR)\europeanbinomialengine.obj" \
	"$(INTDIR)\quantovanillaanalyticengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj"

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

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\frankfurt.obj"	"$(INTDIR)\frankfurt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\frankfurt.obj"	"$(INTDIR)\frankfurt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\frankfurt.obj"	"$(INTDIR)\frankfurt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\frankfurt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\helsinki.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\helsinki.obj"	"$(INTDIR)\helsinki.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\helsinki.obj"	"$(INTDIR)\helsinki.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\helsinki.obj"	"$(INTDIR)\helsinki.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\helsinki.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\johannesburg.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\johannesburg.obj"	"$(INTDIR)\johannesburg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\johannesburg.obj"	"$(INTDIR)\johannesburg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\johannesburg.obj"	"$(INTDIR)\johannesburg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\johannesburg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\london.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\london.obj"	"$(INTDIR)\london.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\london.obj"	"$(INTDIR)\london.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\london.obj"	"$(INTDIR)\london.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\london.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\milan.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\milan.obj"	"$(INTDIR)\milan.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\milan.obj"	"$(INTDIR)\milan.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\milan.obj"	"$(INTDIR)\milan.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\milan.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\newyork.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\newyork.obj"	"$(INTDIR)\newyork.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\newyork.obj"	"$(INTDIR)\newyork.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\newyork.obj"	"$(INTDIR)\newyork.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\newyork.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\sydney.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\sydney.obj"	"$(INTDIR)\sydney.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\sydney.obj"	"$(INTDIR)\sydney.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\sydney.obj"	"$(INTDIR)\sydney.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\sydney.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\target.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\target.obj"	"$(INTDIR)\target.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\target.obj"	"$(INTDIR)\target.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\target.obj"	"$(INTDIR)\target.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\target.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\tokyo.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\tokyo.obj"	"$(INTDIR)\tokyo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\tokyo.obj"	"$(INTDIR)\tokyo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\tokyo.obj"	"$(INTDIR)\tokyo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\tokyo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\toronto.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\toronto.obj"	"$(INTDIR)\toronto.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\toronto.obj"	"$(INTDIR)\toronto.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\toronto.obj"	"$(INTDIR)\toronto.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\toronto.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\wellington.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\wellington.obj"	"$(INTDIR)\wellington.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\wellington.obj"	"$(INTDIR)\wellington.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\wellington.obj"	"$(INTDIR)\wellington.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\wellington.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Calendars\zurich.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\zurich.obj"	"$(INTDIR)\zurich.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\zurich.obj"	"$(INTDIR)\zurich.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\zurich.obj"	"$(INTDIR)\zurich.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\zurich.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\CashFlows\cashflowvectors.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\cashflowvectors.obj"	"$(INTDIR)\cashflowvectors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\cashflowvectors.obj"	"$(INTDIR)\cashflowvectors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\cashflowvectors.obj"	"$(INTDIR)\cashflowvectors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\cashflowvectors.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\CashFlows\floatingratecoupon.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\floatingratecoupon.obj"	"$(INTDIR)\floatingratecoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\floatingratecoupon.obj"	"$(INTDIR)\floatingratecoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\floatingratecoupon.obj"	"$(INTDIR)\floatingratecoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\floatingratecoupon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\CashFlows\shortfloatingcoupon.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\shortfloatingcoupon.obj"	"$(INTDIR)\shortfloatingcoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\shortfloatingcoupon.obj"	"$(INTDIR)\shortfloatingcoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\shortfloatingcoupon.obj"	"$(INTDIR)\shortfloatingcoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\shortfloatingcoupon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\DayCounters\actualactual.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\actualactual.obj"	"$(INTDIR)\actualactual.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\actualactual.obj"	"$(INTDIR)\actualactual.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\actualactual.obj"	"$(INTDIR)\actualactual.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\actualactual.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\DayCounters\thirty360.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\thirty360.obj"	"$(INTDIR)\thirty360.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\thirty360.obj"	"$(INTDIR)\thirty360.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\thirty360.obj"	"$(INTDIR)\thirty360.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\thirty360.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\FiniteDifferences\bsmoperator.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\bsmoperator.obj"	"$(INTDIR)\bsmoperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\bsmoperator.obj"	"$(INTDIR)\bsmoperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\bsmoperator.obj"	"$(INTDIR)\bsmoperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\bsmoperator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\FiniteDifferences\onefactoroperator.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\onefactoroperator.obj"	"$(INTDIR)\onefactoroperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\onefactoroperator.obj"	"$(INTDIR)\onefactoroperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\onefactoroperator.obj"	"$(INTDIR)\onefactoroperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\onefactoroperator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\FiniteDifferences\tridiagonaloperator.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\tridiagonaloperator.obj"	"$(INTDIR)\tridiagonaloperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\tridiagonaloperator.obj"	"$(INTDIR)\tridiagonaloperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\tridiagonaloperator.obj"	"$(INTDIR)\tridiagonaloperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\tridiagonaloperator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\FiniteDifferences\valueatcenter.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\valueatcenter.obj"	"$(INTDIR)\valueatcenter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\valueatcenter.obj"	"$(INTDIR)\valueatcenter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\valueatcenter.obj"	"$(INTDIR)\valueatcenter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\valueatcenter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Indexes\xibor.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\xibor.obj"	"$(INTDIR)\xibor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\xibor.obj"	"$(INTDIR)\xibor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\xibor.obj"	"$(INTDIR)\xibor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\xibor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Indexes\xibormanager.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\xibormanager.obj"	"$(INTDIR)\xibormanager.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\xibormanager.obj"	"$(INTDIR)\xibormanager.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\xibormanager.obj"	"$(INTDIR)\xibormanager.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\xibormanager.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\capfloor.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\capfloor.obj"	"$(INTDIR)\capfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\capfloor.obj"	"$(INTDIR)\capfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\capfloor.obj"	"$(INTDIR)\capfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\capfloor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\forwardvanillaoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\forwardvanillaoption.obj"	"$(INTDIR)\forwardvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\forwardvanillaoption.obj"	"$(INTDIR)\forwardvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\forwardvanillaoption.obj"	"$(INTDIR)\forwardvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\forwardvanillaoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\quantovanillaoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\quantovanillaoption.obj"	"$(INTDIR)\quantovanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\quantovanillaoption.obj"	"$(INTDIR)\quantovanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\quantovanillaoption.obj"	"$(INTDIR)\quantovanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\quantovanillaoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\simpleswap.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\simpleswap.obj"	"$(INTDIR)\simpleswap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\simpleswap.obj"	"$(INTDIR)\simpleswap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\simpleswap.obj"	"$(INTDIR)\simpleswap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\simpleswap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\stock.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\stock.obj"	"$(INTDIR)\stock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\stock.obj"	"$(INTDIR)\stock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\stock.obj"	"$(INTDIR)\stock.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\stock.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\swap.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\swap.obj"	"$(INTDIR)\swap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\swap.obj"	"$(INTDIR)\swap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\swap.obj"	"$(INTDIR)\swap.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\swap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\swaption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\swaption.obj"	"$(INTDIR)\swaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\swaption.obj"	"$(INTDIR)\swaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\swaption.obj"	"$(INTDIR)\swaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\swaption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Instruments\vanillaoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\vanillaoption.obj"	"$(INTDIR)\vanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\vanillaoption.obj"	"$(INTDIR)\vanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\vanillaoption.obj"	"$(INTDIR)\vanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\vanillaoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\chisquaredistribution.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\chisquaredistribution.obj"	"$(INTDIR)\chisquaredistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\chisquaredistribution.obj"	"$(INTDIR)\chisquaredistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\chisquaredistribution.obj"	"$(INTDIR)\chisquaredistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\chisquaredistribution.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\gammadistribution.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\gammadistribution.obj"	"$(INTDIR)\gammadistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\gammadistribution.obj"	"$(INTDIR)\gammadistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\gammadistribution.obj"	"$(INTDIR)\gammadistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\gammadistribution.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\matrix.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\matrix.obj"	"$(INTDIR)\matrix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\matrix.obj"	"$(INTDIR)\matrix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\matrix.obj"	"$(INTDIR)\matrix.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\matrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\multivariateaccumulator.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\multivariateaccumulator.obj"	"$(INTDIR)\multivariateaccumulator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\multivariateaccumulator.obj"	"$(INTDIR)\multivariateaccumulator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\multivariateaccumulator.obj"	"$(INTDIR)\multivariateaccumulator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\multivariateaccumulator.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\normaldistribution.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\normaldistribution.obj"	"$(INTDIR)\normaldistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\normaldistribution.obj"	"$(INTDIR)\normaldistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\normaldistribution.obj"	"$(INTDIR)\normaldistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\normaldistribution.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\statistics.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\statistics.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Math\symmetricschurdecomposition.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\symmetricschurdecomposition.obj"	"$(INTDIR)\symmetricschurdecomposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\symmetricschurdecomposition.obj"	"$(INTDIR)\symmetricschurdecomposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\symmetricschurdecomposition.obj"	"$(INTDIR)\symmetricschurdecomposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\symmetricschurdecomposition.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\arithmeticapopathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\arithmeticapopathpricer.obj"	"$(INTDIR)\arithmeticapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\arithmeticapopathpricer.obj"	"$(INTDIR)\arithmeticapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\arithmeticapopathpricer.obj"	"$(INTDIR)\arithmeticapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\arithmeticapopathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\arithmeticasopathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\arithmeticasopathpricer.obj"	"$(INTDIR)\arithmeticasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\arithmeticasopathpricer.obj"	"$(INTDIR)\arithmeticasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\arithmeticasopathpricer.obj"	"$(INTDIR)\arithmeticasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\arithmeticasopathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\basketpathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\basketpathpricer.obj"	"$(INTDIR)\basketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\basketpathpricer.obj"	"$(INTDIR)\basketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\basketpathpricer.obj"	"$(INTDIR)\basketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\basketpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\cliquetoptionpathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\cliquetoptionpathpricer.obj"	"$(INTDIR)\cliquetoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\cliquetoptionpathpricer.obj"	"$(INTDIR)\cliquetoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\cliquetoptionpathpricer.obj"	"$(INTDIR)\cliquetoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\cliquetoptionpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\europeanpathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\europeanpathpricer.obj"	"$(INTDIR)\europeanpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\europeanpathpricer.obj"	"$(INTDIR)\europeanpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\europeanpathpricer.obj"	"$(INTDIR)\europeanpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\europeanpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\everestpathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\everestpathpricer.obj"	"$(INTDIR)\everestpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\everestpathpricer.obj"	"$(INTDIR)\everestpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\everestpathpricer.obj"	"$(INTDIR)\everestpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\everestpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\geometricapopathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\geometricapopathpricer.obj"	"$(INTDIR)\geometricapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\geometricapopathpricer.obj"	"$(INTDIR)\geometricapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\geometricapopathpricer.obj"	"$(INTDIR)\geometricapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\geometricapopathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\geometricasopathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\geometricasopathpricer.obj"	"$(INTDIR)\geometricasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\geometricasopathpricer.obj"	"$(INTDIR)\geometricasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\geometricasopathpricer.obj"	"$(INTDIR)\geometricasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\geometricasopathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\getcovariance.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\getcovariance.obj"	"$(INTDIR)\getcovariance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\getcovariance.obj"	"$(INTDIR)\getcovariance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\getcovariance.obj"	"$(INTDIR)\getcovariance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\getcovariance.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\himalayapathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\himalayapathpricer.obj"	"$(INTDIR)\himalayapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\himalayapathpricer.obj"	"$(INTDIR)\himalayapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\himalayapathpricer.obj"	"$(INTDIR)\himalayapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\himalayapathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\maxbasketpathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\maxbasketpathpricer.obj"	"$(INTDIR)\maxbasketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\maxbasketpathpricer.obj"	"$(INTDIR)\maxbasketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\maxbasketpathpricer.obj"	"$(INTDIR)\maxbasketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\maxbasketpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\pagodapathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\pagodapathpricer.obj"	"$(INTDIR)\pagodapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\pagodapathpricer.obj"	"$(INTDIR)\pagodapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\pagodapathpricer.obj"	"$(INTDIR)\pagodapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\pagodapathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\MonteCarlo\performanceoptionpathpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\performanceoptionpathpricer.obj"	"$(INTDIR)\performanceoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\performanceoptionpathpricer.obj"	"$(INTDIR)\performanceoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\performanceoptionpathpricer.obj"	"$(INTDIR)\performanceoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\performanceoptionpathpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\analyticalcapfloor.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\analyticalcapfloor.obj"	"$(INTDIR)\analyticalcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\analyticalcapfloor.obj"	"$(INTDIR)\analyticalcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\analyticalcapfloor.obj"	"$(INTDIR)\analyticalcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\analyticalcapfloor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\barrieroption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\barrieroption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\binaryoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\binaryoption.obj"	"$(INTDIR)\binaryoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\binaryoption.obj"	"$(INTDIR)\binaryoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\binaryoption.obj"	"$(INTDIR)\binaryoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\binaryoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\blackcapfloor.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\blackcapfloor.obj"	"$(INTDIR)\blackcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\blackcapfloor.obj"	"$(INTDIR)\blackcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\blackcapfloor.obj"	"$(INTDIR)\blackcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\blackcapfloor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\blackswaption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\blackswaption.obj"	"$(INTDIR)\blackswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\blackswaption.obj"	"$(INTDIR)\blackswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\blackswaption.obj"	"$(INTDIR)\blackswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\blackswaption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\capfloorpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\capfloorpricer.obj"	"$(INTDIR)\capfloorpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\capfloorpricer.obj"	"$(INTDIR)\capfloorpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\capfloorpricer.obj"	"$(INTDIR)\capfloorpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\capfloorpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\cliquetoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\cliquetoption.obj"	"$(INTDIR)\cliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\cliquetoption.obj"	"$(INTDIR)\cliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\cliquetoption.obj"	"$(INTDIR)\cliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\cliquetoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\discretegeometricapo.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\discretegeometricapo.obj"	"$(INTDIR)\discretegeometricapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\discretegeometricapo.obj"	"$(INTDIR)\discretegeometricapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\discretegeometricapo.obj"	"$(INTDIR)\discretegeometricapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\discretegeometricapo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\discretegeometricaso.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\discretegeometricaso.obj"	"$(INTDIR)\discretegeometricaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\discretegeometricaso.obj"	"$(INTDIR)\discretegeometricaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\discretegeometricaso.obj"	"$(INTDIR)\discretegeometricaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\discretegeometricaso.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\europeanoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\europeanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fdbermudanoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fdbermudanoption.obj"	"$(INTDIR)\fdbermudanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fdbermudanoption.obj"	"$(INTDIR)\fdbermudanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fdbermudanoption.obj"	"$(INTDIR)\fdbermudanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fdbermudanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fdbsmoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fdbsmoption.obj"	"$(INTDIR)\fdbsmoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fdbsmoption.obj"	"$(INTDIR)\fdbsmoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fdbsmoption.obj"	"$(INTDIR)\fdbsmoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fdbsmoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fddividendamericanoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fddividendamericanoption.obj"	"$(INTDIR)\fddividendamericanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fddividendamericanoption.obj"	"$(INTDIR)\fddividendamericanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fddividendamericanoption.obj"	"$(INTDIR)\fddividendamericanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fddividendamericanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fddividendeuropeanoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fddividendeuropeanoption.obj"	"$(INTDIR)\fddividendeuropeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fddividendeuropeanoption.obj"	"$(INTDIR)\fddividendeuropeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fddividendeuropeanoption.obj"	"$(INTDIR)\fddividendeuropeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fddividendeuropeanoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fddividendoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fddividendoption.obj"	"$(INTDIR)\fddividendoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fddividendoption.obj"	"$(INTDIR)\fddividendoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fddividendoption.obj"	"$(INTDIR)\fddividendoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fddividendoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fddividendshoutoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fddividendshoutoption.obj"	"$(INTDIR)\fddividendshoutoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fddividendshoutoption.obj"	"$(INTDIR)\fddividendshoutoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fddividendshoutoption.obj"	"$(INTDIR)\fddividendshoutoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fddividendshoutoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fdeuropean.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fdeuropean.obj"	"$(INTDIR)\fdeuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fdeuropean.obj"	"$(INTDIR)\fdeuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fdeuropean.obj"	"$(INTDIR)\fdeuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fdeuropean.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fdmultiperiodoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fdmultiperiodoption.obj"	"$(INTDIR)\fdmultiperiodoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fdmultiperiodoption.obj"	"$(INTDIR)\fdmultiperiodoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fdmultiperiodoption.obj"	"$(INTDIR)\fdmultiperiodoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fdmultiperiodoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\fdstepconditionoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\fdstepconditionoption.obj"	"$(INTDIR)\fdstepconditionoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\fdstepconditionoption.obj"	"$(INTDIR)\fdstepconditionoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\fdstepconditionoption.obj"	"$(INTDIR)\fdstepconditionoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\fdstepconditionoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\jamshidianswaption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\jamshidianswaption.obj"	"$(INTDIR)\jamshidianswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\jamshidianswaption.obj"	"$(INTDIR)\jamshidianswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\jamshidianswaption.obj"	"$(INTDIR)\jamshidianswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\jamshidianswaption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mcbasket.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mcbasket.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mccliquetoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mccliquetoption.obj"	"$(INTDIR)\mccliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mccliquetoption.obj"	"$(INTDIR)\mccliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mccliquetoption.obj"	"$(INTDIR)\mccliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mccliquetoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mcdiscretearithmeticapo.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mcdiscretearithmeticapo.obj"	"$(INTDIR)\mcdiscretearithmeticapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mcdiscretearithmeticapo.obj"	"$(INTDIR)\mcdiscretearithmeticapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mcdiscretearithmeticapo.obj"	"$(INTDIR)\mcdiscretearithmeticapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mcdiscretearithmeticapo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mcdiscretearithmeticaso.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mcdiscretearithmeticaso.obj"	"$(INTDIR)\mcdiscretearithmeticaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mcdiscretearithmeticaso.obj"	"$(INTDIR)\mcdiscretearithmeticaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mcdiscretearithmeticaso.obj"	"$(INTDIR)\mcdiscretearithmeticaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mcdiscretearithmeticaso.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mceuropean.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mceuropean.obj"	"$(INTDIR)\mceuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mceuropean.obj"	"$(INTDIR)\mceuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mceuropean.obj"	"$(INTDIR)\mceuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mceuropean.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mceverest.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mceverest.obj"	"$(INTDIR)\mceverest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mceverest.obj"	"$(INTDIR)\mceverest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mceverest.obj"	"$(INTDIR)\mceverest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mceverest.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mchimalaya.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mchimalaya.obj"	"$(INTDIR)\mchimalaya.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mchimalaya.obj"	"$(INTDIR)\mchimalaya.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mchimalaya.obj"	"$(INTDIR)\mchimalaya.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mchimalaya.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mcmaxbasket.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mcmaxbasket.obj"	"$(INTDIR)\mcmaxbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mcmaxbasket.obj"	"$(INTDIR)\mcmaxbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mcmaxbasket.obj"	"$(INTDIR)\mcmaxbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mcmaxbasket.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mcpagoda.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mcpagoda.obj"	"$(INTDIR)\mcpagoda.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mcpagoda.obj"	"$(INTDIR)\mcpagoda.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mcpagoda.obj"	"$(INTDIR)\mcpagoda.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mcpagoda.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\mcperformanceoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mcperformanceoption.obj"	"$(INTDIR)\mcperformanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mcperformanceoption.obj"	"$(INTDIR)\mcperformanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mcperformanceoption.obj"	"$(INTDIR)\mcperformanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mcperformanceoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\performanceoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\performanceoption.obj"	"$(INTDIR)\performanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\performanceoption.obj"	"$(INTDIR)\performanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\performanceoption.obj"	"$(INTDIR)\performanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\performanceoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\singleassetoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\singleassetoption.obj"	"$(INTDIR)\singleassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\singleassetoption.obj"	"$(INTDIR)\singleassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\singleassetoption.obj"	"$(INTDIR)\singleassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\singleassetoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\swaptionpricer.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\swaptionpricer.obj"	"$(INTDIR)\swaptionpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\swaptionpricer.obj"	"$(INTDIR)\swaptionpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\swaptionpricer.obj"	"$(INTDIR)\swaptionpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\swaptionpricer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\treecapfloor.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\treecapfloor.obj"	"$(INTDIR)\treecapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\treecapfloor.obj"	"$(INTDIR)\treecapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\treecapfloor.obj"	"$(INTDIR)\treecapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\treecapfloor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Pricers\treeswaption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\treeswaption.obj"	"$(INTDIR)\treeswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\treeswaption.obj"	"$(INTDIR)\treeswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\treeswaption.obj"	"$(INTDIR)\treeswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\treeswaption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\RandomNumbers\knuthuniformrng.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\knuthuniformrng.obj"	"$(INTDIR)\knuthuniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\knuthuniformrng.obj"	"$(INTDIR)\knuthuniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\knuthuniformrng.obj"	"$(INTDIR)\knuthuniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\knuthuniformrng.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\RandomNumbers\lecuyeruniformrng.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\lecuyeruniformrng.obj"	"$(INTDIR)\lecuyeruniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\lecuyeruniformrng.obj"	"$(INTDIR)\lecuyeruniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\lecuyeruniformrng.obj"	"$(INTDIR)\lecuyeruniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\lecuyeruniformrng.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\bisection.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\bisection.obj"	"$(INTDIR)\bisection.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\bisection.obj"	"$(INTDIR)\bisection.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\bisection.obj"	"$(INTDIR)\bisection.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\bisection.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\brent.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\brent.obj"	"$(INTDIR)\brent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\brent.obj"	"$(INTDIR)\brent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\brent.obj"	"$(INTDIR)\brent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\brent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\falseposition.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\falseposition.obj"	"$(INTDIR)\falseposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\falseposition.obj"	"$(INTDIR)\falseposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\falseposition.obj"	"$(INTDIR)\falseposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\falseposition.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\newton.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\newton.obj"	"$(INTDIR)\newton.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\newton.obj"	"$(INTDIR)\newton.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\newton.obj"	"$(INTDIR)\newton.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\newton.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\newtonsafe.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\newtonsafe.obj"	"$(INTDIR)\newtonsafe.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\newtonsafe.obj"	"$(INTDIR)\newtonsafe.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\newtonsafe.obj"	"$(INTDIR)\newtonsafe.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\newtonsafe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\ridder.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\ridder.obj"	"$(INTDIR)\ridder.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\ridder.obj"	"$(INTDIR)\ridder.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\ridder.obj"	"$(INTDIR)\ridder.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\ridder.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Solvers1D\secant.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\secant.obj"	"$(INTDIR)\secant.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\secant.obj"	"$(INTDIR)\secant.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\secant.obj"	"$(INTDIR)\secant.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\secant.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\TermStructures\affinetermstructure.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\affinetermstructure.obj"	"$(INTDIR)\affinetermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\affinetermstructure.obj"	"$(INTDIR)\affinetermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\affinetermstructure.obj"	"$(INTDIR)\affinetermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\affinetermstructure.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\TermStructures\compoundforward.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\compoundforward.obj"	"$(INTDIR)\compoundforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\compoundforward.obj"	"$(INTDIR)\compoundforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\compoundforward.obj"	"$(INTDIR)\compoundforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\compoundforward.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\TermStructures\discountcurve.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\discountcurve.obj"	"$(INTDIR)\discountcurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\discountcurve.obj"	"$(INTDIR)\discountcurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\discountcurve.obj"	"$(INTDIR)\discountcurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\discountcurve.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\TermStructures\piecewiseflatforward.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\piecewiseflatforward.obj"	"$(INTDIR)\piecewiseflatforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\piecewiseflatforward.obj"	"$(INTDIR)\piecewiseflatforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\piecewiseflatforward.obj"	"$(INTDIR)\piecewiseflatforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\piecewiseflatforward.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\TermStructures\ratehelpers.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\ratehelpers.obj"	"$(INTDIR)\ratehelpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\ratehelpers.obj"	"$(INTDIR)\ratehelpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\ratehelpers.obj"	"$(INTDIR)\ratehelpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\ratehelpers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Optimization\armijo.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\armijo.obj"	"$(INTDIR)\armijo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\armijo.obj"	"$(INTDIR)\armijo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\armijo.obj"	"$(INTDIR)\armijo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\armijo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Optimization\conjugategradient.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\conjugategradient.obj"	"$(INTDIR)\conjugategradient.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\conjugategradient.obj"	"$(INTDIR)\conjugategradient.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\conjugategradient.obj"	"$(INTDIR)\conjugategradient.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\conjugategradient.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Optimization\simplex.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\simplex.obj"	"$(INTDIR)\simplex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\simplex.obj"	"$(INTDIR)\simplex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\simplex.obj"	"$(INTDIR)\simplex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\simplex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Optimization\steepestdescent.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\steepestdescent.obj"	"$(INTDIR)\steepestdescent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\steepestdescent.obj"	"$(INTDIR)\steepestdescent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\steepestdescent.obj"	"$(INTDIR)\steepestdescent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\steepestdescent.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Lattices\binomialtree.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\binomialtree.obj"	"$(INTDIR)\binomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\binomialtree.obj"	"$(INTDIR)\binomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\binomialtree.obj"	"$(INTDIR)\binomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\binomialtree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Lattices\bsmlattice.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\bsmlattice.obj"	"$(INTDIR)\bsmlattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\bsmlattice.obj"	"$(INTDIR)\bsmlattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\bsmlattice.obj"	"$(INTDIR)\bsmlattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\bsmlattice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Lattices\lattice.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\lattice.obj"	"$(INTDIR)\lattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\lattice.obj"	"$(INTDIR)\lattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\lattice.obj"	"$(INTDIR)\lattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\lattice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Lattices\lattice2d.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\lattice2d.obj"	"$(INTDIR)\lattice2d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\lattice2d.obj"	"$(INTDIR)\lattice2d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\lattice2d.obj"	"$(INTDIR)\lattice2d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\lattice2d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\Lattices\trinomialtree.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\trinomialtree.obj"	"$(INTDIR)\trinomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\trinomialtree.obj"	"$(INTDIR)\trinomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\trinomialtree.obj"	"$(INTDIR)\trinomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\trinomialtree.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\functions\daycounters.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\daycounters.obj"	"$(INTDIR)\daycounters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\daycounters.obj"	"$(INTDIR)\daycounters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\daycounters.obj"	"$(INTDIR)\daycounters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\daycounters.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\functions\mathf.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\mathf.obj"	"$(INTDIR)\mathf.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\mathf.obj"	"$(INTDIR)\mathf.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\mathf.obj"	"$(INTDIR)\mathf.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\mathf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\functions\vols.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\vols.obj"	"$(INTDIR)\vols.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\vols.obj"	"$(INTDIR)\vols.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\vols.obj"	"$(INTDIR)\vols.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\vols.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\CalibrationHelpers\caphelper.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\caphelper.obj"	"$(INTDIR)\caphelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\caphelper.obj"	"$(INTDIR)\caphelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\caphelper.obj"	"$(INTDIR)\caphelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\caphelper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\CalibrationHelpers\swaptionhelper.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\swaptionhelper.obj"	"$(INTDIR)\swaptionhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\swaptionhelper.obj"	"$(INTDIR)\swaptionhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\swaptionhelper.obj"	"$(INTDIR)\swaptionhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\swaptionhelper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\OneFactorModels\blackkarasinski.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\blackkarasinski.obj"	"$(INTDIR)\blackkarasinski.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\blackkarasinski.obj"	"$(INTDIR)\blackkarasinski.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\blackkarasinski.obj"	"$(INTDIR)\blackkarasinski.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\blackkarasinski.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\OneFactorModels\coxingersollross.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\coxingersollross.obj"	"$(INTDIR)\coxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\coxingersollross.obj"	"$(INTDIR)\coxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\coxingersollross.obj"	"$(INTDIR)\coxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\coxingersollross.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\OneFactorModels\extendedcoxingersollross.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\extendedcoxingersollross.obj"	"$(INTDIR)\extendedcoxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\extendedcoxingersollross.obj"	"$(INTDIR)\extendedcoxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\extendedcoxingersollross.obj"	"$(INTDIR)\extendedcoxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\extendedcoxingersollross.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\OneFactorModels\hullwhite.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\hullwhite.obj"	"$(INTDIR)\hullwhite.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\hullwhite.obj"	"$(INTDIR)\hullwhite.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\hullwhite.obj"	"$(INTDIR)\hullwhite.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\hullwhite.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\OneFactorModels\vasicek.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\vasicek.obj"	"$(INTDIR)\vasicek.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\vasicek.obj"	"$(INTDIR)\vasicek.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\vasicek.obj"	"$(INTDIR)\vasicek.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\vasicek.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\TwoFactorModels\g2.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\g2.obj"	"$(INTDIR)\g2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\g2.obj"	"$(INTDIR)\g2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\g2.obj"	"$(INTDIR)\g2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\g2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\calibrationhelper.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\calibrationhelper.obj"	"$(INTDIR)\calibrationhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\calibrationhelper.obj"	"$(INTDIR)\calibrationhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\calibrationhelper.obj"	"$(INTDIR)\calibrationhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\calibrationhelper.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\model.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\model.obj"	"$(INTDIR)\model.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\model.obj"	"$(INTDIR)\model.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\model.obj"	"$(INTDIR)\model.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\onefactormodel.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\onefactormodel.obj"	"$(INTDIR)\onefactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\onefactormodel.obj"	"$(INTDIR)\onefactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\onefactormodel.obj"	"$(INTDIR)\onefactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\onefactormodel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\ShortRateModels\twofactormodel.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\twofactormodel.obj"	"$(INTDIR)\twofactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\twofactormodel.obj"	"$(INTDIR)\twofactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\twofactormodel.obj"	"$(INTDIR)\twofactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\twofactormodel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\PricingEngines\discretizedvanillaoption.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\discretizedvanillaoption.obj"	"$(INTDIR)\discretizedvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\discretizedvanillaoption.obj"	"$(INTDIR)\discretizedvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\discretizedvanillaoption.obj"	"$(INTDIR)\discretizedvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\discretizedvanillaoption.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\PricingEngines\europeananalyticalengine.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\europeananalyticalengine.obj"	"$(INTDIR)\europeananalyticalengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\europeananalyticalengine.obj"	"$(INTDIR)\europeananalyticalengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\europeananalyticalengine.obj"	"$(INTDIR)\europeananalyticalengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\europeananalyticalengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\PricingEngines\europeanbinomialengine.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\europeanbinomialengine.obj"	"$(INTDIR)\europeanbinomialengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\europeanbinomialengine.obj"	"$(INTDIR)\europeanbinomialengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\europeanbinomialengine.obj"	"$(INTDIR)\europeanbinomialengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\europeanbinomialengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\PricingEngines\quantovanillaanalyticengine.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\quantovanillaanalyticengine.obj"	"$(INTDIR)\quantovanillaanalyticengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\quantovanillaanalyticengine.obj"	"$(INTDIR)\quantovanillaanalyticengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\quantovanillaanalyticengine.obj"	"$(INTDIR)\quantovanillaanalyticengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\quantovanillaanalyticengine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\calendar.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\calendar.obj"	"$(INTDIR)\calendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\calendar.obj"	"$(INTDIR)\calendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\calendar.obj"	"$(INTDIR)\calendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\calendar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\dataformatters.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\dataformatters.obj"	"$(INTDIR)\dataformatters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\dataformatters.obj"	"$(INTDIR)\dataformatters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\dataformatters.obj"	"$(INTDIR)\dataformatters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\dataformatters.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\dataparsers.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\dataparsers.obj"	"$(INTDIR)\dataparsers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\dataparsers.obj"	"$(INTDIR)\dataparsers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\dataparsers.obj"	"$(INTDIR)\dataparsers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\dataparsers.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\date.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\date.obj"	"$(INTDIR)\date.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\date.obj"	"$(INTDIR)\date.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\date.obj"	"$(INTDIR)\date.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\date.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\option.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\option.obj"	"$(INTDIR)\option.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\option.obj"	"$(INTDIR)\option.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\option.obj"	"$(INTDIR)\option.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\option.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\scheduler.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\scheduler.obj"	"$(INTDIR)\scheduler.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\scheduler.obj"	"$(INTDIR)\scheduler.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\scheduler.obj"	"$(INTDIR)\scheduler.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\scheduler.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

SOURCE=.\ql\solver1d.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"


"$(INTDIR)\solver1d.obj"	"$(INTDIR)\solver1d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"


"$(INTDIR)\solver1d.obj"	"$(INTDIR)\solver1d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"


"$(INTDIR)\solver1d.obj"	"$(INTDIR)\solver1d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"


"$(INTDIR)\solver1d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 


!ENDIF 

