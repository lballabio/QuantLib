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

!IF  "$(CFG)" == "QuantLib - Win32 Release"

OUTDIR=.\build\Release
INTDIR=.\build\Release
# Begin Custom Macros
OutDir=.\build\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\analyticalvanillaengine.obj"
	-@erase "$(INTDIR)\analyticalvanillaengine.sbr"
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
	-@erase "$(INTDIR)\binomialvanillaengine.obj"
	-@erase "$(INTDIR)\binomialvanillaengine.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\budapest.obj"
	-@erase "$(INTDIR)\budapest.sbr"
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
	-@erase "$(INTDIR)\fdvanillaengine.obj"
	-@erase "$(INTDIR)\fdvanillaengine.sbr"
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
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
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
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
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
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
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
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
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
	-@erase "$(INTDIR)\voltermstructure.obj"
	-@erase "$(INTDIR)\voltermstructure.sbr"
	-@erase "$(INTDIR)\warsaw.obj"
	-@erase "$(INTDIR)\warsaw.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zerocurve.obj"
	-@erase "$(INTDIR)\zerocurve.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /Od /Ob2 /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
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
	"$(INTDIR)\zerocurve.sbr" \
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
	"$(INTDIR)\analyticalvanillaengine.sbr" \
	"$(INTDIR)\binomialvanillaengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\fdvanillaengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\voltermstructure.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\Win32\VisualStudio\QuantLib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
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
	"$(INTDIR)\zerocurve.obj" \
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
	"$(INTDIR)\analyticalvanillaengine.obj" \
	"$(INTDIR)\binomialvanillaengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\fdvanillaengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\voltermstructure.obj"

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

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\analyticalvanillaengine.obj"
	-@erase "$(INTDIR)\analyticalvanillaengine.sbr"
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
	-@erase "$(INTDIR)\binomialvanillaengine.obj"
	-@erase "$(INTDIR)\binomialvanillaengine.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\budapest.obj"
	-@erase "$(INTDIR)\budapest.sbr"
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
	-@erase "$(INTDIR)\fdvanillaengine.obj"
	-@erase "$(INTDIR)\fdvanillaengine.sbr"
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
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
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
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
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
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
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
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
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
	-@erase "$(INTDIR)\voltermstructure.obj"
	-@erase "$(INTDIR)\voltermstructure.sbr"
	-@erase "$(INTDIR)\warsaw.obj"
	-@erase "$(INTDIR)\warsaw.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zerocurve.obj"
	-@erase "$(INTDIR)\zerocurve.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.idb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.pdb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GR /GX /ZI /Od /I ".\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"lib\Win32\VisualStudio\QuantLib" /FD /GZ /c 

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
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
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
	"$(INTDIR)\zerocurve.sbr" \
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
	"$(INTDIR)\analyticalvanillaengine.sbr" \
	"$(INTDIR)\binomialvanillaengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\fdvanillaengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\voltermstructure.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib" 
LIB32_OBJS= \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
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
	"$(INTDIR)\zerocurve.obj" \
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
	"$(INTDIR)\analyticalvanillaengine.obj" \
	"$(INTDIR)\binomialvanillaengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\fdvanillaengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\voltermstructure.obj"

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

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : ".\lib\Win32\VisualStudio\QuantLib.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\analyticalvanillaengine.obj"
	-@erase "$(INTDIR)\analyticalvanillaengine.sbr"
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
	-@erase "$(INTDIR)\binomialvanillaengine.obj"
	-@erase "$(INTDIR)\binomialvanillaengine.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\budapest.obj"
	-@erase "$(INTDIR)\budapest.sbr"
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
	-@erase "$(INTDIR)\fdvanillaengine.obj"
	-@erase "$(INTDIR)\fdvanillaengine.sbr"
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
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
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
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
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
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
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
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
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
	-@erase "$(INTDIR)\voltermstructure.obj"
	-@erase "$(INTDIR)\voltermstructure.sbr"
	-@erase "$(INTDIR)\warsaw.obj"
	-@erase "$(INTDIR)\warsaw.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zerocurve.obj"
	-@erase "$(INTDIR)\zerocurve.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GR /GX /Od /Ob2 /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

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
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
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
	"$(INTDIR)\zerocurve.sbr" \
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
	"$(INTDIR)\analyticalvanillaengine.sbr" \
	"$(INTDIR)\binomialvanillaengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\fdvanillaengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\voltermstructure.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib.lib" 
LIB32_OBJS= \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
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
	"$(INTDIR)\zerocurve.obj" \
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
	"$(INTDIR)\analyticalvanillaengine.obj" \
	"$(INTDIR)\binomialvanillaengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\fdvanillaengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\voltermstructure.obj"

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

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : ".\lib\Win32\VisualStudio\QuantLib_d.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\analyticalcapfloor.obj"
	-@erase "$(INTDIR)\analyticalcapfloor.sbr"
	-@erase "$(INTDIR)\analyticalvanillaengine.obj"
	-@erase "$(INTDIR)\analyticalvanillaengine.sbr"
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
	-@erase "$(INTDIR)\binomialvanillaengine.obj"
	-@erase "$(INTDIR)\binomialvanillaengine.sbr"
	-@erase "$(INTDIR)\bisection.obj"
	-@erase "$(INTDIR)\bisection.sbr"
	-@erase "$(INTDIR)\blackcapfloor.obj"
	-@erase "$(INTDIR)\blackcapfloor.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaption.obj"
	-@erase "$(INTDIR)\blackswaption.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
	-@erase "$(INTDIR)\brent.obj"
	-@erase "$(INTDIR)\brent.sbr"
	-@erase "$(INTDIR)\bsmlattice.obj"
	-@erase "$(INTDIR)\bsmlattice.sbr"
	-@erase "$(INTDIR)\bsmoperator.obj"
	-@erase "$(INTDIR)\bsmoperator.sbr"
	-@erase "$(INTDIR)\budapest.obj"
	-@erase "$(INTDIR)\budapest.sbr"
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
	-@erase "$(INTDIR)\fdvanillaengine.obj"
	-@erase "$(INTDIR)\fdvanillaengine.sbr"
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
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
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
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
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
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\pagodapathpricer.obj"
	-@erase "$(INTDIR)\pagodapathpricer.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\performanceoptionpathpricer.obj"
	-@erase "$(INTDIR)\performanceoptionpathpricer.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
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
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
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
	-@erase "$(INTDIR)\voltermstructure.obj"
	-@erase "$(INTDIR)\voltermstructure.sbr"
	-@erase "$(INTDIR)\warsaw.obj"
	-@erase "$(INTDIR)\warsaw.sbr"
	-@erase "$(INTDIR)\wellington.obj"
	-@erase "$(INTDIR)\wellington.sbr"
	-@erase "$(INTDIR)\xibor.obj"
	-@erase "$(INTDIR)\xibor.sbr"
	-@erase "$(INTDIR)\xibormanager.obj"
	-@erase "$(INTDIR)\xibormanager.sbr"
	-@erase "$(INTDIR)\zerocurve.obj"
	-@erase "$(INTDIR)\zerocurve.sbr"
	-@erase "$(INTDIR)\zurich.obj"
	-@erase "$(INTDIR)\zurich.sbr"
	-@erase "$(OUTDIR)\QuantLib.bsc"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.idb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib.pdb"
	-@erase ".\lib\Win32\VisualStudio\QuantLib_d.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GR /GX /ZI /Od /I ".\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"lib\Win32\VisualStudio\QuantLib" /FD /GZ /c 

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
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\frankfurt.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\london.sbr" \
	"$(INTDIR)\milan.sbr" \
	"$(INTDIR)\newyork.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
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
	"$(INTDIR)\zerocurve.sbr" \
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
	"$(INTDIR)\analyticalvanillaengine.sbr" \
	"$(INTDIR)\binomialvanillaengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\fdvanillaengine.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\option.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\solver1d.sbr" \
	"$(INTDIR)\voltermstructure.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib" 
LIB32_OBJS= \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\frankfurt.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\london.obj" \
	"$(INTDIR)\milan.obj" \
	"$(INTDIR)\newyork.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
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
	"$(INTDIR)\zerocurve.obj" \
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
	"$(INTDIR)\analyticalvanillaengine.obj" \
	"$(INTDIR)\binomialvanillaengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\fdvanillaengine.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\option.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\solver1d.obj" \
	"$(INTDIR)\voltermstructure.obj"

".\lib\Win32\VisualStudio\QuantLib_d.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
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


!IF "$(CFG)" == "QuantLib - Win32 Release" || "$(CFG)" == "QuantLib - Win32 Debug" || "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease" || "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"
SOURCE=.\ql\Calendars\budapest.cpp

"$(INTDIR)\budapest.obj"	"$(INTDIR)\budapest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\frankfurt.cpp

"$(INTDIR)\frankfurt.obj"	"$(INTDIR)\frankfurt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\helsinki.cpp

"$(INTDIR)\helsinki.obj"	"$(INTDIR)\helsinki.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\johannesburg.cpp

"$(INTDIR)\johannesburg.obj"	"$(INTDIR)\johannesburg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\jointcalendar.cpp

"$(INTDIR)\jointcalendar.obj"	"$(INTDIR)\jointcalendar.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Calendars\oslo.cpp

"$(INTDIR)\oslo.obj"	"$(INTDIR)\oslo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\stockholm.cpp

"$(INTDIR)\stockholm.obj"	"$(INTDIR)\stockholm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\sydney.cpp

"$(INTDIR)\sydney.obj"	"$(INTDIR)\sydney.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\target.cpp

"$(INTDIR)\target.obj"	"$(INTDIR)\target.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\tokyo.cpp

"$(INTDIR)\tokyo.obj"	"$(INTDIR)\tokyo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\toronto.cpp

"$(INTDIR)\toronto.obj"	"$(INTDIR)\toronto.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\warsaw.cpp

"$(INTDIR)\warsaw.obj"	"$(INTDIR)\warsaw.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\wellington.cpp

"$(INTDIR)\wellington.obj"	"$(INTDIR)\wellington.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\zurich.cpp

"$(INTDIR)\zurich.obj"	"$(INTDIR)\zurich.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\CashFlows\cashflowvectors.cpp

"$(INTDIR)\cashflowvectors.obj"	"$(INTDIR)\cashflowvectors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\CashFlows\parcoupon.cpp

"$(INTDIR)\parcoupon.obj"	"$(INTDIR)\parcoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\CashFlows\shortfloatingcoupon.cpp

"$(INTDIR)\shortfloatingcoupon.obj"	"$(INTDIR)\shortfloatingcoupon.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\DayCounters\actualactual.cpp

"$(INTDIR)\actualactual.obj"	"$(INTDIR)\actualactual.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\DayCounters\thirty360.cpp

"$(INTDIR)\thirty360.obj"	"$(INTDIR)\thirty360.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\boundarycondition.cpp

"$(INTDIR)\boundarycondition.obj"	"$(INTDIR)\boundarycondition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\bsmoperator.cpp

"$(INTDIR)\bsmoperator.obj"	"$(INTDIR)\bsmoperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\onefactoroperator.cpp

"$(INTDIR)\onefactoroperator.obj"	"$(INTDIR)\onefactoroperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\tridiagonaloperator.cpp

"$(INTDIR)\tridiagonaloperator.obj"	"$(INTDIR)\tridiagonaloperator.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\FiniteDifferences\valueatcenter.cpp

"$(INTDIR)\valueatcenter.obj"	"$(INTDIR)\valueatcenter.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Indexes\xibor.cpp

"$(INTDIR)\xibor.obj"	"$(INTDIR)\xibor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Indexes\xibormanager.cpp

"$(INTDIR)\xibormanager.obj"	"$(INTDIR)\xibormanager.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\capfloor.cpp

"$(INTDIR)\capfloor.obj"	"$(INTDIR)\capfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\forwardvanillaoption.cpp

"$(INTDIR)\forwardvanillaoption.obj"	"$(INTDIR)\forwardvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\quantoforwardvanillaoption.cpp

"$(INTDIR)\quantoforwardvanillaoption.obj"	"$(INTDIR)\quantoforwardvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\quantovanillaoption.cpp

"$(INTDIR)\quantovanillaoption.obj"	"$(INTDIR)\quantovanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Instruments\swaption.cpp

"$(INTDIR)\swaption.obj"	"$(INTDIR)\swaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\vanillaoption.cpp

"$(INTDIR)\vanillaoption.obj"	"$(INTDIR)\vanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\chisquaredistribution.cpp

"$(INTDIR)\chisquaredistribution.obj"	"$(INTDIR)\chisquaredistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\gammadistribution.cpp

"$(INTDIR)\gammadistribution.obj"	"$(INTDIR)\gammadistribution.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Math\statistics.cpp

"$(INTDIR)\statistics.obj"	"$(INTDIR)\statistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\symmetricschurdecomposition.cpp

"$(INTDIR)\symmetricschurdecomposition.obj"	"$(INTDIR)\symmetricschurdecomposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\arithmeticapopathpricer.cpp

"$(INTDIR)\arithmeticapopathpricer.obj"	"$(INTDIR)\arithmeticapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\arithmeticasopathpricer.cpp

"$(INTDIR)\arithmeticasopathpricer.obj"	"$(INTDIR)\arithmeticasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\basketpathpricer.cpp

"$(INTDIR)\basketpathpricer.obj"	"$(INTDIR)\basketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\cliquetoptionpathpricer.cpp

"$(INTDIR)\cliquetoptionpathpricer.obj"	"$(INTDIR)\cliquetoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\europeanpathpricer.cpp

"$(INTDIR)\europeanpathpricer.obj"	"$(INTDIR)\europeanpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\everestpathpricer.cpp

"$(INTDIR)\everestpathpricer.obj"	"$(INTDIR)\everestpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\geometricapopathpricer.cpp

"$(INTDIR)\geometricapopathpricer.obj"	"$(INTDIR)\geometricapopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\geometricasopathpricer.cpp

"$(INTDIR)\geometricasopathpricer.obj"	"$(INTDIR)\geometricasopathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\getcovariance.cpp

"$(INTDIR)\getcovariance.obj"	"$(INTDIR)\getcovariance.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\himalayapathpricer.cpp

"$(INTDIR)\himalayapathpricer.obj"	"$(INTDIR)\himalayapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\maxbasketpathpricer.cpp

"$(INTDIR)\maxbasketpathpricer.obj"	"$(INTDIR)\maxbasketpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\pagodapathpricer.cpp

"$(INTDIR)\pagodapathpricer.obj"	"$(INTDIR)\pagodapathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\MonteCarlo\performanceoptionpathpricer.cpp

"$(INTDIR)\performanceoptionpathpricer.obj"	"$(INTDIR)\performanceoptionpathpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\analyticalcapfloor.cpp

"$(INTDIR)\analyticalcapfloor.obj"	"$(INTDIR)\analyticalcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\barrieroption.cpp

"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\binaryoption.cpp

"$(INTDIR)\binaryoption.obj"	"$(INTDIR)\binaryoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\blackcapfloor.cpp

"$(INTDIR)\blackcapfloor.obj"	"$(INTDIR)\blackcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\blackswaption.cpp

"$(INTDIR)\blackswaption.obj"	"$(INTDIR)\blackswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\capfloorpricer.cpp

"$(INTDIR)\capfloorpricer.obj"	"$(INTDIR)\capfloorpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\cliquetoption.cpp

"$(INTDIR)\cliquetoption.obj"	"$(INTDIR)\cliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\discretegeometricapo.cpp

"$(INTDIR)\discretegeometricapo.obj"	"$(INTDIR)\discretegeometricapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\discretegeometricaso.cpp

"$(INTDIR)\discretegeometricaso.obj"	"$(INTDIR)\discretegeometricaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\europeanoption.cpp

"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fdbermudanoption.cpp

"$(INTDIR)\fdbermudanoption.obj"	"$(INTDIR)\fdbermudanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fdbsmoption.cpp

"$(INTDIR)\fdbsmoption.obj"	"$(INTDIR)\fdbsmoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fddividendamericanoption.cpp

"$(INTDIR)\fddividendamericanoption.obj"	"$(INTDIR)\fddividendamericanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fddividendeuropeanoption.cpp

"$(INTDIR)\fddividendeuropeanoption.obj"	"$(INTDIR)\fddividendeuropeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fddividendoption.cpp

"$(INTDIR)\fddividendoption.obj"	"$(INTDIR)\fddividendoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fddividendshoutoption.cpp

"$(INTDIR)\fddividendshoutoption.obj"	"$(INTDIR)\fddividendshoutoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fdeuropean.cpp

"$(INTDIR)\fdeuropean.obj"	"$(INTDIR)\fdeuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fdmultiperiodoption.cpp

"$(INTDIR)\fdmultiperiodoption.obj"	"$(INTDIR)\fdmultiperiodoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\fdstepconditionoption.cpp

"$(INTDIR)\fdstepconditionoption.obj"	"$(INTDIR)\fdstepconditionoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\jamshidianswaption.cpp

"$(INTDIR)\jamshidianswaption.obj"	"$(INTDIR)\jamshidianswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcbasket.cpp

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mccliquetoption.cpp

"$(INTDIR)\mccliquetoption.obj"	"$(INTDIR)\mccliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcdiscretearithmeticapo.cpp

"$(INTDIR)\mcdiscretearithmeticapo.obj"	"$(INTDIR)\mcdiscretearithmeticapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcdiscretearithmeticaso.cpp

"$(INTDIR)\mcdiscretearithmeticaso.obj"	"$(INTDIR)\mcdiscretearithmeticaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mceuropean.cpp

"$(INTDIR)\mceuropean.obj"	"$(INTDIR)\mceuropean.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mceverest.cpp

"$(INTDIR)\mceverest.obj"	"$(INTDIR)\mceverest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mchimalaya.cpp

"$(INTDIR)\mchimalaya.obj"	"$(INTDIR)\mchimalaya.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcmaxbasket.cpp

"$(INTDIR)\mcmaxbasket.obj"	"$(INTDIR)\mcmaxbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcpagoda.cpp

"$(INTDIR)\mcpagoda.obj"	"$(INTDIR)\mcpagoda.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcperformanceoption.cpp

"$(INTDIR)\mcperformanceoption.obj"	"$(INTDIR)\mcperformanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\performanceoption.cpp

"$(INTDIR)\performanceoption.obj"	"$(INTDIR)\performanceoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\singleassetoption.cpp

"$(INTDIR)\singleassetoption.obj"	"$(INTDIR)\singleassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\swaptionpricer.cpp

"$(INTDIR)\swaptionpricer.obj"	"$(INTDIR)\swaptionpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\treecapfloor.cpp

"$(INTDIR)\treecapfloor.obj"	"$(INTDIR)\treecapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\treeswaption.cpp

"$(INTDIR)\treeswaption.obj"	"$(INTDIR)\treeswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\knuthuniformrng.cpp

"$(INTDIR)\knuthuniformrng.obj"	"$(INTDIR)\knuthuniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\lecuyeruniformrng.cpp

"$(INTDIR)\lecuyeruniformrng.obj"	"$(INTDIR)\lecuyeruniformrng.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\TermStructures\affinetermstructure.cpp

"$(INTDIR)\affinetermstructure.obj"	"$(INTDIR)\affinetermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\compoundforward.cpp

"$(INTDIR)\compoundforward.obj"	"$(INTDIR)\compoundforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\discountcurve.cpp

"$(INTDIR)\discountcurve.obj"	"$(INTDIR)\discountcurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\piecewiseflatforward.cpp

"$(INTDIR)\piecewiseflatforward.obj"	"$(INTDIR)\piecewiseflatforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\ratehelpers.cpp

"$(INTDIR)\ratehelpers.obj"	"$(INTDIR)\ratehelpers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\zerocurve.cpp

"$(INTDIR)\zerocurve.obj"	"$(INTDIR)\zerocurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Optimization\armijo.cpp

"$(INTDIR)\armijo.obj"	"$(INTDIR)\armijo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Optimization\conjugategradient.cpp

"$(INTDIR)\conjugategradient.obj"	"$(INTDIR)\conjugategradient.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Optimization\simplex.cpp

"$(INTDIR)\simplex.obj"	"$(INTDIR)\simplex.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Optimization\steepestdescent.cpp

"$(INTDIR)\steepestdescent.obj"	"$(INTDIR)\steepestdescent.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Lattices\binomialtree.cpp

"$(INTDIR)\binomialtree.obj"	"$(INTDIR)\binomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Lattices\bsmlattice.cpp

"$(INTDIR)\bsmlattice.obj"	"$(INTDIR)\bsmlattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Lattices\lattice.cpp

"$(INTDIR)\lattice.obj"	"$(INTDIR)\lattice.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Lattices\lattice2d.cpp

"$(INTDIR)\lattice2d.obj"	"$(INTDIR)\lattice2d.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Lattices\trinomialtree.cpp

"$(INTDIR)\trinomialtree.obj"	"$(INTDIR)\trinomialtree.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\functions\daycounters.cpp

"$(INTDIR)\daycounters.obj"	"$(INTDIR)\daycounters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\functions\mathf.cpp

"$(INTDIR)\mathf.obj"	"$(INTDIR)\mathf.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\functions\vols.cpp

"$(INTDIR)\vols.obj"	"$(INTDIR)\vols.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\CalibrationHelpers\caphelper.cpp

"$(INTDIR)\caphelper.obj"	"$(INTDIR)\caphelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\CalibrationHelpers\swaptionhelper.cpp

"$(INTDIR)\swaptionhelper.obj"	"$(INTDIR)\swaptionhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\OneFactorModels\blackkarasinski.cpp

"$(INTDIR)\blackkarasinski.obj"	"$(INTDIR)\blackkarasinski.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\OneFactorModels\coxingersollross.cpp

"$(INTDIR)\coxingersollross.obj"	"$(INTDIR)\coxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\OneFactorModels\extendedcoxingersollross.cpp

"$(INTDIR)\extendedcoxingersollross.obj"	"$(INTDIR)\extendedcoxingersollross.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\OneFactorModels\hullwhite.cpp

"$(INTDIR)\hullwhite.obj"	"$(INTDIR)\hullwhite.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\OneFactorModels\vasicek.cpp

"$(INTDIR)\vasicek.obj"	"$(INTDIR)\vasicek.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\TwoFactorModels\g2.cpp

"$(INTDIR)\g2.obj"	"$(INTDIR)\g2.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\calibrationhelper.cpp

"$(INTDIR)\calibrationhelper.obj"	"$(INTDIR)\calibrationhelper.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\model.cpp

"$(INTDIR)\model.obj"	"$(INTDIR)\model.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\onefactormodel.cpp

"$(INTDIR)\onefactormodel.obj"	"$(INTDIR)\onefactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\ShortRateModels\twofactormodel.cpp

"$(INTDIR)\twofactormodel.obj"	"$(INTDIR)\twofactormodel.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\analyticalvanillaengine.cpp

"$(INTDIR)\analyticalvanillaengine.obj"	"$(INTDIR)\analyticalvanillaengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\binomialvanillaengine.cpp

"$(INTDIR)\binomialvanillaengine.obj"	"$(INTDIR)\binomialvanillaengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\discretizedvanillaoption.cpp

"$(INTDIR)\discretizedvanillaoption.obj"	"$(INTDIR)\discretizedvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\fdvanillaengine.cpp

"$(INTDIR)\fdvanillaengine.obj"	"$(INTDIR)\fdvanillaengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

SOURCE=.\ql\calendar.cpp

"$(INTDIR)\calendar.obj"	"$(INTDIR)\calendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\dataformatters.cpp

"$(INTDIR)\dataformatters.obj"	"$(INTDIR)\dataformatters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\dataparsers.cpp

"$(INTDIR)\dataparsers.obj"	"$(INTDIR)\dataparsers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\date.cpp

"$(INTDIR)\date.obj"	"$(INTDIR)\date.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\grid.cpp

"$(INTDIR)\grid.obj"	"$(INTDIR)\grid.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\voltermstructure.cpp

"$(INTDIR)\voltermstructure.obj"	"$(INTDIR)\voltermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

