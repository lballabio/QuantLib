# Microsoft Developer Studio Generated NMAKE File, Based on QuantLib.dsp
!IF "$(CFG)" == ""
CFG=QuantLib - Win32 Debug SingleThread
!MESSAGE No configuration specified. Defaulting to QuantLib - Win32 Debug SingleThread.
!ENDIF 

!IF "$(CFG)" != "QuantLib - Win32 Release" && "$(CFG)" != "QuantLib - Win32 Debug" && "$(CFG)" != "QuantLib - Win32 Release MTDLL" && "$(CFG)" != "QuantLib - Win32 Debug MTDLL" && "$(CFG)" != "QuantLib - Win32 Release SingleThread" && "$(CFG)" != "QuantLib - Win32 Debug SingleThread"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QuantLib.mak" CFG="QuantLib - Win32 Debug SingleThread"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QuantLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Release MTDLL" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Debug MTDLL" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Release SingleThread" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Debug SingleThread" (based on "Win32 (x86) Static Library")
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

ALL : ".\lib\QuantLib-vc6-mt-s-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : "QuantLibFunctions - Win32 Release" ".\lib\QuantLib-vc6-mt-s-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLibFunctions - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\americanpayoffatexpiry.obj"
	-@erase "$(INTDIR)\americanpayoffatexpiry.sbr"
	-@erase "$(INTDIR)\americanpayoffathit.obj"
	-@erase "$(INTDIR)\americanpayoffathit.sbr"
	-@erase "$(INTDIR)\analyticbarrierengine.obj"
	-@erase "$(INTDIR)\analyticbarrierengine.sbr"
	-@erase "$(INTDIR)\analyticcapfloorengine.obj"
	-@erase "$(INTDIR)\analyticcapfloorengine.sbr"
	-@erase "$(INTDIR)\analyticcliquetengine.obj"
	-@erase "$(INTDIR)\analyticcliquetengine.sbr"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.obj"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.sbr"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.obj"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.sbr"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.obj"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.sbr"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticperformanceengine.obj"
	-@erase "$(INTDIR)\analyticperformanceengine.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\asianoption.obj"
	-@erase "$(INTDIR)\asianoption.sbr"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.obj"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basicdataformatters.obj"
	-@erase "$(INTDIR)\basicdataformatters.sbr"
	-@erase "$(INTDIR)\basispointsensitivity.obj"
	-@erase "$(INTDIR)\basispointsensitivity.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\beijing.obj"
	-@erase "$(INTDIR)\beijing.sbr"
	-@erase "$(INTDIR)\beta.obj"
	-@erase "$(INTDIR)\beta.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bivariatenormaldistribution.obj"
	-@erase "$(INTDIR)\bivariatenormaldistribution.sbr"
	-@erase "$(INTDIR)\bjerksundstenslandengine.obj"
	-@erase "$(INTDIR)\bjerksundstenslandengine.sbr"
	-@erase "$(INTDIR)\blackcapfloorengine.obj"
	-@erase "$(INTDIR)\blackcapfloorengine.sbr"
	-@erase "$(INTDIR)\blackformula.obj"
	-@erase "$(INTDIR)\blackformula.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaptionengine.obj"
	-@erase "$(INTDIR)\blackswaptionengine.sbr"
	-@erase "$(INTDIR)\blackvariancecurve.obj"
	-@erase "$(INTDIR)\blackvariancecurve.sbr"
	-@erase "$(INTDIR)\blackvariancesurface.obj"
	-@erase "$(INTDIR)\blackvariancesurface.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
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
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\choleskydecomposition.obj"
	-@erase "$(INTDIR)\choleskydecomposition.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\copenhagen.obj"
	-@erase "$(INTDIR)\copenhagen.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discrepancystatistics.obj"
	-@erase "$(INTDIR)\discrepancystatistics.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedasset.obj"
	-@erase "$(INTDIR)\discretizedasset.sbr"
	-@erase "$(INTDIR)\discretizedcapfloor.obj"
	-@erase "$(INTDIR)\discretizedcapfloor.sbr"
	-@erase "$(INTDIR)\discretizedswaption.obj"
	-@erase "$(INTDIR)\discretizedswaption.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendvanillaoption.obj"
	-@erase "$(INTDIR)\dividendvanillaoption.sbr"
	-@erase "$(INTDIR)\errorfunction.obj"
	-@erase "$(INTDIR)\errorfunction.sbr"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\errors.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\exercise.obj"
	-@erase "$(INTDIR)\exercise.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\extendeddiscountcurve.obj"
	-@erase "$(INTDIR)\extendeddiscountcurve.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
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
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\g2swaptionengine.obj"
	-@erase "$(INTDIR)\g2swaptionengine.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\generalstatistics.obj"
	-@erase "$(INTDIR)\generalstatistics.sbr"
	-@erase "$(INTDIR)\germany.obj"
	-@erase "$(INTDIR)\germany.sbr"
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
	-@erase "$(INTDIR)\haltonrsg.obj"
	-@erase "$(INTDIR)\haltonrsg.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\hongkong.obj"
	-@erase "$(INTDIR)\hongkong.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\incompletegamma.obj"
	-@erase "$(INTDIR)\incompletegamma.sbr"
	-@erase "$(INTDIR)\incrementalstatistics.obj"
	-@erase "$(INTDIR)\incrementalstatistics.sbr"
	-@erase "$(INTDIR)\integralengine.obj"
	-@erase "$(INTDIR)\integralengine.sbr"
	-@erase "$(INTDIR)\italy.obj"
	-@erase "$(INTDIR)\italy.sbr"
	-@erase "$(INTDIR)\jamshidianswaptionengine.obj"
	-@erase "$(INTDIR)\jamshidianswaptionengine.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
	-@erase "$(INTDIR)\jumpdiffusionengine.obj"
	-@erase "$(INTDIR)\jumpdiffusionengine.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\localvolsurface.obj"
	-@erase "$(INTDIR)\localvolsurface.sbr"
	-@erase "$(INTDIR)\mcamericanbasketengine.obj"
	-@erase "$(INTDIR)\mcamericanbasketengine.sbr"
	-@erase "$(INTDIR)\mcbarrierengine.obj"
	-@erase "$(INTDIR)\mcbarrierengine.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mcbasketengine.obj"
	-@erase "$(INTDIR)\mcbasketengine.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdigitalengine.obj"
	-@erase "$(INTDIR)\mcdigitalengine.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
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
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\mt19937uniformrng.obj"
	-@erase "$(INTDIR)\mt19937uniformrng.sbr"
	-@erase "$(INTDIR)\multiassetoption.obj"
	-@erase "$(INTDIR)\multiassetoption.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\oneassetoption.obj"
	-@erase "$(INTDIR)\oneassetoption.sbr"
	-@erase "$(INTDIR)\oneassetstrikedoption.obj"
	-@erase "$(INTDIR)\oneassetstrikedoption.sbr"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\primenumbers.obj"
	-@erase "$(INTDIR)\primenumbers.sbr"
	-@erase "$(INTDIR)\primitivepolynomials.obj"
	-@erase "$(INTDIR)\primitivepolynomials.sbr"
	-@erase "$(INTDIR)\pseudosqrt.obj"
	-@erase "$(INTDIR)\pseudosqrt.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\riyadh.obj"
	-@erase "$(INTDIR)\riyadh.sbr"
	-@erase "$(INTDIR)\rounding.obj"
	-@erase "$(INTDIR)\rounding.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\seoul.obj"
	-@erase "$(INTDIR)\seoul.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpledaycounter.obj"
	-@erase "$(INTDIR)\simpledaycounter.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singapore.obj"
	-@erase "$(INTDIR)\singapore.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\sobolrsg.obj"
	-@erase "$(INTDIR)\sobolrsg.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stochasticprocess.obj"
	-@erase "$(INTDIR)\stochasticprocess.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
	-@erase "$(INTDIR)\stulzengine.obj"
	-@erase "$(INTDIR)\stulzengine.sbr"
	-@erase "$(INTDIR)\svd.obj"
	-@erase "$(INTDIR)\svd.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\taiwan.obj"
	-@erase "$(INTDIR)\taiwan.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\timebasket.obj"
	-@erase "$(INTDIR)\timebasket.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloorengine.obj"
	-@erase "$(INTDIR)\treecapfloorengine.sbr"
	-@erase "$(INTDIR)\treeswaptionengine.obj"
	-@erase "$(INTDIR)\treeswaptionengine.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\unitedkingdom.obj"
	-@erase "$(INTDIR)\unitedkingdom.sbr"
	-@erase "$(INTDIR)\unitedstates.obj"
	-@erase "$(INTDIR)\unitedstates.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
	-@erase "$(INTDIR)\vc60.idb"
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
	-@erase ".\lib\QuantLib-vc6-mt-s-0_3_7.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Oi- /c 

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
	"$(INTDIR)\beijing.sbr" \
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\copenhagen.sbr" \
	"$(INTDIR)\germany.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\hongkong.sbr" \
	"$(INTDIR)\italy.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\riyadh.sbr" \
	"$(INTDIR)\seoul.sbr" \
	"$(INTDIR)\singapore.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\taiwan.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\unitedkingdom.sbr" \
	"$(INTDIR)\unitedstates.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\basispointsensitivity.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\timebasket.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\simpledaycounter.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\asianoption.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendvanillaoption.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\multiassetoption.sbr" \
	"$(INTDIR)\oneassetoption.sbr" \
	"$(INTDIR)\oneassetstrikedoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\beta.sbr" \
	"$(INTDIR)\bivariatenormaldistribution.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\choleskydecomposition.sbr" \
	"$(INTDIR)\discrepancystatistics.sbr" \
	"$(INTDIR)\errorfunction.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\generalstatistics.sbr" \
	"$(INTDIR)\incompletegamma.sbr" \
	"$(INTDIR)\incrementalstatistics.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\primenumbers.sbr" \
	"$(INTDIR)\pseudosqrt.sbr" \
	"$(INTDIR)\rounding.sbr" \
	"$(INTDIR)\svd.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\cliquetoptionpricer.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\analyticcontinuousasianengine.sbr" \
	"$(INTDIR)\analyticdiscreteasianengine.sbr" \
	"$(INTDIR)\analyticbarrierengine.sbr" \
	"$(INTDIR)\mcbarrierengine.sbr" \
	"$(INTDIR)\mcamericanbasketengine.sbr" \
	"$(INTDIR)\mcbasketengine.sbr" \
	"$(INTDIR)\stulzengine.sbr" \
	"$(INTDIR)\analyticdigitalamericanengine.sbr" \
	"$(INTDIR)\analyticdividendeuropeanengine.sbr" \
	"$(INTDIR)\analyticeuropeanengine.sbr" \
	"$(INTDIR)\baroneadesiwhaleyengine.sbr" \
	"$(INTDIR)\bjerksundstenslandengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\integralengine.sbr" \
	"$(INTDIR)\jumpdiffusionengine.sbr" \
	"$(INTDIR)\mcdigitalengine.sbr" \
	"$(INTDIR)\analyticcapfloorengine.sbr" \
	"$(INTDIR)\blackcapfloorengine.sbr" \
	"$(INTDIR)\discretizedcapfloor.sbr" \
	"$(INTDIR)\treecapfloorengine.sbr" \
	"$(INTDIR)\blackswaptionengine.sbr" \
	"$(INTDIR)\discretizedswaption.sbr" \
	"$(INTDIR)\jamshidianswaptionengine.sbr" \
	"$(INTDIR)\treeswaptionengine.sbr" \
	"$(INTDIR)\analyticcliquetengine.sbr" \
	"$(INTDIR)\analyticperformanceengine.sbr" \
	"$(INTDIR)\americanpayoffatexpiry.sbr" \
	"$(INTDIR)\americanpayoffathit.sbr" \
	"$(INTDIR)\blackformula.sbr" \
	"$(INTDIR)\haltonrsg.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\mt19937uniformrng.sbr" \
	"$(INTDIR)\primitivepolynomials.sbr" \
	"$(INTDIR)\sobolrsg.sbr" \
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
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\extendeddiscountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\zerocurve.sbr" \
	"$(INTDIR)\blackvariancecurve.sbr" \
	"$(INTDIR)\blackvariancesurface.sbr" \
	"$(INTDIR)\localvolsurface.sbr" \
	"$(INTDIR)\basicdataformatters.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\discretizedasset.sbr" \
	"$(INTDIR)\errors.sbr" \
	"$(INTDIR)\exercise.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\stochasticprocess.sbr" \
	"$(INTDIR)\voltermstructure.sbr" \
	"$(INTDIR)\g2swaptionengine.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\QuantLib-vc6-mt-s-0_3_7.lib" 
LIB32_OBJS= \
	"$(INTDIR)\beijing.obj" \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\copenhagen.obj" \
	"$(INTDIR)\germany.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\hongkong.obj" \
	"$(INTDIR)\italy.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\riyadh.obj" \
	"$(INTDIR)\seoul.obj" \
	"$(INTDIR)\singapore.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\taiwan.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\unitedkingdom.obj" \
	"$(INTDIR)\unitedstates.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\basispointsensitivity.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\timebasket.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\simpledaycounter.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\asianoption.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendvanillaoption.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\multiassetoption.obj" \
	"$(INTDIR)\oneassetoption.obj" \
	"$(INTDIR)\oneassetstrikedoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\beta.obj" \
	"$(INTDIR)\bivariatenormaldistribution.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\choleskydecomposition.obj" \
	"$(INTDIR)\discrepancystatistics.obj" \
	"$(INTDIR)\errorfunction.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\generalstatistics.obj" \
	"$(INTDIR)\incompletegamma.obj" \
	"$(INTDIR)\incrementalstatistics.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\primenumbers.obj" \
	"$(INTDIR)\pseudosqrt.obj" \
	"$(INTDIR)\rounding.obj" \
	"$(INTDIR)\svd.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\cliquetoptionpricer.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\analyticcontinuousasianengine.obj" \
	"$(INTDIR)\analyticdiscreteasianengine.obj" \
	"$(INTDIR)\analyticbarrierengine.obj" \
	"$(INTDIR)\mcbarrierengine.obj" \
	"$(INTDIR)\mcamericanbasketengine.obj" \
	"$(INTDIR)\mcbasketengine.obj" \
	"$(INTDIR)\stulzengine.obj" \
	"$(INTDIR)\analyticdigitalamericanengine.obj" \
	"$(INTDIR)\analyticdividendeuropeanengine.obj" \
	"$(INTDIR)\analyticeuropeanengine.obj" \
	"$(INTDIR)\baroneadesiwhaleyengine.obj" \
	"$(INTDIR)\bjerksundstenslandengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\integralengine.obj" \
	"$(INTDIR)\jumpdiffusionengine.obj" \
	"$(INTDIR)\mcdigitalengine.obj" \
	"$(INTDIR)\analyticcapfloorengine.obj" \
	"$(INTDIR)\blackcapfloorengine.obj" \
	"$(INTDIR)\discretizedcapfloor.obj" \
	"$(INTDIR)\treecapfloorengine.obj" \
	"$(INTDIR)\blackswaptionengine.obj" \
	"$(INTDIR)\discretizedswaption.obj" \
	"$(INTDIR)\jamshidianswaptionengine.obj" \
	"$(INTDIR)\treeswaptionengine.obj" \
	"$(INTDIR)\analyticcliquetengine.obj" \
	"$(INTDIR)\analyticperformanceengine.obj" \
	"$(INTDIR)\americanpayoffatexpiry.obj" \
	"$(INTDIR)\americanpayoffathit.obj" \
	"$(INTDIR)\blackformula.obj" \
	"$(INTDIR)\haltonrsg.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\mt19937uniformrng.obj" \
	"$(INTDIR)\primitivepolynomials.obj" \
	"$(INTDIR)\sobolrsg.obj" \
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
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\extendeddiscountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\zerocurve.obj" \
	"$(INTDIR)\blackvariancecurve.obj" \
	"$(INTDIR)\blackvariancesurface.obj" \
	"$(INTDIR)\localvolsurface.obj" \
	"$(INTDIR)\basicdataformatters.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\discretizedasset.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\exercise.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\stochasticprocess.obj" \
	"$(INTDIR)\voltermstructure.obj" \
	"$(INTDIR)\g2swaptionengine.obj" \
	".\lib\QuantLibFunctions-vc6-mt-s-0_3_7.lib"

".\lib\QuantLib-vc6-mt-s-0_3_7.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
   if not exist lib mkdir lib
	 $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

OUTDIR=.\build\Debug
INTDIR=.\build\Debug
# Begin Custom Macros
OutDir=.\build\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\QuantLib-vc6-mt-sgd-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : "QuantLibFunctions - Win32 Debug" ".\lib\QuantLib-vc6-mt-sgd-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLibFunctions - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\americanpayoffatexpiry.obj"
	-@erase "$(INTDIR)\americanpayoffatexpiry.sbr"
	-@erase "$(INTDIR)\americanpayoffathit.obj"
	-@erase "$(INTDIR)\americanpayoffathit.sbr"
	-@erase "$(INTDIR)\analyticbarrierengine.obj"
	-@erase "$(INTDIR)\analyticbarrierengine.sbr"
	-@erase "$(INTDIR)\analyticcapfloorengine.obj"
	-@erase "$(INTDIR)\analyticcapfloorengine.sbr"
	-@erase "$(INTDIR)\analyticcliquetengine.obj"
	-@erase "$(INTDIR)\analyticcliquetengine.sbr"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.obj"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.sbr"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.obj"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.sbr"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.obj"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.sbr"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticperformanceengine.obj"
	-@erase "$(INTDIR)\analyticperformanceengine.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\asianoption.obj"
	-@erase "$(INTDIR)\asianoption.sbr"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.obj"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basicdataformatters.obj"
	-@erase "$(INTDIR)\basicdataformatters.sbr"
	-@erase "$(INTDIR)\basispointsensitivity.obj"
	-@erase "$(INTDIR)\basispointsensitivity.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\beijing.obj"
	-@erase "$(INTDIR)\beijing.sbr"
	-@erase "$(INTDIR)\beta.obj"
	-@erase "$(INTDIR)\beta.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bivariatenormaldistribution.obj"
	-@erase "$(INTDIR)\bivariatenormaldistribution.sbr"
	-@erase "$(INTDIR)\bjerksundstenslandengine.obj"
	-@erase "$(INTDIR)\bjerksundstenslandengine.sbr"
	-@erase "$(INTDIR)\blackcapfloorengine.obj"
	-@erase "$(INTDIR)\blackcapfloorengine.sbr"
	-@erase "$(INTDIR)\blackformula.obj"
	-@erase "$(INTDIR)\blackformula.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaptionengine.obj"
	-@erase "$(INTDIR)\blackswaptionengine.sbr"
	-@erase "$(INTDIR)\blackvariancecurve.obj"
	-@erase "$(INTDIR)\blackvariancecurve.sbr"
	-@erase "$(INTDIR)\blackvariancesurface.obj"
	-@erase "$(INTDIR)\blackvariancesurface.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
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
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\choleskydecomposition.obj"
	-@erase "$(INTDIR)\choleskydecomposition.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\copenhagen.obj"
	-@erase "$(INTDIR)\copenhagen.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discrepancystatistics.obj"
	-@erase "$(INTDIR)\discrepancystatistics.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedasset.obj"
	-@erase "$(INTDIR)\discretizedasset.sbr"
	-@erase "$(INTDIR)\discretizedcapfloor.obj"
	-@erase "$(INTDIR)\discretizedcapfloor.sbr"
	-@erase "$(INTDIR)\discretizedswaption.obj"
	-@erase "$(INTDIR)\discretizedswaption.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendvanillaoption.obj"
	-@erase "$(INTDIR)\dividendvanillaoption.sbr"
	-@erase "$(INTDIR)\errorfunction.obj"
	-@erase "$(INTDIR)\errorfunction.sbr"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\errors.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\exercise.obj"
	-@erase "$(INTDIR)\exercise.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\extendeddiscountcurve.obj"
	-@erase "$(INTDIR)\extendeddiscountcurve.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
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
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\g2swaptionengine.obj"
	-@erase "$(INTDIR)\g2swaptionengine.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\generalstatistics.obj"
	-@erase "$(INTDIR)\generalstatistics.sbr"
	-@erase "$(INTDIR)\germany.obj"
	-@erase "$(INTDIR)\germany.sbr"
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
	-@erase "$(INTDIR)\haltonrsg.obj"
	-@erase "$(INTDIR)\haltonrsg.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\hongkong.obj"
	-@erase "$(INTDIR)\hongkong.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\incompletegamma.obj"
	-@erase "$(INTDIR)\incompletegamma.sbr"
	-@erase "$(INTDIR)\incrementalstatistics.obj"
	-@erase "$(INTDIR)\incrementalstatistics.sbr"
	-@erase "$(INTDIR)\integralengine.obj"
	-@erase "$(INTDIR)\integralengine.sbr"
	-@erase "$(INTDIR)\italy.obj"
	-@erase "$(INTDIR)\italy.sbr"
	-@erase "$(INTDIR)\jamshidianswaptionengine.obj"
	-@erase "$(INTDIR)\jamshidianswaptionengine.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
	-@erase "$(INTDIR)\jumpdiffusionengine.obj"
	-@erase "$(INTDIR)\jumpdiffusionengine.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\localvolsurface.obj"
	-@erase "$(INTDIR)\localvolsurface.sbr"
	-@erase "$(INTDIR)\mcamericanbasketengine.obj"
	-@erase "$(INTDIR)\mcamericanbasketengine.sbr"
	-@erase "$(INTDIR)\mcbarrierengine.obj"
	-@erase "$(INTDIR)\mcbarrierengine.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mcbasketengine.obj"
	-@erase "$(INTDIR)\mcbasketengine.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdigitalengine.obj"
	-@erase "$(INTDIR)\mcdigitalengine.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
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
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\mt19937uniformrng.obj"
	-@erase "$(INTDIR)\mt19937uniformrng.sbr"
	-@erase "$(INTDIR)\multiassetoption.obj"
	-@erase "$(INTDIR)\multiassetoption.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\oneassetoption.obj"
	-@erase "$(INTDIR)\oneassetoption.sbr"
	-@erase "$(INTDIR)\oneassetstrikedoption.obj"
	-@erase "$(INTDIR)\oneassetstrikedoption.sbr"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\primenumbers.obj"
	-@erase "$(INTDIR)\primenumbers.sbr"
	-@erase "$(INTDIR)\primitivepolynomials.obj"
	-@erase "$(INTDIR)\primitivepolynomials.sbr"
	-@erase "$(INTDIR)\pseudosqrt.obj"
	-@erase "$(INTDIR)\pseudosqrt.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\riyadh.obj"
	-@erase "$(INTDIR)\riyadh.sbr"
	-@erase "$(INTDIR)\rounding.obj"
	-@erase "$(INTDIR)\rounding.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\seoul.obj"
	-@erase "$(INTDIR)\seoul.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpledaycounter.obj"
	-@erase "$(INTDIR)\simpledaycounter.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singapore.obj"
	-@erase "$(INTDIR)\singapore.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\sobolrsg.obj"
	-@erase "$(INTDIR)\sobolrsg.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stochasticprocess.obj"
	-@erase "$(INTDIR)\stochasticprocess.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
	-@erase "$(INTDIR)\stulzengine.obj"
	-@erase "$(INTDIR)\stulzengine.sbr"
	-@erase "$(INTDIR)\svd.obj"
	-@erase "$(INTDIR)\svd.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\taiwan.obj"
	-@erase "$(INTDIR)\taiwan.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\timebasket.obj"
	-@erase "$(INTDIR)\timebasket.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloorengine.obj"
	-@erase "$(INTDIR)\treecapfloorengine.sbr"
	-@erase "$(INTDIR)\treeswaptionengine.obj"
	-@erase "$(INTDIR)\treeswaptionengine.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\unitedkingdom.obj"
	-@erase "$(INTDIR)\unitedkingdom.sbr"
	-@erase "$(INTDIR)\unitedstates.obj"
	-@erase "$(INTDIR)\unitedstates.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
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
	-@erase ".\lib\QuantLib-vc6-mt-sgd-0_3_7.idb"
	-@erase ".\lib\QuantLib-vc6-mt-sgd-0_3_7.lib"
	-@erase ".\lib\QuantLib-vc6-mt-sgd-0_3_7.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd".\lib\QuantLib-vc6-mt-sgd-0_3_7" /FD /GZ /c 

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
	"$(INTDIR)\beijing.sbr" \
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\copenhagen.sbr" \
	"$(INTDIR)\germany.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\hongkong.sbr" \
	"$(INTDIR)\italy.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\riyadh.sbr" \
	"$(INTDIR)\seoul.sbr" \
	"$(INTDIR)\singapore.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\taiwan.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\unitedkingdom.sbr" \
	"$(INTDIR)\unitedstates.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\basispointsensitivity.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\timebasket.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\simpledaycounter.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\asianoption.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendvanillaoption.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\multiassetoption.sbr" \
	"$(INTDIR)\oneassetoption.sbr" \
	"$(INTDIR)\oneassetstrikedoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\beta.sbr" \
	"$(INTDIR)\bivariatenormaldistribution.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\choleskydecomposition.sbr" \
	"$(INTDIR)\discrepancystatistics.sbr" \
	"$(INTDIR)\errorfunction.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\generalstatistics.sbr" \
	"$(INTDIR)\incompletegamma.sbr" \
	"$(INTDIR)\incrementalstatistics.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\primenumbers.sbr" \
	"$(INTDIR)\pseudosqrt.sbr" \
	"$(INTDIR)\rounding.sbr" \
	"$(INTDIR)\svd.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\cliquetoptionpricer.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\analyticcontinuousasianengine.sbr" \
	"$(INTDIR)\analyticdiscreteasianengine.sbr" \
	"$(INTDIR)\analyticbarrierengine.sbr" \
	"$(INTDIR)\mcbarrierengine.sbr" \
	"$(INTDIR)\mcamericanbasketengine.sbr" \
	"$(INTDIR)\mcbasketengine.sbr" \
	"$(INTDIR)\stulzengine.sbr" \
	"$(INTDIR)\analyticdigitalamericanengine.sbr" \
	"$(INTDIR)\analyticdividendeuropeanengine.sbr" \
	"$(INTDIR)\analyticeuropeanengine.sbr" \
	"$(INTDIR)\baroneadesiwhaleyengine.sbr" \
	"$(INTDIR)\bjerksundstenslandengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\integralengine.sbr" \
	"$(INTDIR)\jumpdiffusionengine.sbr" \
	"$(INTDIR)\mcdigitalengine.sbr" \
	"$(INTDIR)\analyticcapfloorengine.sbr" \
	"$(INTDIR)\blackcapfloorengine.sbr" \
	"$(INTDIR)\discretizedcapfloor.sbr" \
	"$(INTDIR)\treecapfloorengine.sbr" \
	"$(INTDIR)\blackswaptionengine.sbr" \
	"$(INTDIR)\discretizedswaption.sbr" \
	"$(INTDIR)\jamshidianswaptionengine.sbr" \
	"$(INTDIR)\treeswaptionengine.sbr" \
	"$(INTDIR)\analyticcliquetengine.sbr" \
	"$(INTDIR)\analyticperformanceengine.sbr" \
	"$(INTDIR)\americanpayoffatexpiry.sbr" \
	"$(INTDIR)\americanpayoffathit.sbr" \
	"$(INTDIR)\blackformula.sbr" \
	"$(INTDIR)\haltonrsg.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\mt19937uniformrng.sbr" \
	"$(INTDIR)\primitivepolynomials.sbr" \
	"$(INTDIR)\sobolrsg.sbr" \
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
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\extendeddiscountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\zerocurve.sbr" \
	"$(INTDIR)\blackvariancecurve.sbr" \
	"$(INTDIR)\blackvariancesurface.sbr" \
	"$(INTDIR)\localvolsurface.sbr" \
	"$(INTDIR)\basicdataformatters.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\discretizedasset.sbr" \
	"$(INTDIR)\errors.sbr" \
	"$(INTDIR)\exercise.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\stochasticprocess.sbr" \
	"$(INTDIR)\voltermstructure.sbr" \
	"$(INTDIR)\g2swaptionengine.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\QuantLib-vc6-mt-sgd-0_3_7.lib" 
LIB32_OBJS= \
	"$(INTDIR)\beijing.obj" \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\copenhagen.obj" \
	"$(INTDIR)\germany.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\hongkong.obj" \
	"$(INTDIR)\italy.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\riyadh.obj" \
	"$(INTDIR)\seoul.obj" \
	"$(INTDIR)\singapore.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\taiwan.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\unitedkingdom.obj" \
	"$(INTDIR)\unitedstates.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\basispointsensitivity.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\timebasket.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\simpledaycounter.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\asianoption.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendvanillaoption.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\multiassetoption.obj" \
	"$(INTDIR)\oneassetoption.obj" \
	"$(INTDIR)\oneassetstrikedoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\beta.obj" \
	"$(INTDIR)\bivariatenormaldistribution.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\choleskydecomposition.obj" \
	"$(INTDIR)\discrepancystatistics.obj" \
	"$(INTDIR)\errorfunction.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\generalstatistics.obj" \
	"$(INTDIR)\incompletegamma.obj" \
	"$(INTDIR)\incrementalstatistics.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\primenumbers.obj" \
	"$(INTDIR)\pseudosqrt.obj" \
	"$(INTDIR)\rounding.obj" \
	"$(INTDIR)\svd.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\cliquetoptionpricer.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\analyticcontinuousasianengine.obj" \
	"$(INTDIR)\analyticdiscreteasianengine.obj" \
	"$(INTDIR)\analyticbarrierengine.obj" \
	"$(INTDIR)\mcbarrierengine.obj" \
	"$(INTDIR)\mcamericanbasketengine.obj" \
	"$(INTDIR)\mcbasketengine.obj" \
	"$(INTDIR)\stulzengine.obj" \
	"$(INTDIR)\analyticdigitalamericanengine.obj" \
	"$(INTDIR)\analyticdividendeuropeanengine.obj" \
	"$(INTDIR)\analyticeuropeanengine.obj" \
	"$(INTDIR)\baroneadesiwhaleyengine.obj" \
	"$(INTDIR)\bjerksundstenslandengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\integralengine.obj" \
	"$(INTDIR)\jumpdiffusionengine.obj" \
	"$(INTDIR)\mcdigitalengine.obj" \
	"$(INTDIR)\analyticcapfloorengine.obj" \
	"$(INTDIR)\blackcapfloorengine.obj" \
	"$(INTDIR)\discretizedcapfloor.obj" \
	"$(INTDIR)\treecapfloorengine.obj" \
	"$(INTDIR)\blackswaptionengine.obj" \
	"$(INTDIR)\discretizedswaption.obj" \
	"$(INTDIR)\jamshidianswaptionengine.obj" \
	"$(INTDIR)\treeswaptionengine.obj" \
	"$(INTDIR)\analyticcliquetengine.obj" \
	"$(INTDIR)\analyticperformanceengine.obj" \
	"$(INTDIR)\americanpayoffatexpiry.obj" \
	"$(INTDIR)\americanpayoffathit.obj" \
	"$(INTDIR)\blackformula.obj" \
	"$(INTDIR)\haltonrsg.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\mt19937uniformrng.obj" \
	"$(INTDIR)\primitivepolynomials.obj" \
	"$(INTDIR)\sobolrsg.obj" \
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
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\extendeddiscountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\zerocurve.obj" \
	"$(INTDIR)\blackvariancecurve.obj" \
	"$(INTDIR)\blackvariancesurface.obj" \
	"$(INTDIR)\localvolsurface.obj" \
	"$(INTDIR)\basicdataformatters.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\discretizedasset.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\exercise.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\stochasticprocess.obj" \
	"$(INTDIR)\voltermstructure.obj" \
	"$(INTDIR)\g2swaptionengine.obj" \
	".\lib\QuantLibFunctions-vc6-mt-sgd-0_3_7.lib"

".\lib\QuantLib-vc6-mt-sgd-0_3_7.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
   if not exist lib mkdir lib
	 $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Release MTDLL"

OUTDIR=.\build\ReleaseMTDLL
INTDIR=.\build\ReleaseMTDLL
# Begin Custom Macros
OutDir=.\build\ReleaseMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\QuantLib-vc6-mt-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : "QuantLibFunctions - Win32 Release MTDLL" ".\lib\QuantLib-vc6-mt-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLibFunctions - Win32 Release MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\americanpayoffatexpiry.obj"
	-@erase "$(INTDIR)\americanpayoffatexpiry.sbr"
	-@erase "$(INTDIR)\americanpayoffathit.obj"
	-@erase "$(INTDIR)\americanpayoffathit.sbr"
	-@erase "$(INTDIR)\analyticbarrierengine.obj"
	-@erase "$(INTDIR)\analyticbarrierengine.sbr"
	-@erase "$(INTDIR)\analyticcapfloorengine.obj"
	-@erase "$(INTDIR)\analyticcapfloorengine.sbr"
	-@erase "$(INTDIR)\analyticcliquetengine.obj"
	-@erase "$(INTDIR)\analyticcliquetengine.sbr"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.obj"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.sbr"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.obj"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.sbr"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.obj"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.sbr"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticperformanceengine.obj"
	-@erase "$(INTDIR)\analyticperformanceengine.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\asianoption.obj"
	-@erase "$(INTDIR)\asianoption.sbr"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.obj"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basicdataformatters.obj"
	-@erase "$(INTDIR)\basicdataformatters.sbr"
	-@erase "$(INTDIR)\basispointsensitivity.obj"
	-@erase "$(INTDIR)\basispointsensitivity.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\beijing.obj"
	-@erase "$(INTDIR)\beijing.sbr"
	-@erase "$(INTDIR)\beta.obj"
	-@erase "$(INTDIR)\beta.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bivariatenormaldistribution.obj"
	-@erase "$(INTDIR)\bivariatenormaldistribution.sbr"
	-@erase "$(INTDIR)\bjerksundstenslandengine.obj"
	-@erase "$(INTDIR)\bjerksundstenslandengine.sbr"
	-@erase "$(INTDIR)\blackcapfloorengine.obj"
	-@erase "$(INTDIR)\blackcapfloorengine.sbr"
	-@erase "$(INTDIR)\blackformula.obj"
	-@erase "$(INTDIR)\blackformula.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaptionengine.obj"
	-@erase "$(INTDIR)\blackswaptionengine.sbr"
	-@erase "$(INTDIR)\blackvariancecurve.obj"
	-@erase "$(INTDIR)\blackvariancecurve.sbr"
	-@erase "$(INTDIR)\blackvariancesurface.obj"
	-@erase "$(INTDIR)\blackvariancesurface.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
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
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\choleskydecomposition.obj"
	-@erase "$(INTDIR)\choleskydecomposition.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\copenhagen.obj"
	-@erase "$(INTDIR)\copenhagen.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discrepancystatistics.obj"
	-@erase "$(INTDIR)\discrepancystatistics.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedasset.obj"
	-@erase "$(INTDIR)\discretizedasset.sbr"
	-@erase "$(INTDIR)\discretizedcapfloor.obj"
	-@erase "$(INTDIR)\discretizedcapfloor.sbr"
	-@erase "$(INTDIR)\discretizedswaption.obj"
	-@erase "$(INTDIR)\discretizedswaption.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendvanillaoption.obj"
	-@erase "$(INTDIR)\dividendvanillaoption.sbr"
	-@erase "$(INTDIR)\errorfunction.obj"
	-@erase "$(INTDIR)\errorfunction.sbr"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\errors.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\exercise.obj"
	-@erase "$(INTDIR)\exercise.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\extendeddiscountcurve.obj"
	-@erase "$(INTDIR)\extendeddiscountcurve.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
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
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\g2swaptionengine.obj"
	-@erase "$(INTDIR)\g2swaptionengine.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\generalstatistics.obj"
	-@erase "$(INTDIR)\generalstatistics.sbr"
	-@erase "$(INTDIR)\germany.obj"
	-@erase "$(INTDIR)\germany.sbr"
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
	-@erase "$(INTDIR)\haltonrsg.obj"
	-@erase "$(INTDIR)\haltonrsg.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\hongkong.obj"
	-@erase "$(INTDIR)\hongkong.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\incompletegamma.obj"
	-@erase "$(INTDIR)\incompletegamma.sbr"
	-@erase "$(INTDIR)\incrementalstatistics.obj"
	-@erase "$(INTDIR)\incrementalstatistics.sbr"
	-@erase "$(INTDIR)\integralengine.obj"
	-@erase "$(INTDIR)\integralengine.sbr"
	-@erase "$(INTDIR)\italy.obj"
	-@erase "$(INTDIR)\italy.sbr"
	-@erase "$(INTDIR)\jamshidianswaptionengine.obj"
	-@erase "$(INTDIR)\jamshidianswaptionengine.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
	-@erase "$(INTDIR)\jumpdiffusionengine.obj"
	-@erase "$(INTDIR)\jumpdiffusionengine.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\localvolsurface.obj"
	-@erase "$(INTDIR)\localvolsurface.sbr"
	-@erase "$(INTDIR)\mcamericanbasketengine.obj"
	-@erase "$(INTDIR)\mcamericanbasketengine.sbr"
	-@erase "$(INTDIR)\mcbarrierengine.obj"
	-@erase "$(INTDIR)\mcbarrierengine.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mcbasketengine.obj"
	-@erase "$(INTDIR)\mcbasketengine.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdigitalengine.obj"
	-@erase "$(INTDIR)\mcdigitalengine.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
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
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\mt19937uniformrng.obj"
	-@erase "$(INTDIR)\mt19937uniformrng.sbr"
	-@erase "$(INTDIR)\multiassetoption.obj"
	-@erase "$(INTDIR)\multiassetoption.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\oneassetoption.obj"
	-@erase "$(INTDIR)\oneassetoption.sbr"
	-@erase "$(INTDIR)\oneassetstrikedoption.obj"
	-@erase "$(INTDIR)\oneassetstrikedoption.sbr"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\primenumbers.obj"
	-@erase "$(INTDIR)\primenumbers.sbr"
	-@erase "$(INTDIR)\primitivepolynomials.obj"
	-@erase "$(INTDIR)\primitivepolynomials.sbr"
	-@erase "$(INTDIR)\pseudosqrt.obj"
	-@erase "$(INTDIR)\pseudosqrt.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\riyadh.obj"
	-@erase "$(INTDIR)\riyadh.sbr"
	-@erase "$(INTDIR)\rounding.obj"
	-@erase "$(INTDIR)\rounding.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\seoul.obj"
	-@erase "$(INTDIR)\seoul.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpledaycounter.obj"
	-@erase "$(INTDIR)\simpledaycounter.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singapore.obj"
	-@erase "$(INTDIR)\singapore.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\sobolrsg.obj"
	-@erase "$(INTDIR)\sobolrsg.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stochasticprocess.obj"
	-@erase "$(INTDIR)\stochasticprocess.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
	-@erase "$(INTDIR)\stulzengine.obj"
	-@erase "$(INTDIR)\stulzengine.sbr"
	-@erase "$(INTDIR)\svd.obj"
	-@erase "$(INTDIR)\svd.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\taiwan.obj"
	-@erase "$(INTDIR)\taiwan.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\timebasket.obj"
	-@erase "$(INTDIR)\timebasket.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloorengine.obj"
	-@erase "$(INTDIR)\treecapfloorengine.sbr"
	-@erase "$(INTDIR)\treeswaptionengine.obj"
	-@erase "$(INTDIR)\treeswaptionengine.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\unitedkingdom.obj"
	-@erase "$(INTDIR)\unitedkingdom.sbr"
	-@erase "$(INTDIR)\unitedstates.obj"
	-@erase "$(INTDIR)\unitedstates.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
	-@erase "$(INTDIR)\vc60.idb"
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
	-@erase ".\lib\QuantLib-vc6-mt-0_3_7.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Oi- /c 

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
	"$(INTDIR)\beijing.sbr" \
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\copenhagen.sbr" \
	"$(INTDIR)\germany.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\hongkong.sbr" \
	"$(INTDIR)\italy.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\riyadh.sbr" \
	"$(INTDIR)\seoul.sbr" \
	"$(INTDIR)\singapore.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\taiwan.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\unitedkingdom.sbr" \
	"$(INTDIR)\unitedstates.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\basispointsensitivity.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\timebasket.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\simpledaycounter.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\asianoption.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendvanillaoption.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\multiassetoption.sbr" \
	"$(INTDIR)\oneassetoption.sbr" \
	"$(INTDIR)\oneassetstrikedoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\beta.sbr" \
	"$(INTDIR)\bivariatenormaldistribution.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\choleskydecomposition.sbr" \
	"$(INTDIR)\discrepancystatistics.sbr" \
	"$(INTDIR)\errorfunction.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\generalstatistics.sbr" \
	"$(INTDIR)\incompletegamma.sbr" \
	"$(INTDIR)\incrementalstatistics.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\primenumbers.sbr" \
	"$(INTDIR)\pseudosqrt.sbr" \
	"$(INTDIR)\rounding.sbr" \
	"$(INTDIR)\svd.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\cliquetoptionpricer.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\analyticcontinuousasianengine.sbr" \
	"$(INTDIR)\analyticdiscreteasianengine.sbr" \
	"$(INTDIR)\analyticbarrierengine.sbr" \
	"$(INTDIR)\mcbarrierengine.sbr" \
	"$(INTDIR)\mcamericanbasketengine.sbr" \
	"$(INTDIR)\mcbasketengine.sbr" \
	"$(INTDIR)\stulzengine.sbr" \
	"$(INTDIR)\analyticdigitalamericanengine.sbr" \
	"$(INTDIR)\analyticdividendeuropeanengine.sbr" \
	"$(INTDIR)\analyticeuropeanengine.sbr" \
	"$(INTDIR)\baroneadesiwhaleyengine.sbr" \
	"$(INTDIR)\bjerksundstenslandengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\integralengine.sbr" \
	"$(INTDIR)\jumpdiffusionengine.sbr" \
	"$(INTDIR)\mcdigitalengine.sbr" \
	"$(INTDIR)\analyticcapfloorengine.sbr" \
	"$(INTDIR)\blackcapfloorengine.sbr" \
	"$(INTDIR)\discretizedcapfloor.sbr" \
	"$(INTDIR)\treecapfloorengine.sbr" \
	"$(INTDIR)\blackswaptionengine.sbr" \
	"$(INTDIR)\discretizedswaption.sbr" \
	"$(INTDIR)\jamshidianswaptionengine.sbr" \
	"$(INTDIR)\treeswaptionengine.sbr" \
	"$(INTDIR)\analyticcliquetengine.sbr" \
	"$(INTDIR)\analyticperformanceengine.sbr" \
	"$(INTDIR)\americanpayoffatexpiry.sbr" \
	"$(INTDIR)\americanpayoffathit.sbr" \
	"$(INTDIR)\blackformula.sbr" \
	"$(INTDIR)\haltonrsg.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\mt19937uniformrng.sbr" \
	"$(INTDIR)\primitivepolynomials.sbr" \
	"$(INTDIR)\sobolrsg.sbr" \
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
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\extendeddiscountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\zerocurve.sbr" \
	"$(INTDIR)\blackvariancecurve.sbr" \
	"$(INTDIR)\blackvariancesurface.sbr" \
	"$(INTDIR)\localvolsurface.sbr" \
	"$(INTDIR)\basicdataformatters.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\discretizedasset.sbr" \
	"$(INTDIR)\errors.sbr" \
	"$(INTDIR)\exercise.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\stochasticprocess.sbr" \
	"$(INTDIR)\voltermstructure.sbr" \
	"$(INTDIR)\g2swaptionengine.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\QuantLib-vc6-mt-0_3_7.lib" 
LIB32_OBJS= \
	"$(INTDIR)\beijing.obj" \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\copenhagen.obj" \
	"$(INTDIR)\germany.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\hongkong.obj" \
	"$(INTDIR)\italy.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\riyadh.obj" \
	"$(INTDIR)\seoul.obj" \
	"$(INTDIR)\singapore.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\taiwan.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\unitedkingdom.obj" \
	"$(INTDIR)\unitedstates.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\basispointsensitivity.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\timebasket.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\simpledaycounter.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\asianoption.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendvanillaoption.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\multiassetoption.obj" \
	"$(INTDIR)\oneassetoption.obj" \
	"$(INTDIR)\oneassetstrikedoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\beta.obj" \
	"$(INTDIR)\bivariatenormaldistribution.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\choleskydecomposition.obj" \
	"$(INTDIR)\discrepancystatistics.obj" \
	"$(INTDIR)\errorfunction.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\generalstatistics.obj" \
	"$(INTDIR)\incompletegamma.obj" \
	"$(INTDIR)\incrementalstatistics.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\primenumbers.obj" \
	"$(INTDIR)\pseudosqrt.obj" \
	"$(INTDIR)\rounding.obj" \
	"$(INTDIR)\svd.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\cliquetoptionpricer.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\analyticcontinuousasianengine.obj" \
	"$(INTDIR)\analyticdiscreteasianengine.obj" \
	"$(INTDIR)\analyticbarrierengine.obj" \
	"$(INTDIR)\mcbarrierengine.obj" \
	"$(INTDIR)\mcamericanbasketengine.obj" \
	"$(INTDIR)\mcbasketengine.obj" \
	"$(INTDIR)\stulzengine.obj" \
	"$(INTDIR)\analyticdigitalamericanengine.obj" \
	"$(INTDIR)\analyticdividendeuropeanengine.obj" \
	"$(INTDIR)\analyticeuropeanengine.obj" \
	"$(INTDIR)\baroneadesiwhaleyengine.obj" \
	"$(INTDIR)\bjerksundstenslandengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\integralengine.obj" \
	"$(INTDIR)\jumpdiffusionengine.obj" \
	"$(INTDIR)\mcdigitalengine.obj" \
	"$(INTDIR)\analyticcapfloorengine.obj" \
	"$(INTDIR)\blackcapfloorengine.obj" \
	"$(INTDIR)\discretizedcapfloor.obj" \
	"$(INTDIR)\treecapfloorengine.obj" \
	"$(INTDIR)\blackswaptionengine.obj" \
	"$(INTDIR)\discretizedswaption.obj" \
	"$(INTDIR)\jamshidianswaptionengine.obj" \
	"$(INTDIR)\treeswaptionengine.obj" \
	"$(INTDIR)\analyticcliquetengine.obj" \
	"$(INTDIR)\analyticperformanceengine.obj" \
	"$(INTDIR)\americanpayoffatexpiry.obj" \
	"$(INTDIR)\americanpayoffathit.obj" \
	"$(INTDIR)\blackformula.obj" \
	"$(INTDIR)\haltonrsg.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\mt19937uniformrng.obj" \
	"$(INTDIR)\primitivepolynomials.obj" \
	"$(INTDIR)\sobolrsg.obj" \
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
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\extendeddiscountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\zerocurve.obj" \
	"$(INTDIR)\blackvariancecurve.obj" \
	"$(INTDIR)\blackvariancesurface.obj" \
	"$(INTDIR)\localvolsurface.obj" \
	"$(INTDIR)\basicdataformatters.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\discretizedasset.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\exercise.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\stochasticprocess.obj" \
	"$(INTDIR)\voltermstructure.obj" \
	"$(INTDIR)\g2swaptionengine.obj" \
	".\lib\QuantLibFunctions-vc6-mt-0_3_7.lib"

".\lib\QuantLib-vc6-mt-0_3_7.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
   if not exist lib mkdir lib
	 $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug MTDLL"

OUTDIR=.\build\DebugMTDLL
INTDIR=.\build\DebugMTDLL
# Begin Custom Macros
OutDir=.\build\DebugMTDLL
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\QuantLib-vc6-mt-gd-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : "QuantLibFunctions - Win32 Debug MTDLL" ".\lib\QuantLib-vc6-mt-gd-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLibFunctions - Win32 Debug MTDLLCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\americanpayoffatexpiry.obj"
	-@erase "$(INTDIR)\americanpayoffatexpiry.sbr"
	-@erase "$(INTDIR)\americanpayoffathit.obj"
	-@erase "$(INTDIR)\americanpayoffathit.sbr"
	-@erase "$(INTDIR)\analyticbarrierengine.obj"
	-@erase "$(INTDIR)\analyticbarrierengine.sbr"
	-@erase "$(INTDIR)\analyticcapfloorengine.obj"
	-@erase "$(INTDIR)\analyticcapfloorengine.sbr"
	-@erase "$(INTDIR)\analyticcliquetengine.obj"
	-@erase "$(INTDIR)\analyticcliquetengine.sbr"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.obj"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.sbr"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.obj"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.sbr"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.obj"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.sbr"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticperformanceengine.obj"
	-@erase "$(INTDIR)\analyticperformanceengine.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\asianoption.obj"
	-@erase "$(INTDIR)\asianoption.sbr"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.obj"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basicdataformatters.obj"
	-@erase "$(INTDIR)\basicdataformatters.sbr"
	-@erase "$(INTDIR)\basispointsensitivity.obj"
	-@erase "$(INTDIR)\basispointsensitivity.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\beijing.obj"
	-@erase "$(INTDIR)\beijing.sbr"
	-@erase "$(INTDIR)\beta.obj"
	-@erase "$(INTDIR)\beta.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bivariatenormaldistribution.obj"
	-@erase "$(INTDIR)\bivariatenormaldistribution.sbr"
	-@erase "$(INTDIR)\bjerksundstenslandengine.obj"
	-@erase "$(INTDIR)\bjerksundstenslandengine.sbr"
	-@erase "$(INTDIR)\blackcapfloorengine.obj"
	-@erase "$(INTDIR)\blackcapfloorengine.sbr"
	-@erase "$(INTDIR)\blackformula.obj"
	-@erase "$(INTDIR)\blackformula.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaptionengine.obj"
	-@erase "$(INTDIR)\blackswaptionengine.sbr"
	-@erase "$(INTDIR)\blackvariancecurve.obj"
	-@erase "$(INTDIR)\blackvariancecurve.sbr"
	-@erase "$(INTDIR)\blackvariancesurface.obj"
	-@erase "$(INTDIR)\blackvariancesurface.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
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
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\choleskydecomposition.obj"
	-@erase "$(INTDIR)\choleskydecomposition.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\copenhagen.obj"
	-@erase "$(INTDIR)\copenhagen.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discrepancystatistics.obj"
	-@erase "$(INTDIR)\discrepancystatistics.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedasset.obj"
	-@erase "$(INTDIR)\discretizedasset.sbr"
	-@erase "$(INTDIR)\discretizedcapfloor.obj"
	-@erase "$(INTDIR)\discretizedcapfloor.sbr"
	-@erase "$(INTDIR)\discretizedswaption.obj"
	-@erase "$(INTDIR)\discretizedswaption.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendvanillaoption.obj"
	-@erase "$(INTDIR)\dividendvanillaoption.sbr"
	-@erase "$(INTDIR)\errorfunction.obj"
	-@erase "$(INTDIR)\errorfunction.sbr"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\errors.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\exercise.obj"
	-@erase "$(INTDIR)\exercise.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\extendeddiscountcurve.obj"
	-@erase "$(INTDIR)\extendeddiscountcurve.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
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
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\g2swaptionengine.obj"
	-@erase "$(INTDIR)\g2swaptionengine.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\generalstatistics.obj"
	-@erase "$(INTDIR)\generalstatistics.sbr"
	-@erase "$(INTDIR)\germany.obj"
	-@erase "$(INTDIR)\germany.sbr"
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
	-@erase "$(INTDIR)\haltonrsg.obj"
	-@erase "$(INTDIR)\haltonrsg.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\hongkong.obj"
	-@erase "$(INTDIR)\hongkong.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\incompletegamma.obj"
	-@erase "$(INTDIR)\incompletegamma.sbr"
	-@erase "$(INTDIR)\incrementalstatistics.obj"
	-@erase "$(INTDIR)\incrementalstatistics.sbr"
	-@erase "$(INTDIR)\integralengine.obj"
	-@erase "$(INTDIR)\integralengine.sbr"
	-@erase "$(INTDIR)\italy.obj"
	-@erase "$(INTDIR)\italy.sbr"
	-@erase "$(INTDIR)\jamshidianswaptionengine.obj"
	-@erase "$(INTDIR)\jamshidianswaptionengine.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
	-@erase "$(INTDIR)\jumpdiffusionengine.obj"
	-@erase "$(INTDIR)\jumpdiffusionengine.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\localvolsurface.obj"
	-@erase "$(INTDIR)\localvolsurface.sbr"
	-@erase "$(INTDIR)\mcamericanbasketengine.obj"
	-@erase "$(INTDIR)\mcamericanbasketengine.sbr"
	-@erase "$(INTDIR)\mcbarrierengine.obj"
	-@erase "$(INTDIR)\mcbarrierengine.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mcbasketengine.obj"
	-@erase "$(INTDIR)\mcbasketengine.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdigitalengine.obj"
	-@erase "$(INTDIR)\mcdigitalengine.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
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
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\mt19937uniformrng.obj"
	-@erase "$(INTDIR)\mt19937uniformrng.sbr"
	-@erase "$(INTDIR)\multiassetoption.obj"
	-@erase "$(INTDIR)\multiassetoption.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\oneassetoption.obj"
	-@erase "$(INTDIR)\oneassetoption.sbr"
	-@erase "$(INTDIR)\oneassetstrikedoption.obj"
	-@erase "$(INTDIR)\oneassetstrikedoption.sbr"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\primenumbers.obj"
	-@erase "$(INTDIR)\primenumbers.sbr"
	-@erase "$(INTDIR)\primitivepolynomials.obj"
	-@erase "$(INTDIR)\primitivepolynomials.sbr"
	-@erase "$(INTDIR)\pseudosqrt.obj"
	-@erase "$(INTDIR)\pseudosqrt.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\riyadh.obj"
	-@erase "$(INTDIR)\riyadh.sbr"
	-@erase "$(INTDIR)\rounding.obj"
	-@erase "$(INTDIR)\rounding.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\seoul.obj"
	-@erase "$(INTDIR)\seoul.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpledaycounter.obj"
	-@erase "$(INTDIR)\simpledaycounter.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singapore.obj"
	-@erase "$(INTDIR)\singapore.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\sobolrsg.obj"
	-@erase "$(INTDIR)\sobolrsg.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stochasticprocess.obj"
	-@erase "$(INTDIR)\stochasticprocess.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
	-@erase "$(INTDIR)\stulzengine.obj"
	-@erase "$(INTDIR)\stulzengine.sbr"
	-@erase "$(INTDIR)\svd.obj"
	-@erase "$(INTDIR)\svd.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\taiwan.obj"
	-@erase "$(INTDIR)\taiwan.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\timebasket.obj"
	-@erase "$(INTDIR)\timebasket.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloorengine.obj"
	-@erase "$(INTDIR)\treecapfloorengine.sbr"
	-@erase "$(INTDIR)\treeswaptionengine.obj"
	-@erase "$(INTDIR)\treeswaptionengine.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\unitedkingdom.obj"
	-@erase "$(INTDIR)\unitedkingdom.sbr"
	-@erase "$(INTDIR)\unitedstates.obj"
	-@erase "$(INTDIR)\unitedstates.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
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
	-@erase ".\lib\QuantLib-vc6-mt-gd-0_3_7.idb"
	-@erase ".\lib\QuantLib-vc6-mt-gd-0_3_7.lib"
	-@erase ".\lib\QuantLib-vc6-mt-gd-0_3_7.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd".\lib\QuantLib-vc6-mt-gd-0_3_7" /FD /GZ /c 

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
	"$(INTDIR)\beijing.sbr" \
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\copenhagen.sbr" \
	"$(INTDIR)\germany.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\hongkong.sbr" \
	"$(INTDIR)\italy.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\riyadh.sbr" \
	"$(INTDIR)\seoul.sbr" \
	"$(INTDIR)\singapore.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\taiwan.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\unitedkingdom.sbr" \
	"$(INTDIR)\unitedstates.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\basispointsensitivity.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\timebasket.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\simpledaycounter.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\asianoption.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendvanillaoption.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\multiassetoption.sbr" \
	"$(INTDIR)\oneassetoption.sbr" \
	"$(INTDIR)\oneassetstrikedoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\beta.sbr" \
	"$(INTDIR)\bivariatenormaldistribution.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\choleskydecomposition.sbr" \
	"$(INTDIR)\discrepancystatistics.sbr" \
	"$(INTDIR)\errorfunction.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\generalstatistics.sbr" \
	"$(INTDIR)\incompletegamma.sbr" \
	"$(INTDIR)\incrementalstatistics.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\primenumbers.sbr" \
	"$(INTDIR)\pseudosqrt.sbr" \
	"$(INTDIR)\rounding.sbr" \
	"$(INTDIR)\svd.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\cliquetoptionpricer.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\analyticcontinuousasianengine.sbr" \
	"$(INTDIR)\analyticdiscreteasianengine.sbr" \
	"$(INTDIR)\analyticbarrierengine.sbr" \
	"$(INTDIR)\mcbarrierengine.sbr" \
	"$(INTDIR)\mcamericanbasketengine.sbr" \
	"$(INTDIR)\mcbasketengine.sbr" \
	"$(INTDIR)\stulzengine.sbr" \
	"$(INTDIR)\analyticdigitalamericanengine.sbr" \
	"$(INTDIR)\analyticdividendeuropeanengine.sbr" \
	"$(INTDIR)\analyticeuropeanengine.sbr" \
	"$(INTDIR)\baroneadesiwhaleyengine.sbr" \
	"$(INTDIR)\bjerksundstenslandengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\integralengine.sbr" \
	"$(INTDIR)\jumpdiffusionengine.sbr" \
	"$(INTDIR)\mcdigitalengine.sbr" \
	"$(INTDIR)\analyticcapfloorengine.sbr" \
	"$(INTDIR)\blackcapfloorengine.sbr" \
	"$(INTDIR)\discretizedcapfloor.sbr" \
	"$(INTDIR)\treecapfloorengine.sbr" \
	"$(INTDIR)\blackswaptionengine.sbr" \
	"$(INTDIR)\discretizedswaption.sbr" \
	"$(INTDIR)\jamshidianswaptionengine.sbr" \
	"$(INTDIR)\treeswaptionengine.sbr" \
	"$(INTDIR)\analyticcliquetengine.sbr" \
	"$(INTDIR)\analyticperformanceengine.sbr" \
	"$(INTDIR)\americanpayoffatexpiry.sbr" \
	"$(INTDIR)\americanpayoffathit.sbr" \
	"$(INTDIR)\blackformula.sbr" \
	"$(INTDIR)\haltonrsg.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\mt19937uniformrng.sbr" \
	"$(INTDIR)\primitivepolynomials.sbr" \
	"$(INTDIR)\sobolrsg.sbr" \
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
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\extendeddiscountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\zerocurve.sbr" \
	"$(INTDIR)\blackvariancecurve.sbr" \
	"$(INTDIR)\blackvariancesurface.sbr" \
	"$(INTDIR)\localvolsurface.sbr" \
	"$(INTDIR)\basicdataformatters.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\discretizedasset.sbr" \
	"$(INTDIR)\errors.sbr" \
	"$(INTDIR)\exercise.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\stochasticprocess.sbr" \
	"$(INTDIR)\voltermstructure.sbr" \
	"$(INTDIR)\g2swaptionengine.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\QuantLib-vc6-mt-gd-0_3_7.lib" 
LIB32_OBJS= \
	"$(INTDIR)\beijing.obj" \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\copenhagen.obj" \
	"$(INTDIR)\germany.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\hongkong.obj" \
	"$(INTDIR)\italy.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\riyadh.obj" \
	"$(INTDIR)\seoul.obj" \
	"$(INTDIR)\singapore.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\taiwan.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\unitedkingdom.obj" \
	"$(INTDIR)\unitedstates.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\basispointsensitivity.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\timebasket.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\simpledaycounter.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\asianoption.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendvanillaoption.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\multiassetoption.obj" \
	"$(INTDIR)\oneassetoption.obj" \
	"$(INTDIR)\oneassetstrikedoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\beta.obj" \
	"$(INTDIR)\bivariatenormaldistribution.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\choleskydecomposition.obj" \
	"$(INTDIR)\discrepancystatistics.obj" \
	"$(INTDIR)\errorfunction.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\generalstatistics.obj" \
	"$(INTDIR)\incompletegamma.obj" \
	"$(INTDIR)\incrementalstatistics.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\primenumbers.obj" \
	"$(INTDIR)\pseudosqrt.obj" \
	"$(INTDIR)\rounding.obj" \
	"$(INTDIR)\svd.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\cliquetoptionpricer.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\analyticcontinuousasianengine.obj" \
	"$(INTDIR)\analyticdiscreteasianengine.obj" \
	"$(INTDIR)\analyticbarrierengine.obj" \
	"$(INTDIR)\mcbarrierengine.obj" \
	"$(INTDIR)\mcamericanbasketengine.obj" \
	"$(INTDIR)\mcbasketengine.obj" \
	"$(INTDIR)\stulzengine.obj" \
	"$(INTDIR)\analyticdigitalamericanengine.obj" \
	"$(INTDIR)\analyticdividendeuropeanengine.obj" \
	"$(INTDIR)\analyticeuropeanengine.obj" \
	"$(INTDIR)\baroneadesiwhaleyengine.obj" \
	"$(INTDIR)\bjerksundstenslandengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\integralengine.obj" \
	"$(INTDIR)\jumpdiffusionengine.obj" \
	"$(INTDIR)\mcdigitalengine.obj" \
	"$(INTDIR)\analyticcapfloorengine.obj" \
	"$(INTDIR)\blackcapfloorengine.obj" \
	"$(INTDIR)\discretizedcapfloor.obj" \
	"$(INTDIR)\treecapfloorengine.obj" \
	"$(INTDIR)\blackswaptionengine.obj" \
	"$(INTDIR)\discretizedswaption.obj" \
	"$(INTDIR)\jamshidianswaptionengine.obj" \
	"$(INTDIR)\treeswaptionengine.obj" \
	"$(INTDIR)\analyticcliquetengine.obj" \
	"$(INTDIR)\analyticperformanceengine.obj" \
	"$(INTDIR)\americanpayoffatexpiry.obj" \
	"$(INTDIR)\americanpayoffathit.obj" \
	"$(INTDIR)\blackformula.obj" \
	"$(INTDIR)\haltonrsg.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\mt19937uniformrng.obj" \
	"$(INTDIR)\primitivepolynomials.obj" \
	"$(INTDIR)\sobolrsg.obj" \
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
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\extendeddiscountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\zerocurve.obj" \
	"$(INTDIR)\blackvariancecurve.obj" \
	"$(INTDIR)\blackvariancesurface.obj" \
	"$(INTDIR)\localvolsurface.obj" \
	"$(INTDIR)\basicdataformatters.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\discretizedasset.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\exercise.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\stochasticprocess.obj" \
	"$(INTDIR)\voltermstructure.obj" \
	"$(INTDIR)\g2swaptionengine.obj" \
	".\lib\QuantLibFunctions-vc6-mt-gd-0_3_7.lib"

".\lib\QuantLib-vc6-mt-gd-0_3_7.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
   if not exist lib mkdir lib
	 $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Release SingleThread"

OUTDIR=.\build\ReleaseST
INTDIR=.\build\ReleaseST
# Begin Custom Macros
OutDir=.\build\ReleaseST
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\QuantLib-vc6-s-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : "QuantLibFunctions - Win32 Release SingleThread" ".\lib\QuantLib-vc6-s-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLibFunctions - Win32 Release SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\americanpayoffatexpiry.obj"
	-@erase "$(INTDIR)\americanpayoffatexpiry.sbr"
	-@erase "$(INTDIR)\americanpayoffathit.obj"
	-@erase "$(INTDIR)\americanpayoffathit.sbr"
	-@erase "$(INTDIR)\analyticbarrierengine.obj"
	-@erase "$(INTDIR)\analyticbarrierengine.sbr"
	-@erase "$(INTDIR)\analyticcapfloorengine.obj"
	-@erase "$(INTDIR)\analyticcapfloorengine.sbr"
	-@erase "$(INTDIR)\analyticcliquetengine.obj"
	-@erase "$(INTDIR)\analyticcliquetengine.sbr"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.obj"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.sbr"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.obj"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.sbr"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.obj"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.sbr"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticperformanceengine.obj"
	-@erase "$(INTDIR)\analyticperformanceengine.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\asianoption.obj"
	-@erase "$(INTDIR)\asianoption.sbr"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.obj"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basicdataformatters.obj"
	-@erase "$(INTDIR)\basicdataformatters.sbr"
	-@erase "$(INTDIR)\basispointsensitivity.obj"
	-@erase "$(INTDIR)\basispointsensitivity.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\beijing.obj"
	-@erase "$(INTDIR)\beijing.sbr"
	-@erase "$(INTDIR)\beta.obj"
	-@erase "$(INTDIR)\beta.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bivariatenormaldistribution.obj"
	-@erase "$(INTDIR)\bivariatenormaldistribution.sbr"
	-@erase "$(INTDIR)\bjerksundstenslandengine.obj"
	-@erase "$(INTDIR)\bjerksundstenslandengine.sbr"
	-@erase "$(INTDIR)\blackcapfloorengine.obj"
	-@erase "$(INTDIR)\blackcapfloorengine.sbr"
	-@erase "$(INTDIR)\blackformula.obj"
	-@erase "$(INTDIR)\blackformula.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaptionengine.obj"
	-@erase "$(INTDIR)\blackswaptionengine.sbr"
	-@erase "$(INTDIR)\blackvariancecurve.obj"
	-@erase "$(INTDIR)\blackvariancecurve.sbr"
	-@erase "$(INTDIR)\blackvariancesurface.obj"
	-@erase "$(INTDIR)\blackvariancesurface.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
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
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\choleskydecomposition.obj"
	-@erase "$(INTDIR)\choleskydecomposition.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\copenhagen.obj"
	-@erase "$(INTDIR)\copenhagen.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discrepancystatistics.obj"
	-@erase "$(INTDIR)\discrepancystatistics.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedasset.obj"
	-@erase "$(INTDIR)\discretizedasset.sbr"
	-@erase "$(INTDIR)\discretizedcapfloor.obj"
	-@erase "$(INTDIR)\discretizedcapfloor.sbr"
	-@erase "$(INTDIR)\discretizedswaption.obj"
	-@erase "$(INTDIR)\discretizedswaption.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendvanillaoption.obj"
	-@erase "$(INTDIR)\dividendvanillaoption.sbr"
	-@erase "$(INTDIR)\errorfunction.obj"
	-@erase "$(INTDIR)\errorfunction.sbr"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\errors.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\exercise.obj"
	-@erase "$(INTDIR)\exercise.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\extendeddiscountcurve.obj"
	-@erase "$(INTDIR)\extendeddiscountcurve.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
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
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\g2swaptionengine.obj"
	-@erase "$(INTDIR)\g2swaptionengine.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\generalstatistics.obj"
	-@erase "$(INTDIR)\generalstatistics.sbr"
	-@erase "$(INTDIR)\germany.obj"
	-@erase "$(INTDIR)\germany.sbr"
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
	-@erase "$(INTDIR)\haltonrsg.obj"
	-@erase "$(INTDIR)\haltonrsg.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\hongkong.obj"
	-@erase "$(INTDIR)\hongkong.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\incompletegamma.obj"
	-@erase "$(INTDIR)\incompletegamma.sbr"
	-@erase "$(INTDIR)\incrementalstatistics.obj"
	-@erase "$(INTDIR)\incrementalstatistics.sbr"
	-@erase "$(INTDIR)\integralengine.obj"
	-@erase "$(INTDIR)\integralengine.sbr"
	-@erase "$(INTDIR)\italy.obj"
	-@erase "$(INTDIR)\italy.sbr"
	-@erase "$(INTDIR)\jamshidianswaptionengine.obj"
	-@erase "$(INTDIR)\jamshidianswaptionengine.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
	-@erase "$(INTDIR)\jumpdiffusionengine.obj"
	-@erase "$(INTDIR)\jumpdiffusionengine.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\localvolsurface.obj"
	-@erase "$(INTDIR)\localvolsurface.sbr"
	-@erase "$(INTDIR)\mcamericanbasketengine.obj"
	-@erase "$(INTDIR)\mcamericanbasketengine.sbr"
	-@erase "$(INTDIR)\mcbarrierengine.obj"
	-@erase "$(INTDIR)\mcbarrierengine.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mcbasketengine.obj"
	-@erase "$(INTDIR)\mcbasketengine.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdigitalengine.obj"
	-@erase "$(INTDIR)\mcdigitalengine.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
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
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\mt19937uniformrng.obj"
	-@erase "$(INTDIR)\mt19937uniformrng.sbr"
	-@erase "$(INTDIR)\multiassetoption.obj"
	-@erase "$(INTDIR)\multiassetoption.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\oneassetoption.obj"
	-@erase "$(INTDIR)\oneassetoption.sbr"
	-@erase "$(INTDIR)\oneassetstrikedoption.obj"
	-@erase "$(INTDIR)\oneassetstrikedoption.sbr"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\primenumbers.obj"
	-@erase "$(INTDIR)\primenumbers.sbr"
	-@erase "$(INTDIR)\primitivepolynomials.obj"
	-@erase "$(INTDIR)\primitivepolynomials.sbr"
	-@erase "$(INTDIR)\pseudosqrt.obj"
	-@erase "$(INTDIR)\pseudosqrt.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\riyadh.obj"
	-@erase "$(INTDIR)\riyadh.sbr"
	-@erase "$(INTDIR)\rounding.obj"
	-@erase "$(INTDIR)\rounding.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\seoul.obj"
	-@erase "$(INTDIR)\seoul.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpledaycounter.obj"
	-@erase "$(INTDIR)\simpledaycounter.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singapore.obj"
	-@erase "$(INTDIR)\singapore.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\sobolrsg.obj"
	-@erase "$(INTDIR)\sobolrsg.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stochasticprocess.obj"
	-@erase "$(INTDIR)\stochasticprocess.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
	-@erase "$(INTDIR)\stulzengine.obj"
	-@erase "$(INTDIR)\stulzengine.sbr"
	-@erase "$(INTDIR)\svd.obj"
	-@erase "$(INTDIR)\svd.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\taiwan.obj"
	-@erase "$(INTDIR)\taiwan.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\timebasket.obj"
	-@erase "$(INTDIR)\timebasket.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloorengine.obj"
	-@erase "$(INTDIR)\treecapfloorengine.sbr"
	-@erase "$(INTDIR)\treeswaptionengine.obj"
	-@erase "$(INTDIR)\treeswaptionengine.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\unitedkingdom.obj"
	-@erase "$(INTDIR)\unitedkingdom.sbr"
	-@erase "$(INTDIR)\unitedstates.obj"
	-@erase "$(INTDIR)\unitedstates.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
	-@erase "$(INTDIR)\vc60.idb"
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
	-@erase ".\lib\QuantLib-vc6-s-0_3_7.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /W3 /Gi /GR /GX /O2 /Ob2 /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Oi- /c 

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
	"$(INTDIR)\beijing.sbr" \
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\copenhagen.sbr" \
	"$(INTDIR)\germany.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\hongkong.sbr" \
	"$(INTDIR)\italy.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\riyadh.sbr" \
	"$(INTDIR)\seoul.sbr" \
	"$(INTDIR)\singapore.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\taiwan.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\unitedkingdom.sbr" \
	"$(INTDIR)\unitedstates.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\basispointsensitivity.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\timebasket.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\simpledaycounter.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\asianoption.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendvanillaoption.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\multiassetoption.sbr" \
	"$(INTDIR)\oneassetoption.sbr" \
	"$(INTDIR)\oneassetstrikedoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\beta.sbr" \
	"$(INTDIR)\bivariatenormaldistribution.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\choleskydecomposition.sbr" \
	"$(INTDIR)\discrepancystatistics.sbr" \
	"$(INTDIR)\errorfunction.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\generalstatistics.sbr" \
	"$(INTDIR)\incompletegamma.sbr" \
	"$(INTDIR)\incrementalstatistics.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\primenumbers.sbr" \
	"$(INTDIR)\pseudosqrt.sbr" \
	"$(INTDIR)\rounding.sbr" \
	"$(INTDIR)\svd.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\cliquetoptionpricer.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\analyticcontinuousasianengine.sbr" \
	"$(INTDIR)\analyticdiscreteasianengine.sbr" \
	"$(INTDIR)\analyticbarrierengine.sbr" \
	"$(INTDIR)\mcbarrierengine.sbr" \
	"$(INTDIR)\mcamericanbasketengine.sbr" \
	"$(INTDIR)\mcbasketengine.sbr" \
	"$(INTDIR)\stulzengine.sbr" \
	"$(INTDIR)\analyticdigitalamericanengine.sbr" \
	"$(INTDIR)\analyticdividendeuropeanengine.sbr" \
	"$(INTDIR)\analyticeuropeanengine.sbr" \
	"$(INTDIR)\baroneadesiwhaleyengine.sbr" \
	"$(INTDIR)\bjerksundstenslandengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\integralengine.sbr" \
	"$(INTDIR)\jumpdiffusionengine.sbr" \
	"$(INTDIR)\mcdigitalengine.sbr" \
	"$(INTDIR)\analyticcapfloorengine.sbr" \
	"$(INTDIR)\blackcapfloorengine.sbr" \
	"$(INTDIR)\discretizedcapfloor.sbr" \
	"$(INTDIR)\treecapfloorengine.sbr" \
	"$(INTDIR)\blackswaptionengine.sbr" \
	"$(INTDIR)\discretizedswaption.sbr" \
	"$(INTDIR)\jamshidianswaptionengine.sbr" \
	"$(INTDIR)\treeswaptionengine.sbr" \
	"$(INTDIR)\analyticcliquetengine.sbr" \
	"$(INTDIR)\analyticperformanceengine.sbr" \
	"$(INTDIR)\americanpayoffatexpiry.sbr" \
	"$(INTDIR)\americanpayoffathit.sbr" \
	"$(INTDIR)\blackformula.sbr" \
	"$(INTDIR)\haltonrsg.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\mt19937uniformrng.sbr" \
	"$(INTDIR)\primitivepolynomials.sbr" \
	"$(INTDIR)\sobolrsg.sbr" \
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
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\extendeddiscountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\zerocurve.sbr" \
	"$(INTDIR)\blackvariancecurve.sbr" \
	"$(INTDIR)\blackvariancesurface.sbr" \
	"$(INTDIR)\localvolsurface.sbr" \
	"$(INTDIR)\basicdataformatters.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\discretizedasset.sbr" \
	"$(INTDIR)\errors.sbr" \
	"$(INTDIR)\exercise.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\stochasticprocess.sbr" \
	"$(INTDIR)\voltermstructure.sbr" \
	"$(INTDIR)\g2swaptionengine.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\QuantLib-vc6-s-0_3_7.lib" 
LIB32_OBJS= \
	"$(INTDIR)\beijing.obj" \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\copenhagen.obj" \
	"$(INTDIR)\germany.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\hongkong.obj" \
	"$(INTDIR)\italy.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\riyadh.obj" \
	"$(INTDIR)\seoul.obj" \
	"$(INTDIR)\singapore.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\taiwan.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\unitedkingdom.obj" \
	"$(INTDIR)\unitedstates.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\basispointsensitivity.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\timebasket.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\simpledaycounter.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\asianoption.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendvanillaoption.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\multiassetoption.obj" \
	"$(INTDIR)\oneassetoption.obj" \
	"$(INTDIR)\oneassetstrikedoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\beta.obj" \
	"$(INTDIR)\bivariatenormaldistribution.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\choleskydecomposition.obj" \
	"$(INTDIR)\discrepancystatistics.obj" \
	"$(INTDIR)\errorfunction.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\generalstatistics.obj" \
	"$(INTDIR)\incompletegamma.obj" \
	"$(INTDIR)\incrementalstatistics.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\primenumbers.obj" \
	"$(INTDIR)\pseudosqrt.obj" \
	"$(INTDIR)\rounding.obj" \
	"$(INTDIR)\svd.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\cliquetoptionpricer.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\analyticcontinuousasianengine.obj" \
	"$(INTDIR)\analyticdiscreteasianengine.obj" \
	"$(INTDIR)\analyticbarrierengine.obj" \
	"$(INTDIR)\mcbarrierengine.obj" \
	"$(INTDIR)\mcamericanbasketengine.obj" \
	"$(INTDIR)\mcbasketengine.obj" \
	"$(INTDIR)\stulzengine.obj" \
	"$(INTDIR)\analyticdigitalamericanengine.obj" \
	"$(INTDIR)\analyticdividendeuropeanengine.obj" \
	"$(INTDIR)\analyticeuropeanengine.obj" \
	"$(INTDIR)\baroneadesiwhaleyengine.obj" \
	"$(INTDIR)\bjerksundstenslandengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\integralengine.obj" \
	"$(INTDIR)\jumpdiffusionengine.obj" \
	"$(INTDIR)\mcdigitalengine.obj" \
	"$(INTDIR)\analyticcapfloorengine.obj" \
	"$(INTDIR)\blackcapfloorengine.obj" \
	"$(INTDIR)\discretizedcapfloor.obj" \
	"$(INTDIR)\treecapfloorengine.obj" \
	"$(INTDIR)\blackswaptionengine.obj" \
	"$(INTDIR)\discretizedswaption.obj" \
	"$(INTDIR)\jamshidianswaptionengine.obj" \
	"$(INTDIR)\treeswaptionengine.obj" \
	"$(INTDIR)\analyticcliquetengine.obj" \
	"$(INTDIR)\analyticperformanceengine.obj" \
	"$(INTDIR)\americanpayoffatexpiry.obj" \
	"$(INTDIR)\americanpayoffathit.obj" \
	"$(INTDIR)\blackformula.obj" \
	"$(INTDIR)\haltonrsg.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\mt19937uniformrng.obj" \
	"$(INTDIR)\primitivepolynomials.obj" \
	"$(INTDIR)\sobolrsg.obj" \
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
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\extendeddiscountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\zerocurve.obj" \
	"$(INTDIR)\blackvariancecurve.obj" \
	"$(INTDIR)\blackvariancesurface.obj" \
	"$(INTDIR)\localvolsurface.obj" \
	"$(INTDIR)\basicdataformatters.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\discretizedasset.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\exercise.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\stochasticprocess.obj" \
	"$(INTDIR)\voltermstructure.obj" \
	"$(INTDIR)\g2swaptionengine.obj" \
	".\lib\QuantLibFunctions-vc6-s-0_3_7.lib"

".\lib\QuantLib-vc6-s-0_3_7.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
   if not exist lib mkdir lib
	 $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug SingleThread"

OUTDIR=.\build\DebugST
INTDIR=.\build\DebugST
# Begin Custom Macros
OutDir=.\build\DebugST
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : ".\lib\QuantLib-vc6-sgd-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ELSE 

ALL : "QuantLibFunctions - Win32 Debug SingleThread" ".\lib\QuantLib-vc6-sgd-0_3_7.lib" "$(OUTDIR)\QuantLib.bsc"

!ENDIF 

!IF "$(RECURSE)" == "1" 
CLEAN :"QuantLibFunctions - Win32 Debug SingleThreadCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\actualactual.obj"
	-@erase "$(INTDIR)\actualactual.sbr"
	-@erase "$(INTDIR)\affinetermstructure.obj"
	-@erase "$(INTDIR)\affinetermstructure.sbr"
	-@erase "$(INTDIR)\americanpayoffatexpiry.obj"
	-@erase "$(INTDIR)\americanpayoffatexpiry.sbr"
	-@erase "$(INTDIR)\americanpayoffathit.obj"
	-@erase "$(INTDIR)\americanpayoffathit.sbr"
	-@erase "$(INTDIR)\analyticbarrierengine.obj"
	-@erase "$(INTDIR)\analyticbarrierengine.sbr"
	-@erase "$(INTDIR)\analyticcapfloorengine.obj"
	-@erase "$(INTDIR)\analyticcapfloorengine.sbr"
	-@erase "$(INTDIR)\analyticcliquetengine.obj"
	-@erase "$(INTDIR)\analyticcliquetengine.sbr"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.obj"
	-@erase "$(INTDIR)\analyticcontinuousasianengine.sbr"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.obj"
	-@erase "$(INTDIR)\analyticdigitalamericanengine.sbr"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.obj"
	-@erase "$(INTDIR)\analyticdiscreteasianengine.sbr"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticdividendeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticeuropeanengine.obj"
	-@erase "$(INTDIR)\analyticeuropeanengine.sbr"
	-@erase "$(INTDIR)\analyticperformanceengine.obj"
	-@erase "$(INTDIR)\analyticperformanceengine.sbr"
	-@erase "$(INTDIR)\armijo.obj"
	-@erase "$(INTDIR)\armijo.sbr"
	-@erase "$(INTDIR)\asianoption.obj"
	-@erase "$(INTDIR)\asianoption.sbr"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.obj"
	-@erase "$(INTDIR)\baroneadesiwhaleyengine.sbr"
	-@erase "$(INTDIR)\barrieroption.obj"
	-@erase "$(INTDIR)\barrieroption.sbr"
	-@erase "$(INTDIR)\basicdataformatters.obj"
	-@erase "$(INTDIR)\basicdataformatters.sbr"
	-@erase "$(INTDIR)\basispointsensitivity.obj"
	-@erase "$(INTDIR)\basispointsensitivity.sbr"
	-@erase "$(INTDIR)\basketoption.obj"
	-@erase "$(INTDIR)\basketoption.sbr"
	-@erase "$(INTDIR)\beijing.obj"
	-@erase "$(INTDIR)\beijing.sbr"
	-@erase "$(INTDIR)\beta.obj"
	-@erase "$(INTDIR)\beta.sbr"
	-@erase "$(INTDIR)\binomialtree.obj"
	-@erase "$(INTDIR)\binomialtree.sbr"
	-@erase "$(INTDIR)\bivariatenormaldistribution.obj"
	-@erase "$(INTDIR)\bivariatenormaldistribution.sbr"
	-@erase "$(INTDIR)\bjerksundstenslandengine.obj"
	-@erase "$(INTDIR)\bjerksundstenslandengine.sbr"
	-@erase "$(INTDIR)\blackcapfloorengine.obj"
	-@erase "$(INTDIR)\blackcapfloorengine.sbr"
	-@erase "$(INTDIR)\blackformula.obj"
	-@erase "$(INTDIR)\blackformula.sbr"
	-@erase "$(INTDIR)\blackkarasinski.obj"
	-@erase "$(INTDIR)\blackkarasinski.sbr"
	-@erase "$(INTDIR)\blackswaptionengine.obj"
	-@erase "$(INTDIR)\blackswaptionengine.sbr"
	-@erase "$(INTDIR)\blackvariancecurve.obj"
	-@erase "$(INTDIR)\blackvariancecurve.sbr"
	-@erase "$(INTDIR)\blackvariancesurface.obj"
	-@erase "$(INTDIR)\blackvariancesurface.sbr"
	-@erase "$(INTDIR)\boundarycondition.obj"
	-@erase "$(INTDIR)\boundarycondition.sbr"
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
	-@erase "$(INTDIR)\caphelper.obj"
	-@erase "$(INTDIR)\caphelper.sbr"
	-@erase "$(INTDIR)\cashflowvectors.obj"
	-@erase "$(INTDIR)\cashflowvectors.sbr"
	-@erase "$(INTDIR)\chisquaredistribution.obj"
	-@erase "$(INTDIR)\chisquaredistribution.sbr"
	-@erase "$(INTDIR)\choleskydecomposition.obj"
	-@erase "$(INTDIR)\choleskydecomposition.sbr"
	-@erase "$(INTDIR)\cliquetoption.obj"
	-@erase "$(INTDIR)\cliquetoption.sbr"
	-@erase "$(INTDIR)\cliquetoptionpricer.obj"
	-@erase "$(INTDIR)\cliquetoptionpricer.sbr"
	-@erase "$(INTDIR)\compoundforward.obj"
	-@erase "$(INTDIR)\compoundforward.sbr"
	-@erase "$(INTDIR)\conjugategradient.obj"
	-@erase "$(INTDIR)\conjugategradient.sbr"
	-@erase "$(INTDIR)\copenhagen.obj"
	-@erase "$(INTDIR)\copenhagen.sbr"
	-@erase "$(INTDIR)\coxingersollross.obj"
	-@erase "$(INTDIR)\coxingersollross.sbr"
	-@erase "$(INTDIR)\dataformatters.obj"
	-@erase "$(INTDIR)\dataformatters.sbr"
	-@erase "$(INTDIR)\dataparsers.obj"
	-@erase "$(INTDIR)\dataparsers.sbr"
	-@erase "$(INTDIR)\date.obj"
	-@erase "$(INTDIR)\date.sbr"
	-@erase "$(INTDIR)\discountcurve.obj"
	-@erase "$(INTDIR)\discountcurve.sbr"
	-@erase "$(INTDIR)\discrepancystatistics.obj"
	-@erase "$(INTDIR)\discrepancystatistics.sbr"
	-@erase "$(INTDIR)\discretegeometricapo.obj"
	-@erase "$(INTDIR)\discretegeometricapo.sbr"
	-@erase "$(INTDIR)\discretegeometricaso.obj"
	-@erase "$(INTDIR)\discretegeometricaso.sbr"
	-@erase "$(INTDIR)\discretizedasset.obj"
	-@erase "$(INTDIR)\discretizedasset.sbr"
	-@erase "$(INTDIR)\discretizedcapfloor.obj"
	-@erase "$(INTDIR)\discretizedcapfloor.sbr"
	-@erase "$(INTDIR)\discretizedswaption.obj"
	-@erase "$(INTDIR)\discretizedswaption.sbr"
	-@erase "$(INTDIR)\discretizedvanillaoption.obj"
	-@erase "$(INTDIR)\discretizedvanillaoption.sbr"
	-@erase "$(INTDIR)\dividendeuropeanoption.obj"
	-@erase "$(INTDIR)\dividendeuropeanoption.sbr"
	-@erase "$(INTDIR)\dividendvanillaoption.obj"
	-@erase "$(INTDIR)\dividendvanillaoption.sbr"
	-@erase "$(INTDIR)\errorfunction.obj"
	-@erase "$(INTDIR)\errorfunction.sbr"
	-@erase "$(INTDIR)\errors.obj"
	-@erase "$(INTDIR)\errors.sbr"
	-@erase "$(INTDIR)\europeanoption.obj"
	-@erase "$(INTDIR)\europeanoption.sbr"
	-@erase "$(INTDIR)\exercise.obj"
	-@erase "$(INTDIR)\exercise.sbr"
	-@erase "$(INTDIR)\extendedcoxingersollross.obj"
	-@erase "$(INTDIR)\extendedcoxingersollross.sbr"
	-@erase "$(INTDIR)\extendeddiscountcurve.obj"
	-@erase "$(INTDIR)\extendeddiscountcurve.sbr"
	-@erase "$(INTDIR)\factorial.obj"
	-@erase "$(INTDIR)\factorial.sbr"
	-@erase "$(INTDIR)\fdbermudanoption.obj"
	-@erase "$(INTDIR)\fdbermudanoption.sbr"
	-@erase "$(INTDIR)\fdbsmoption.obj"
	-@erase "$(INTDIR)\fdbsmoption.sbr"
	-@erase "$(INTDIR)\fddividendamericanoption.obj"
	-@erase "$(INTDIR)\fddividendamericanoption.sbr"
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
	-@erase "$(INTDIR)\forwardvanillaoption.obj"
	-@erase "$(INTDIR)\forwardvanillaoption.sbr"
	-@erase "$(INTDIR)\g2.obj"
	-@erase "$(INTDIR)\g2.sbr"
	-@erase "$(INTDIR)\g2swaptionengine.obj"
	-@erase "$(INTDIR)\g2swaptionengine.sbr"
	-@erase "$(INTDIR)\gammadistribution.obj"
	-@erase "$(INTDIR)\gammadistribution.sbr"
	-@erase "$(INTDIR)\generalstatistics.obj"
	-@erase "$(INTDIR)\generalstatistics.sbr"
	-@erase "$(INTDIR)\germany.obj"
	-@erase "$(INTDIR)\germany.sbr"
	-@erase "$(INTDIR)\grid.obj"
	-@erase "$(INTDIR)\grid.sbr"
	-@erase "$(INTDIR)\haltonrsg.obj"
	-@erase "$(INTDIR)\haltonrsg.sbr"
	-@erase "$(INTDIR)\helsinki.obj"
	-@erase "$(INTDIR)\helsinki.sbr"
	-@erase "$(INTDIR)\hongkong.obj"
	-@erase "$(INTDIR)\hongkong.sbr"
	-@erase "$(INTDIR)\hullwhite.obj"
	-@erase "$(INTDIR)\hullwhite.sbr"
	-@erase "$(INTDIR)\incompletegamma.obj"
	-@erase "$(INTDIR)\incompletegamma.sbr"
	-@erase "$(INTDIR)\incrementalstatistics.obj"
	-@erase "$(INTDIR)\incrementalstatistics.sbr"
	-@erase "$(INTDIR)\integralengine.obj"
	-@erase "$(INTDIR)\integralengine.sbr"
	-@erase "$(INTDIR)\italy.obj"
	-@erase "$(INTDIR)\italy.sbr"
	-@erase "$(INTDIR)\jamshidianswaptionengine.obj"
	-@erase "$(INTDIR)\jamshidianswaptionengine.sbr"
	-@erase "$(INTDIR)\johannesburg.obj"
	-@erase "$(INTDIR)\johannesburg.sbr"
	-@erase "$(INTDIR)\jointcalendar.obj"
	-@erase "$(INTDIR)\jointcalendar.sbr"
	-@erase "$(INTDIR)\jumpdiffusionengine.obj"
	-@erase "$(INTDIR)\jumpdiffusionengine.sbr"
	-@erase "$(INTDIR)\knuthuniformrng.obj"
	-@erase "$(INTDIR)\knuthuniformrng.sbr"
	-@erase "$(INTDIR)\lattice.obj"
	-@erase "$(INTDIR)\lattice.sbr"
	-@erase "$(INTDIR)\lattice2d.obj"
	-@erase "$(INTDIR)\lattice2d.sbr"
	-@erase "$(INTDIR)\lecuyeruniformrng.obj"
	-@erase "$(INTDIR)\lecuyeruniformrng.sbr"
	-@erase "$(INTDIR)\localvolsurface.obj"
	-@erase "$(INTDIR)\localvolsurface.sbr"
	-@erase "$(INTDIR)\mcamericanbasketengine.obj"
	-@erase "$(INTDIR)\mcamericanbasketengine.sbr"
	-@erase "$(INTDIR)\mcbarrierengine.obj"
	-@erase "$(INTDIR)\mcbarrierengine.sbr"
	-@erase "$(INTDIR)\mcbasket.obj"
	-@erase "$(INTDIR)\mcbasket.sbr"
	-@erase "$(INTDIR)\mcbasketengine.obj"
	-@erase "$(INTDIR)\mcbasketengine.sbr"
	-@erase "$(INTDIR)\mccliquetoption.obj"
	-@erase "$(INTDIR)\mccliquetoption.sbr"
	-@erase "$(INTDIR)\mcdigitalengine.obj"
	-@erase "$(INTDIR)\mcdigitalengine.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticapo.sbr"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.obj"
	-@erase "$(INTDIR)\mcdiscretearithmeticaso.sbr"
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
	-@erase "$(INTDIR)\model.obj"
	-@erase "$(INTDIR)\model.sbr"
	-@erase "$(INTDIR)\mt19937uniformrng.obj"
	-@erase "$(INTDIR)\mt19937uniformrng.sbr"
	-@erase "$(INTDIR)\multiassetoption.obj"
	-@erase "$(INTDIR)\multiassetoption.sbr"
	-@erase "$(INTDIR)\normaldistribution.obj"
	-@erase "$(INTDIR)\normaldistribution.sbr"
	-@erase "$(INTDIR)\oneassetoption.obj"
	-@erase "$(INTDIR)\oneassetoption.sbr"
	-@erase "$(INTDIR)\oneassetstrikedoption.obj"
	-@erase "$(INTDIR)\oneassetstrikedoption.sbr"
	-@erase "$(INTDIR)\onefactormodel.obj"
	-@erase "$(INTDIR)\onefactormodel.sbr"
	-@erase "$(INTDIR)\onefactoroperator.obj"
	-@erase "$(INTDIR)\onefactoroperator.sbr"
	-@erase "$(INTDIR)\oslo.obj"
	-@erase "$(INTDIR)\oslo.sbr"
	-@erase "$(INTDIR)\parcoupon.obj"
	-@erase "$(INTDIR)\parcoupon.sbr"
	-@erase "$(INTDIR)\performanceoption.obj"
	-@erase "$(INTDIR)\performanceoption.sbr"
	-@erase "$(INTDIR)\piecewiseflatforward.obj"
	-@erase "$(INTDIR)\piecewiseflatforward.sbr"
	-@erase "$(INTDIR)\primenumbers.obj"
	-@erase "$(INTDIR)\primenumbers.sbr"
	-@erase "$(INTDIR)\primitivepolynomials.obj"
	-@erase "$(INTDIR)\primitivepolynomials.sbr"
	-@erase "$(INTDIR)\pseudosqrt.obj"
	-@erase "$(INTDIR)\pseudosqrt.sbr"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.obj"
	-@erase "$(INTDIR)\quantoforwardvanillaoption.sbr"
	-@erase "$(INTDIR)\quantovanillaoption.obj"
	-@erase "$(INTDIR)\quantovanillaoption.sbr"
	-@erase "$(INTDIR)\ratehelpers.obj"
	-@erase "$(INTDIR)\ratehelpers.sbr"
	-@erase "$(INTDIR)\riyadh.obj"
	-@erase "$(INTDIR)\riyadh.sbr"
	-@erase "$(INTDIR)\rounding.obj"
	-@erase "$(INTDIR)\rounding.sbr"
	-@erase "$(INTDIR)\scheduler.obj"
	-@erase "$(INTDIR)\scheduler.sbr"
	-@erase "$(INTDIR)\seoul.obj"
	-@erase "$(INTDIR)\seoul.sbr"
	-@erase "$(INTDIR)\shortfloatingcoupon.obj"
	-@erase "$(INTDIR)\shortfloatingcoupon.sbr"
	-@erase "$(INTDIR)\simpledaycounter.obj"
	-@erase "$(INTDIR)\simpledaycounter.sbr"
	-@erase "$(INTDIR)\simpleswap.obj"
	-@erase "$(INTDIR)\simpleswap.sbr"
	-@erase "$(INTDIR)\simplex.obj"
	-@erase "$(INTDIR)\simplex.sbr"
	-@erase "$(INTDIR)\singapore.obj"
	-@erase "$(INTDIR)\singapore.sbr"
	-@erase "$(INTDIR)\singleassetoption.obj"
	-@erase "$(INTDIR)\singleassetoption.sbr"
	-@erase "$(INTDIR)\sobolrsg.obj"
	-@erase "$(INTDIR)\sobolrsg.sbr"
	-@erase "$(INTDIR)\steepestdescent.obj"
	-@erase "$(INTDIR)\steepestdescent.sbr"
	-@erase "$(INTDIR)\stochasticprocess.obj"
	-@erase "$(INTDIR)\stochasticprocess.sbr"
	-@erase "$(INTDIR)\stock.obj"
	-@erase "$(INTDIR)\stock.sbr"
	-@erase "$(INTDIR)\stockholm.obj"
	-@erase "$(INTDIR)\stockholm.sbr"
	-@erase "$(INTDIR)\stulzengine.obj"
	-@erase "$(INTDIR)\stulzengine.sbr"
	-@erase "$(INTDIR)\svd.obj"
	-@erase "$(INTDIR)\svd.sbr"
	-@erase "$(INTDIR)\swap.obj"
	-@erase "$(INTDIR)\swap.sbr"
	-@erase "$(INTDIR)\swaption.obj"
	-@erase "$(INTDIR)\swaption.sbr"
	-@erase "$(INTDIR)\swaptionhelper.obj"
	-@erase "$(INTDIR)\swaptionhelper.sbr"
	-@erase "$(INTDIR)\sydney.obj"
	-@erase "$(INTDIR)\sydney.sbr"
	-@erase "$(INTDIR)\symmetricschurdecomposition.obj"
	-@erase "$(INTDIR)\symmetricschurdecomposition.sbr"
	-@erase "$(INTDIR)\taiwan.obj"
	-@erase "$(INTDIR)\taiwan.sbr"
	-@erase "$(INTDIR)\target.obj"
	-@erase "$(INTDIR)\target.sbr"
	-@erase "$(INTDIR)\thirty360.obj"
	-@erase "$(INTDIR)\thirty360.sbr"
	-@erase "$(INTDIR)\timebasket.obj"
	-@erase "$(INTDIR)\timebasket.sbr"
	-@erase "$(INTDIR)\tokyo.obj"
	-@erase "$(INTDIR)\tokyo.sbr"
	-@erase "$(INTDIR)\toronto.obj"
	-@erase "$(INTDIR)\toronto.sbr"
	-@erase "$(INTDIR)\treecapfloorengine.obj"
	-@erase "$(INTDIR)\treecapfloorengine.sbr"
	-@erase "$(INTDIR)\treeswaptionengine.obj"
	-@erase "$(INTDIR)\treeswaptionengine.sbr"
	-@erase "$(INTDIR)\tridiagonaloperator.obj"
	-@erase "$(INTDIR)\tridiagonaloperator.sbr"
	-@erase "$(INTDIR)\trinomialtree.obj"
	-@erase "$(INTDIR)\trinomialtree.sbr"
	-@erase "$(INTDIR)\twofactormodel.obj"
	-@erase "$(INTDIR)\twofactormodel.sbr"
	-@erase "$(INTDIR)\unitedkingdom.obj"
	-@erase "$(INTDIR)\unitedkingdom.sbr"
	-@erase "$(INTDIR)\unitedstates.obj"
	-@erase "$(INTDIR)\unitedstates.sbr"
	-@erase "$(INTDIR)\valueatcenter.obj"
	-@erase "$(INTDIR)\valueatcenter.sbr"
	-@erase "$(INTDIR)\vanillaoption.obj"
	-@erase "$(INTDIR)\vanillaoption.sbr"
	-@erase "$(INTDIR)\vasicek.obj"
	-@erase "$(INTDIR)\vasicek.sbr"
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
	-@erase ".\lib\QuantLib-vc6-sgd-0_3_7.idb"
	-@erase ".\lib\QuantLib-vc6-sgd-0_3_7.lib"
	-@erase ".\lib\QuantLib-vc6-sgd-0_3_7.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd".\lib\QuantLib-vc6-sgd-0_3_7" /FD /GZ /c 

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
	"$(INTDIR)\beijing.sbr" \
	"$(INTDIR)\budapest.sbr" \
	"$(INTDIR)\copenhagen.sbr" \
	"$(INTDIR)\germany.sbr" \
	"$(INTDIR)\helsinki.sbr" \
	"$(INTDIR)\hongkong.sbr" \
	"$(INTDIR)\italy.sbr" \
	"$(INTDIR)\johannesburg.sbr" \
	"$(INTDIR)\jointcalendar.sbr" \
	"$(INTDIR)\oslo.sbr" \
	"$(INTDIR)\riyadh.sbr" \
	"$(INTDIR)\seoul.sbr" \
	"$(INTDIR)\singapore.sbr" \
	"$(INTDIR)\stockholm.sbr" \
	"$(INTDIR)\sydney.sbr" \
	"$(INTDIR)\taiwan.sbr" \
	"$(INTDIR)\target.sbr" \
	"$(INTDIR)\tokyo.sbr" \
	"$(INTDIR)\toronto.sbr" \
	"$(INTDIR)\unitedkingdom.sbr" \
	"$(INTDIR)\unitedstates.sbr" \
	"$(INTDIR)\warsaw.sbr" \
	"$(INTDIR)\wellington.sbr" \
	"$(INTDIR)\zurich.sbr" \
	"$(INTDIR)\basispointsensitivity.sbr" \
	"$(INTDIR)\cashflowvectors.sbr" \
	"$(INTDIR)\parcoupon.sbr" \
	"$(INTDIR)\shortfloatingcoupon.sbr" \
	"$(INTDIR)\timebasket.sbr" \
	"$(INTDIR)\actualactual.sbr" \
	"$(INTDIR)\simpledaycounter.sbr" \
	"$(INTDIR)\thirty360.sbr" \
	"$(INTDIR)\boundarycondition.sbr" \
	"$(INTDIR)\bsmoperator.sbr" \
	"$(INTDIR)\onefactoroperator.sbr" \
	"$(INTDIR)\tridiagonaloperator.sbr" \
	"$(INTDIR)\valueatcenter.sbr" \
	"$(INTDIR)\xibor.sbr" \
	"$(INTDIR)\xibormanager.sbr" \
	"$(INTDIR)\asianoption.sbr" \
	"$(INTDIR)\barrieroption.sbr" \
	"$(INTDIR)\basketoption.sbr" \
	"$(INTDIR)\capfloor.sbr" \
	"$(INTDIR)\cliquetoption.sbr" \
	"$(INTDIR)\dividendvanillaoption.sbr" \
	"$(INTDIR)\europeanoption.sbr" \
	"$(INTDIR)\forwardvanillaoption.sbr" \
	"$(INTDIR)\multiassetoption.sbr" \
	"$(INTDIR)\oneassetoption.sbr" \
	"$(INTDIR)\oneassetstrikedoption.sbr" \
	"$(INTDIR)\quantoforwardvanillaoption.sbr" \
	"$(INTDIR)\quantovanillaoption.sbr" \
	"$(INTDIR)\simpleswap.sbr" \
	"$(INTDIR)\stock.sbr" \
	"$(INTDIR)\swap.sbr" \
	"$(INTDIR)\swaption.sbr" \
	"$(INTDIR)\vanillaoption.sbr" \
	"$(INTDIR)\binomialtree.sbr" \
	"$(INTDIR)\bsmlattice.sbr" \
	"$(INTDIR)\lattice.sbr" \
	"$(INTDIR)\lattice2d.sbr" \
	"$(INTDIR)\trinomialtree.sbr" \
	"$(INTDIR)\beta.sbr" \
	"$(INTDIR)\bivariatenormaldistribution.sbr" \
	"$(INTDIR)\chisquaredistribution.sbr" \
	"$(INTDIR)\choleskydecomposition.sbr" \
	"$(INTDIR)\discrepancystatistics.sbr" \
	"$(INTDIR)\errorfunction.sbr" \
	"$(INTDIR)\factorial.sbr" \
	"$(INTDIR)\gammadistribution.sbr" \
	"$(INTDIR)\generalstatistics.sbr" \
	"$(INTDIR)\incompletegamma.sbr" \
	"$(INTDIR)\incrementalstatistics.sbr" \
	"$(INTDIR)\normaldistribution.sbr" \
	"$(INTDIR)\primenumbers.sbr" \
	"$(INTDIR)\pseudosqrt.sbr" \
	"$(INTDIR)\rounding.sbr" \
	"$(INTDIR)\svd.sbr" \
	"$(INTDIR)\symmetricschurdecomposition.sbr" \
	"$(INTDIR)\armijo.sbr" \
	"$(INTDIR)\conjugategradient.sbr" \
	"$(INTDIR)\simplex.sbr" \
	"$(INTDIR)\steepestdescent.sbr" \
	"$(INTDIR)\cliquetoptionpricer.sbr" \
	"$(INTDIR)\discretegeometricapo.sbr" \
	"$(INTDIR)\discretegeometricaso.sbr" \
	"$(INTDIR)\dividendeuropeanoption.sbr" \
	"$(INTDIR)\fdbermudanoption.sbr" \
	"$(INTDIR)\fdbsmoption.sbr" \
	"$(INTDIR)\fddividendamericanoption.sbr" \
	"$(INTDIR)\fddividendoption.sbr" \
	"$(INTDIR)\fddividendshoutoption.sbr" \
	"$(INTDIR)\fdeuropean.sbr" \
	"$(INTDIR)\fdmultiperiodoption.sbr" \
	"$(INTDIR)\fdstepconditionoption.sbr" \
	"$(INTDIR)\mcbasket.sbr" \
	"$(INTDIR)\mccliquetoption.sbr" \
	"$(INTDIR)\mcdiscretearithmeticapo.sbr" \
	"$(INTDIR)\mcdiscretearithmeticaso.sbr" \
	"$(INTDIR)\mceverest.sbr" \
	"$(INTDIR)\mchimalaya.sbr" \
	"$(INTDIR)\mcmaxbasket.sbr" \
	"$(INTDIR)\mcpagoda.sbr" \
	"$(INTDIR)\mcperformanceoption.sbr" \
	"$(INTDIR)\performanceoption.sbr" \
	"$(INTDIR)\singleassetoption.sbr" \
	"$(INTDIR)\analyticcontinuousasianengine.sbr" \
	"$(INTDIR)\analyticdiscreteasianengine.sbr" \
	"$(INTDIR)\analyticbarrierengine.sbr" \
	"$(INTDIR)\mcbarrierengine.sbr" \
	"$(INTDIR)\mcamericanbasketengine.sbr" \
	"$(INTDIR)\mcbasketengine.sbr" \
	"$(INTDIR)\stulzengine.sbr" \
	"$(INTDIR)\analyticdigitalamericanengine.sbr" \
	"$(INTDIR)\analyticdividendeuropeanengine.sbr" \
	"$(INTDIR)\analyticeuropeanengine.sbr" \
	"$(INTDIR)\baroneadesiwhaleyengine.sbr" \
	"$(INTDIR)\bjerksundstenslandengine.sbr" \
	"$(INTDIR)\discretizedvanillaoption.sbr" \
	"$(INTDIR)\integralengine.sbr" \
	"$(INTDIR)\jumpdiffusionengine.sbr" \
	"$(INTDIR)\mcdigitalengine.sbr" \
	"$(INTDIR)\analyticcapfloorengine.sbr" \
	"$(INTDIR)\blackcapfloorengine.sbr" \
	"$(INTDIR)\discretizedcapfloor.sbr" \
	"$(INTDIR)\treecapfloorengine.sbr" \
	"$(INTDIR)\blackswaptionengine.sbr" \
	"$(INTDIR)\discretizedswaption.sbr" \
	"$(INTDIR)\jamshidianswaptionengine.sbr" \
	"$(INTDIR)\treeswaptionengine.sbr" \
	"$(INTDIR)\analyticcliquetengine.sbr" \
	"$(INTDIR)\analyticperformanceengine.sbr" \
	"$(INTDIR)\americanpayoffatexpiry.sbr" \
	"$(INTDIR)\americanpayoffathit.sbr" \
	"$(INTDIR)\blackformula.sbr" \
	"$(INTDIR)\haltonrsg.sbr" \
	"$(INTDIR)\knuthuniformrng.sbr" \
	"$(INTDIR)\lecuyeruniformrng.sbr" \
	"$(INTDIR)\mt19937uniformrng.sbr" \
	"$(INTDIR)\primitivepolynomials.sbr" \
	"$(INTDIR)\sobolrsg.sbr" \
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
	"$(INTDIR)\affinetermstructure.sbr" \
	"$(INTDIR)\compoundforward.sbr" \
	"$(INTDIR)\discountcurve.sbr" \
	"$(INTDIR)\extendeddiscountcurve.sbr" \
	"$(INTDIR)\piecewiseflatforward.sbr" \
	"$(INTDIR)\ratehelpers.sbr" \
	"$(INTDIR)\zerocurve.sbr" \
	"$(INTDIR)\blackvariancecurve.sbr" \
	"$(INTDIR)\blackvariancesurface.sbr" \
	"$(INTDIR)\localvolsurface.sbr" \
	"$(INTDIR)\basicdataformatters.sbr" \
	"$(INTDIR)\calendar.sbr" \
	"$(INTDIR)\dataformatters.sbr" \
	"$(INTDIR)\dataparsers.sbr" \
	"$(INTDIR)\date.sbr" \
	"$(INTDIR)\discretizedasset.sbr" \
	"$(INTDIR)\errors.sbr" \
	"$(INTDIR)\exercise.sbr" \
	"$(INTDIR)\grid.sbr" \
	"$(INTDIR)\scheduler.sbr" \
	"$(INTDIR)\stochasticprocess.sbr" \
	"$(INTDIR)\voltermstructure.sbr" \
	"$(INTDIR)\g2swaptionengine.sbr"

"$(OUTDIR)\QuantLib.bsc" : "$(OUTDIR)" $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:".\lib\QuantLib-vc6-sgd-0_3_7.lib" 
LIB32_OBJS= \
	"$(INTDIR)\beijing.obj" \
	"$(INTDIR)\budapest.obj" \
	"$(INTDIR)\copenhagen.obj" \
	"$(INTDIR)\germany.obj" \
	"$(INTDIR)\helsinki.obj" \
	"$(INTDIR)\hongkong.obj" \
	"$(INTDIR)\italy.obj" \
	"$(INTDIR)\johannesburg.obj" \
	"$(INTDIR)\jointcalendar.obj" \
	"$(INTDIR)\oslo.obj" \
	"$(INTDIR)\riyadh.obj" \
	"$(INTDIR)\seoul.obj" \
	"$(INTDIR)\singapore.obj" \
	"$(INTDIR)\stockholm.obj" \
	"$(INTDIR)\sydney.obj" \
	"$(INTDIR)\taiwan.obj" \
	"$(INTDIR)\target.obj" \
	"$(INTDIR)\tokyo.obj" \
	"$(INTDIR)\toronto.obj" \
	"$(INTDIR)\unitedkingdom.obj" \
	"$(INTDIR)\unitedstates.obj" \
	"$(INTDIR)\warsaw.obj" \
	"$(INTDIR)\wellington.obj" \
	"$(INTDIR)\zurich.obj" \
	"$(INTDIR)\basispointsensitivity.obj" \
	"$(INTDIR)\cashflowvectors.obj" \
	"$(INTDIR)\parcoupon.obj" \
	"$(INTDIR)\shortfloatingcoupon.obj" \
	"$(INTDIR)\timebasket.obj" \
	"$(INTDIR)\actualactual.obj" \
	"$(INTDIR)\simpledaycounter.obj" \
	"$(INTDIR)\thirty360.obj" \
	"$(INTDIR)\boundarycondition.obj" \
	"$(INTDIR)\bsmoperator.obj" \
	"$(INTDIR)\onefactoroperator.obj" \
	"$(INTDIR)\tridiagonaloperator.obj" \
	"$(INTDIR)\valueatcenter.obj" \
	"$(INTDIR)\xibor.obj" \
	"$(INTDIR)\xibormanager.obj" \
	"$(INTDIR)\asianoption.obj" \
	"$(INTDIR)\barrieroption.obj" \
	"$(INTDIR)\basketoption.obj" \
	"$(INTDIR)\capfloor.obj" \
	"$(INTDIR)\cliquetoption.obj" \
	"$(INTDIR)\dividendvanillaoption.obj" \
	"$(INTDIR)\europeanoption.obj" \
	"$(INTDIR)\forwardvanillaoption.obj" \
	"$(INTDIR)\multiassetoption.obj" \
	"$(INTDIR)\oneassetoption.obj" \
	"$(INTDIR)\oneassetstrikedoption.obj" \
	"$(INTDIR)\quantoforwardvanillaoption.obj" \
	"$(INTDIR)\quantovanillaoption.obj" \
	"$(INTDIR)\simpleswap.obj" \
	"$(INTDIR)\stock.obj" \
	"$(INTDIR)\swap.obj" \
	"$(INTDIR)\swaption.obj" \
	"$(INTDIR)\vanillaoption.obj" \
	"$(INTDIR)\binomialtree.obj" \
	"$(INTDIR)\bsmlattice.obj" \
	"$(INTDIR)\lattice.obj" \
	"$(INTDIR)\lattice2d.obj" \
	"$(INTDIR)\trinomialtree.obj" \
	"$(INTDIR)\beta.obj" \
	"$(INTDIR)\bivariatenormaldistribution.obj" \
	"$(INTDIR)\chisquaredistribution.obj" \
	"$(INTDIR)\choleskydecomposition.obj" \
	"$(INTDIR)\discrepancystatistics.obj" \
	"$(INTDIR)\errorfunction.obj" \
	"$(INTDIR)\factorial.obj" \
	"$(INTDIR)\gammadistribution.obj" \
	"$(INTDIR)\generalstatistics.obj" \
	"$(INTDIR)\incompletegamma.obj" \
	"$(INTDIR)\incrementalstatistics.obj" \
	"$(INTDIR)\normaldistribution.obj" \
	"$(INTDIR)\primenumbers.obj" \
	"$(INTDIR)\pseudosqrt.obj" \
	"$(INTDIR)\rounding.obj" \
	"$(INTDIR)\svd.obj" \
	"$(INTDIR)\symmetricschurdecomposition.obj" \
	"$(INTDIR)\armijo.obj" \
	"$(INTDIR)\conjugategradient.obj" \
	"$(INTDIR)\simplex.obj" \
	"$(INTDIR)\steepestdescent.obj" \
	"$(INTDIR)\cliquetoptionpricer.obj" \
	"$(INTDIR)\discretegeometricapo.obj" \
	"$(INTDIR)\discretegeometricaso.obj" \
	"$(INTDIR)\dividendeuropeanoption.obj" \
	"$(INTDIR)\fdbermudanoption.obj" \
	"$(INTDIR)\fdbsmoption.obj" \
	"$(INTDIR)\fddividendamericanoption.obj" \
	"$(INTDIR)\fddividendoption.obj" \
	"$(INTDIR)\fddividendshoutoption.obj" \
	"$(INTDIR)\fdeuropean.obj" \
	"$(INTDIR)\fdmultiperiodoption.obj" \
	"$(INTDIR)\fdstepconditionoption.obj" \
	"$(INTDIR)\mcbasket.obj" \
	"$(INTDIR)\mccliquetoption.obj" \
	"$(INTDIR)\mcdiscretearithmeticapo.obj" \
	"$(INTDIR)\mcdiscretearithmeticaso.obj" \
	"$(INTDIR)\mceverest.obj" \
	"$(INTDIR)\mchimalaya.obj" \
	"$(INTDIR)\mcmaxbasket.obj" \
	"$(INTDIR)\mcpagoda.obj" \
	"$(INTDIR)\mcperformanceoption.obj" \
	"$(INTDIR)\performanceoption.obj" \
	"$(INTDIR)\singleassetoption.obj" \
	"$(INTDIR)\analyticcontinuousasianengine.obj" \
	"$(INTDIR)\analyticdiscreteasianengine.obj" \
	"$(INTDIR)\analyticbarrierengine.obj" \
	"$(INTDIR)\mcbarrierengine.obj" \
	"$(INTDIR)\mcamericanbasketengine.obj" \
	"$(INTDIR)\mcbasketengine.obj" \
	"$(INTDIR)\stulzengine.obj" \
	"$(INTDIR)\analyticdigitalamericanengine.obj" \
	"$(INTDIR)\analyticdividendeuropeanengine.obj" \
	"$(INTDIR)\analyticeuropeanengine.obj" \
	"$(INTDIR)\baroneadesiwhaleyengine.obj" \
	"$(INTDIR)\bjerksundstenslandengine.obj" \
	"$(INTDIR)\discretizedvanillaoption.obj" \
	"$(INTDIR)\integralengine.obj" \
	"$(INTDIR)\jumpdiffusionengine.obj" \
	"$(INTDIR)\mcdigitalengine.obj" \
	"$(INTDIR)\analyticcapfloorengine.obj" \
	"$(INTDIR)\blackcapfloorengine.obj" \
	"$(INTDIR)\discretizedcapfloor.obj" \
	"$(INTDIR)\treecapfloorengine.obj" \
	"$(INTDIR)\blackswaptionengine.obj" \
	"$(INTDIR)\discretizedswaption.obj" \
	"$(INTDIR)\jamshidianswaptionengine.obj" \
	"$(INTDIR)\treeswaptionengine.obj" \
	"$(INTDIR)\analyticcliquetengine.obj" \
	"$(INTDIR)\analyticperformanceengine.obj" \
	"$(INTDIR)\americanpayoffatexpiry.obj" \
	"$(INTDIR)\americanpayoffathit.obj" \
	"$(INTDIR)\blackformula.obj" \
	"$(INTDIR)\haltonrsg.obj" \
	"$(INTDIR)\knuthuniformrng.obj" \
	"$(INTDIR)\lecuyeruniformrng.obj" \
	"$(INTDIR)\mt19937uniformrng.obj" \
	"$(INTDIR)\primitivepolynomials.obj" \
	"$(INTDIR)\sobolrsg.obj" \
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
	"$(INTDIR)\affinetermstructure.obj" \
	"$(INTDIR)\compoundforward.obj" \
	"$(INTDIR)\discountcurve.obj" \
	"$(INTDIR)\extendeddiscountcurve.obj" \
	"$(INTDIR)\piecewiseflatforward.obj" \
	"$(INTDIR)\ratehelpers.obj" \
	"$(INTDIR)\zerocurve.obj" \
	"$(INTDIR)\blackvariancecurve.obj" \
	"$(INTDIR)\blackvariancesurface.obj" \
	"$(INTDIR)\localvolsurface.obj" \
	"$(INTDIR)\basicdataformatters.obj" \
	"$(INTDIR)\calendar.obj" \
	"$(INTDIR)\dataformatters.obj" \
	"$(INTDIR)\dataparsers.obj" \
	"$(INTDIR)\date.obj" \
	"$(INTDIR)\discretizedasset.obj" \
	"$(INTDIR)\errors.obj" \
	"$(INTDIR)\exercise.obj" \
	"$(INTDIR)\grid.obj" \
	"$(INTDIR)\scheduler.obj" \
	"$(INTDIR)\stochasticprocess.obj" \
	"$(INTDIR)\voltermstructure.obj" \
	"$(INTDIR)\g2swaptionengine.obj" \
	".\lib\QuantLibFunctions-vc6-sgd-0_3_7.lib"

".\lib\QuantLib-vc6-sgd-0_3_7.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
   if not exist lib mkdir lib
	 $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

SOURCE="$(InputPath)"

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("QuantLib.dep")
!INCLUDE "QuantLib.dep"
!ELSE 
!MESSAGE Warning: cannot find "QuantLib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "QuantLib - Win32 Release" || "$(CFG)" == "QuantLib - Win32 Debug" || "$(CFG)" == "QuantLib - Win32 Release MTDLL" || "$(CFG)" == "QuantLib - Win32 Debug MTDLL" || "$(CFG)" == "QuantLib - Win32 Release SingleThread" || "$(CFG)" == "QuantLib - Win32 Debug SingleThread"
SOURCE=.\ql\Calendars\beijing.cpp

"$(INTDIR)\beijing.obj"	"$(INTDIR)\beijing.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\budapest.cpp

"$(INTDIR)\budapest.obj"	"$(INTDIR)\budapest.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\copenhagen.cpp

"$(INTDIR)\copenhagen.obj"	"$(INTDIR)\copenhagen.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\germany.cpp

"$(INTDIR)\germany.obj"	"$(INTDIR)\germany.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\helsinki.cpp

"$(INTDIR)\helsinki.obj"	"$(INTDIR)\helsinki.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\hongkong.cpp

"$(INTDIR)\hongkong.obj"	"$(INTDIR)\hongkong.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\italy.cpp

"$(INTDIR)\italy.obj"	"$(INTDIR)\italy.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\johannesburg.cpp

"$(INTDIR)\johannesburg.obj"	"$(INTDIR)\johannesburg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\jointcalendar.cpp

"$(INTDIR)\jointcalendar.obj"	"$(INTDIR)\jointcalendar.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\oslo.cpp

"$(INTDIR)\oslo.obj"	"$(INTDIR)\oslo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\riyadh.cpp

"$(INTDIR)\riyadh.obj"	"$(INTDIR)\riyadh.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\seoul.cpp

"$(INTDIR)\seoul.obj"	"$(INTDIR)\seoul.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\singapore.cpp

"$(INTDIR)\singapore.obj"	"$(INTDIR)\singapore.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\stockholm.cpp

"$(INTDIR)\stockholm.obj"	"$(INTDIR)\stockholm.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\sydney.cpp

"$(INTDIR)\sydney.obj"	"$(INTDIR)\sydney.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\taiwan.cpp

"$(INTDIR)\taiwan.obj"	"$(INTDIR)\taiwan.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Calendars\unitedkingdom.cpp

"$(INTDIR)\unitedkingdom.obj"	"$(INTDIR)\unitedkingdom.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Calendars\unitedstates.cpp

"$(INTDIR)\unitedstates.obj"	"$(INTDIR)\unitedstates.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\CashFlows\basispointsensitivity.cpp

"$(INTDIR)\basispointsensitivity.obj"	"$(INTDIR)\basispointsensitivity.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\CashFlows\timebasket.cpp

"$(INTDIR)\timebasket.obj"	"$(INTDIR)\timebasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\DayCounters\actualactual.cpp

"$(INTDIR)\actualactual.obj"	"$(INTDIR)\actualactual.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\DayCounters\simpledaycounter.cpp

"$(INTDIR)\simpledaycounter.obj"	"$(INTDIR)\simpledaycounter.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Instruments\asianoption.cpp

"$(INTDIR)\asianoption.obj"	"$(INTDIR)\asianoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\barrieroption.cpp

"$(INTDIR)\barrieroption.obj"	"$(INTDIR)\barrieroption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\basketoption.cpp

"$(INTDIR)\basketoption.obj"	"$(INTDIR)\basketoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\capfloor.cpp

"$(INTDIR)\capfloor.obj"	"$(INTDIR)\capfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\cliquetoption.cpp

"$(INTDIR)\cliquetoption.obj"	"$(INTDIR)\cliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\dividendvanillaoption.cpp

"$(INTDIR)\dividendvanillaoption.obj"	"$(INTDIR)\dividendvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\europeanoption.cpp

"$(INTDIR)\europeanoption.obj"	"$(INTDIR)\europeanoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\forwardvanillaoption.cpp

"$(INTDIR)\forwardvanillaoption.obj"	"$(INTDIR)\forwardvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\multiassetoption.cpp

"$(INTDIR)\multiassetoption.obj"	"$(INTDIR)\multiassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\oneassetoption.cpp

"$(INTDIR)\oneassetoption.obj"	"$(INTDIR)\oneassetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Instruments\oneassetstrikedoption.cpp

"$(INTDIR)\oneassetstrikedoption.obj"	"$(INTDIR)\oneassetstrikedoption.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Math\beta.cpp

"$(INTDIR)\beta.obj"	"$(INTDIR)\beta.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\bivariatenormaldistribution.cpp

"$(INTDIR)\bivariatenormaldistribution.obj"	"$(INTDIR)\bivariatenormaldistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\chisquaredistribution.cpp

"$(INTDIR)\chisquaredistribution.obj"	"$(INTDIR)\chisquaredistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\choleskydecomposition.cpp

"$(INTDIR)\choleskydecomposition.obj"	"$(INTDIR)\choleskydecomposition.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\discrepancystatistics.cpp

"$(INTDIR)\discrepancystatistics.obj"	"$(INTDIR)\discrepancystatistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\errorfunction.cpp

"$(INTDIR)\errorfunction.obj"	"$(INTDIR)\errorfunction.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\factorial.cpp

"$(INTDIR)\factorial.obj"	"$(INTDIR)\factorial.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\gammadistribution.cpp

"$(INTDIR)\gammadistribution.obj"	"$(INTDIR)\gammadistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\generalstatistics.cpp

"$(INTDIR)\generalstatistics.obj"	"$(INTDIR)\generalstatistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\incompletegamma.cpp

"$(INTDIR)\incompletegamma.obj"	"$(INTDIR)\incompletegamma.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\incrementalstatistics.cpp

"$(INTDIR)\incrementalstatistics.obj"	"$(INTDIR)\incrementalstatistics.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\normaldistribution.cpp

"$(INTDIR)\normaldistribution.obj"	"$(INTDIR)\normaldistribution.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\primenumbers.cpp

"$(INTDIR)\primenumbers.obj"	"$(INTDIR)\primenumbers.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\pseudosqrt.cpp

"$(INTDIR)\pseudosqrt.obj"	"$(INTDIR)\pseudosqrt.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\rounding.cpp

"$(INTDIR)\rounding.obj"	"$(INTDIR)\rounding.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\svd.cpp

"$(INTDIR)\svd.obj"	"$(INTDIR)\svd.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Math\symmetricschurdecomposition.cpp

"$(INTDIR)\symmetricschurdecomposition.obj"	"$(INTDIR)\symmetricschurdecomposition.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Pricers\cliquetoptionpricer.cpp

"$(INTDIR)\cliquetoptionpricer.obj"	"$(INTDIR)\cliquetoptionpricer.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\discretegeometricapo.cpp

"$(INTDIR)\discretegeometricapo.obj"	"$(INTDIR)\discretegeometricapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\discretegeometricaso.cpp

"$(INTDIR)\discretegeometricaso.obj"	"$(INTDIR)\discretegeometricaso.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\dividendeuropeanoption.cpp

"$(INTDIR)\dividendeuropeanoption.obj"	"$(INTDIR)\dividendeuropeanoption.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Pricers\mcbasket.cpp

!IF  "$(CFG)" == "QuantLib - Win32 Release"

CPP_SWITCHES=/nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Oi- /c 

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

CPP_SWITCHES=/nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd".\lib\QuantLib-vc6-mt-sgd-0_3_7" /FD /GZ /c 

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Release MTDLL"

CPP_SWITCHES=/nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Oi- /c 

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug MTDLL"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd".\lib\QuantLib-vc6-mt-gd-0_3_7" /FD /GZ /c 

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Release SingleThread"

CPP_SWITCHES=/nologo /ML /W3 /Gi /GR /GX /O2 /Ob2 /I "." /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /Oi- /c 

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug SingleThread"

CPP_SWITCHES=/nologo /MLd /W3 /Gm /Gi /GR /GX /ZI /Od /I "." /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /Fr"$(INTDIR)\\" /Fp"$(INTDIR)\QuantLib.pch" /YX /Fo"$(INTDIR)\\" /Fd".\lib\QuantLib-vc6-sgd-0_3_7" /FD /GZ /c 

"$(INTDIR)\mcbasket.obj"	"$(INTDIR)\mcbasket.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 

SOURCE=.\ql\Pricers\mccliquetoption.cpp

"$(INTDIR)\mccliquetoption.obj"	"$(INTDIR)\mccliquetoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcdiscretearithmeticapo.cpp

"$(INTDIR)\mcdiscretearithmeticapo.obj"	"$(INTDIR)\mcdiscretearithmeticapo.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Pricers\mcdiscretearithmeticaso.cpp

"$(INTDIR)\mcdiscretearithmeticaso.obj"	"$(INTDIR)\mcdiscretearithmeticaso.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\PricingEngines\Asian\analyticcontinuousasianengine.cpp

"$(INTDIR)\analyticcontinuousasianengine.obj"	"$(INTDIR)\analyticcontinuousasianengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Asian\analyticdiscreteasianengine.cpp

"$(INTDIR)\analyticdiscreteasianengine.obj"	"$(INTDIR)\analyticdiscreteasianengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Barrier\analyticbarrierengine.cpp

"$(INTDIR)\analyticbarrierengine.obj"	"$(INTDIR)\analyticbarrierengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Barrier\mcbarrierengine.cpp

"$(INTDIR)\mcbarrierengine.obj"	"$(INTDIR)\mcbarrierengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Basket\mcamericanbasketengine.cpp

"$(INTDIR)\mcamericanbasketengine.obj"	"$(INTDIR)\mcamericanbasketengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Basket\mcbasketengine.cpp

"$(INTDIR)\mcbasketengine.obj"	"$(INTDIR)\mcbasketengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Basket\stulzengine.cpp

"$(INTDIR)\stulzengine.obj"	"$(INTDIR)\stulzengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\analyticdigitalamericanengine.cpp

"$(INTDIR)\analyticdigitalamericanengine.obj"	"$(INTDIR)\analyticdigitalamericanengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\analyticdividendeuropeanengine.cpp

"$(INTDIR)\analyticdividendeuropeanengine.obj"	"$(INTDIR)\analyticdividendeuropeanengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\analyticeuropeanengine.cpp

"$(INTDIR)\analyticeuropeanengine.obj"	"$(INTDIR)\analyticeuropeanengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\baroneadesiwhaleyengine.cpp

"$(INTDIR)\baroneadesiwhaleyengine.obj"	"$(INTDIR)\baroneadesiwhaleyengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\bjerksundstenslandengine.cpp

"$(INTDIR)\bjerksundstenslandengine.obj"	"$(INTDIR)\bjerksundstenslandengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\discretizedvanillaoption.cpp

"$(INTDIR)\discretizedvanillaoption.obj"	"$(INTDIR)\discretizedvanillaoption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\integralengine.cpp

"$(INTDIR)\integralengine.obj"	"$(INTDIR)\integralengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\jumpdiffusionengine.cpp

"$(INTDIR)\jumpdiffusionengine.obj"	"$(INTDIR)\jumpdiffusionengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Vanilla\mcdigitalengine.cpp

"$(INTDIR)\mcdigitalengine.obj"	"$(INTDIR)\mcdigitalengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\CapFloor\analyticcapfloorengine.cpp

"$(INTDIR)\analyticcapfloorengine.obj"	"$(INTDIR)\analyticcapfloorengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\CapFloor\blackcapfloorengine.cpp

"$(INTDIR)\blackcapfloorengine.obj"	"$(INTDIR)\blackcapfloorengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\CapFloor\discretizedcapfloor.cpp

"$(INTDIR)\discretizedcapfloor.obj"	"$(INTDIR)\discretizedcapfloor.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\CapFloor\treecapfloorengine.cpp

"$(INTDIR)\treecapfloorengine.obj"	"$(INTDIR)\treecapfloorengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Swaption\blackswaptionengine.cpp

"$(INTDIR)\blackswaptionengine.obj"	"$(INTDIR)\blackswaptionengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Swaption\discretizedswaption.cpp

"$(INTDIR)\discretizedswaption.obj"	"$(INTDIR)\discretizedswaption.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Swaption\g2swaptionengine.cpp

"$(INTDIR)\g2swaptionengine.obj"	"$(INTDIR)\g2swaptionengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Swaption\jamshidianswaptionengine.cpp

"$(INTDIR)\jamshidianswaptionengine.obj"	"$(INTDIR)\jamshidianswaptionengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Swaption\treeswaptionengine.cpp

"$(INTDIR)\treeswaptionengine.obj"	"$(INTDIR)\treeswaptionengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Cliquet\analyticcliquetengine.cpp

"$(INTDIR)\analyticcliquetengine.obj"	"$(INTDIR)\analyticcliquetengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\Cliquet\analyticperformanceengine.cpp

"$(INTDIR)\analyticperformanceengine.obj"	"$(INTDIR)\analyticperformanceengine.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\americanpayoffatexpiry.cpp

"$(INTDIR)\americanpayoffatexpiry.obj"	"$(INTDIR)\americanpayoffatexpiry.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\americanpayoffathit.cpp

"$(INTDIR)\americanpayoffathit.obj"	"$(INTDIR)\americanpayoffathit.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\PricingEngines\blackformula.cpp

"$(INTDIR)\blackformula.obj"	"$(INTDIR)\blackformula.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\haltonrsg.cpp

"$(INTDIR)\haltonrsg.obj"	"$(INTDIR)\haltonrsg.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\knuthuniformrng.cpp

"$(INTDIR)\knuthuniformrng.obj"	"$(INTDIR)\knuthuniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\lecuyeruniformrng.cpp

"$(INTDIR)\lecuyeruniformrng.obj"	"$(INTDIR)\lecuyeruniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\mt19937uniformrng.cpp

"$(INTDIR)\mt19937uniformrng.obj"	"$(INTDIR)\mt19937uniformrng.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\primitivepolynomials.c

"$(INTDIR)\primitivepolynomials.obj"	"$(INTDIR)\primitivepolynomials.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\RandomNumbers\sobolrsg.cpp

"$(INTDIR)\sobolrsg.obj"	"$(INTDIR)\sobolrsg.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\TermStructures\affinetermstructure.cpp

"$(INTDIR)\affinetermstructure.obj"	"$(INTDIR)\affinetermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\compoundforward.cpp

"$(INTDIR)\compoundforward.obj"	"$(INTDIR)\compoundforward.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\discountcurve.cpp

"$(INTDIR)\discountcurve.obj"	"$(INTDIR)\discountcurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\TermStructures\extendeddiscountcurve.cpp

"$(INTDIR)\extendeddiscountcurve.obj"	"$(INTDIR)\extendeddiscountcurve.sbr" : $(SOURCE) "$(INTDIR)"
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


SOURCE=.\ql\Volatilities\blackvariancecurve.cpp

"$(INTDIR)\blackvariancecurve.obj"	"$(INTDIR)\blackvariancecurve.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Volatilities\blackvariancesurface.cpp

"$(INTDIR)\blackvariancesurface.obj"	"$(INTDIR)\blackvariancesurface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\Volatilities\localvolsurface.cpp

"$(INTDIR)\localvolsurface.obj"	"$(INTDIR)\localvolsurface.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


!IF  "$(CFG)" == "QuantLib - Win32 Release"

"QuantLibFunctions - Win32 Release" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Release" 
   cd "..\..\.."

"QuantLibFunctions - Win32 ReleaseCLEAN" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Release" RECURSE=1 CLEAN 
   cd "..\..\.."

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

"QuantLibFunctions - Win32 Debug" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Debug" 
   cd "..\..\.."

"QuantLibFunctions - Win32 DebugCLEAN" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Debug" RECURSE=1 CLEAN 
   cd "..\..\.."

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Release MTDLL"

"QuantLibFunctions - Win32 Release MTDLL" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Release MTDLL" 
   cd "..\..\.."

"QuantLibFunctions - Win32 Release MTDLLCLEAN" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Release MTDLL" RECURSE=1 CLEAN 
   cd "..\..\.."

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug MTDLL"

"QuantLibFunctions - Win32 Debug MTDLL" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Debug MTDLL" 
   cd "..\..\.."

"QuantLibFunctions - Win32 Debug MTDLLCLEAN" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Debug MTDLL" RECURSE=1 CLEAN 
   cd "..\..\.."

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Release SingleThread"

"QuantLibFunctions - Win32 Release SingleThread" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Release SingleThread" 
   cd "..\..\.."

"QuantLibFunctions - Win32 Release SingleThreadCLEAN" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Release SingleThread" RECURSE=1 CLEAN 
   cd "..\..\.."

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug SingleThread"

"QuantLibFunctions - Win32 Debug SingleThread" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Debug SingleThread" 
   cd "..\..\.."

"QuantLibFunctions - Win32 Debug SingleThreadCLEAN" : 
   cd ".\FUNCTIONS\QL\FUNCTIONS"
   $(MAKE) /$(MAKEFLAGS) /F .\QuantLibFunctions.mak CFG="QuantLibFunctions - Win32 Debug SingleThread" RECURSE=1 CLEAN 
   cd "..\..\.."

!ENDIF 

SOURCE=.\ql\basicdataformatters.cpp

"$(INTDIR)\basicdataformatters.obj"	"$(INTDIR)\basicdataformatters.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


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


SOURCE=.\ql\discretizedasset.cpp

"$(INTDIR)\discretizedasset.obj"	"$(INTDIR)\discretizedasset.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\errors.cpp

"$(INTDIR)\errors.obj"	"$(INTDIR)\errors.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\exercise.cpp

"$(INTDIR)\exercise.obj"	"$(INTDIR)\exercise.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\grid.cpp

"$(INTDIR)\grid.obj"	"$(INTDIR)\grid.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\scheduler.cpp

"$(INTDIR)\scheduler.obj"	"$(INTDIR)\scheduler.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\stochasticprocess.cpp

"$(INTDIR)\stochasticprocess.obj"	"$(INTDIR)\stochasticprocess.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\ql\voltermstructure.cpp

"$(INTDIR)\voltermstructure.obj"	"$(INTDIR)\voltermstructure.sbr" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

