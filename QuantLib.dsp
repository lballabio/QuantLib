# Microsoft Developer Studio Project File - Name="QuantLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=QuantLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QuantLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QuantLib.mak" CFG="QuantLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QuantLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QuantLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /I "Include\Pricers" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "QuantLib - Win32 Release"
# Name "QuantLib - Win32 Debug"
# Begin Group "Calendars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sources\Calendars\frankfurt.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\frankfurt.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\helsinki.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\helsinki.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\london.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\london.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\milan.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\milan.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\newyork.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\newyork.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\target.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\target.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\wellington.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\wellington.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\westerncalendar.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\westerncalendar.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\zurich.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Calendars\zurich.hpp
# End Source File
# End Group
# Begin Group "Currencies"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Currencies\aud.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\cad.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\chf.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\dem.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\dkk.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\eur.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\gbp.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\itl.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\jpy.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\sek.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Currencies\usd.hpp
# End Source File
# End Group
# Begin Group "Day Counters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\DayCounters\actual360.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\DayCounters\actual365.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\DayCounters\actualactual.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\DayCounters\actualactual.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\DayCounters\thirty360.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\DayCounters\thirty360.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\DayCounters\thirty360european.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\DayCounters\thirty360italian.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\DayCounters\thirty360italian.hpp
# End Source File
# End Group
# Begin Group "Finite Differences"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\FiniteDifferences\backwardeuler.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\boundarycondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\FiniteDifferences\bsmoperator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\bsmoperator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\cranknicolson.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\dminus.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\dplus.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\dplusdminus.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\dzero.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\finitedifferencemodel.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\forwardeuler.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\identity.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\operator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\operatortraits.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\standardfdmodel.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\standardstepcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\stepcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\FiniteDifferences\tridiagonaloperator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\tridiagonaloperator.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\FiniteDifferences\valueatcenter.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\FiniteDifferences\valueatcenter.hpp
# End Source File
# End Group
# Begin Group "Instruments"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Instruments\stock.hpp
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Math\cubicspline.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\interpolation.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\lexicographicalview.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\linearinterpolation.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\matrix.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\multivariateaccumulator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\multivariateaccumulator.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\normaldistribution.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\normaldistribution.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\statistics.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\symmetriceigenvalues.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\symmetricschurdecomposition.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\symmetricschurdecomposition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Math\vartool.hpp
# End Source File
# End Group
# Begin Group "MonteCarlo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\MonteCarlo\antitheticcv.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\avgpriceasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\avgpriceasianpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\avgstrikeasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\avgstrikeasianpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\basketpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\basketpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\boxmuller.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\centrallimitgaussian.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\controlvariatedpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\controlvariatedpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\europeanpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\europeanpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\everestpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\everestpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\gaussianarraygenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\gaussianrandomgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\generalmontecarlo.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\geometricasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\geometricasianpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\getcovariance.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\getcovariance.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\himalayapathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\himalayapathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\lecuyerrandomgenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\lecuyerrandomgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\mcoptionsample.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\mcpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\multifactormontecarlooption.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\multifactorpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\multipath.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\multipathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\multipathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\onefactormontecarlooption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\pagodapathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\pagodapathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\path.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\pathmontecarlo.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\pathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\randomarraygenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\standardmultipathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\standardpathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\MonteCarlo\uniformrandomgenerator.hpp
# End Source File
# End Group
# Begin Group "Patterns"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Patterns\observable.hpp
# End Source File
# End Group
# Begin Group "Pricers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Pricers\americancondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\americanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\averagepriceasian.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\averagepriceasian.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\averagestrikeasian.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\averagestrikeasian.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\barrieroption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\barrieroption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bermudanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\bermudanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\binaryoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\binaryoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bsmeuropeanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\bsmeuropeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bsmnumericaloption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\bsmnumericaloption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bsmoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\bsmoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\cliquetoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\cliquetoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendamericanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\dividendamericanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendeuropeanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\dividendeuropeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\dividendoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendshoutoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\dividendshoutoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\everestoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\everestoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\finitedifferenceeuropean.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\finitedifferenceeuropean.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\geometricasianoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\himalaya.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\himalaya.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\mceuropeanpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\mceuropeanpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\multiperiodoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\multiperiodoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\pagodaoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\pagodaoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\plainbasketoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\plainbasketoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\shoutcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\shoutoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\stepconditionoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Pricers\stepconditionoption.hpp
# End Source File
# End Group
# Begin Group "Solvers 1D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sources\Solvers1D\bisection.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\bisection.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\brent.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\brent.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\falseposition.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\falseposition.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\newton.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\newton.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\newtonsafe.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\newtonsafe.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\ridder.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\ridder.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\secant.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\Solvers1D\secant.hpp
# End Source File
# End Group
# Begin Group "Term Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\TermStructures\flatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\TermStructures\piecewiseconstantforwards.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\TermStructures\piecewiseconstantforwards.hpp
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\Utilities\combiningiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Utilities\couplingiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Utilities\filteringiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Utilities\iteratorcategories.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Utilities\processingiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\Utilities\steppingiterator.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\array.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\calendar.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\calendar.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\config.msvc.h
# End Source File
# Begin Source File

SOURCE=.\Include\currency.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\dataformatters.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\dataformatters.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\date.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\date.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\daycounter.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\depositrate.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\discountfactor.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\expressiontemplates.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\forwardvolsurface.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\handle.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\history.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\instrument.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\null.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\options.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\qldefines.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\qlerrors.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\quantlib.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\rate.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\ratehelper.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ratehelper.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\riskstatistics.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\solver1d.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\solver1d.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\spread.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\swaptionvolsurface.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\termstructure.hpp
# End Source File
# End Target
# End Project
