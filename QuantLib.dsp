# Microsoft Developer Studio Project File - Name="QuantLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=QuantLib - Win32 OnTheEdgeDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QuantLib.mak".
!MESSAGE 
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
# ADD CPP /nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib.lib"

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
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "QuantLib___Win32_OnTheEdgeDebug"
# PROP BASE Intermediate_Dir "QuantLib___Win32_OnTheEdgeDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "OnTheEdgeDebug"
# PROP Intermediate_Dir "OnTheEdgeDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib"
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "QuantLib___Win32_OnTheEdgeRelease"
# PROP BASE Intermediate_Dir "QuantLib___Win32_OnTheEdgeRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "OnTheEdgeRelease"
# PROP Intermediate_Dir "OnTheEdgeRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib.lib"
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib.lib"

!ENDIF 

# Begin Target

# Name "QuantLib - Win32 Release"
# Name "QuantLib - Win32 Debug"
# Name "QuantLib - Win32 OnTheEdgeDebug"
# Name "QuantLib - Win32 OnTheEdgeRelease"
# Begin Group "Calendars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sources\Calendars\frankfurt.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\frankfurt.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\helsinki.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\helsinki.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\london.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\london.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\milan.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\milan.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\newyork.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\newyork.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\target.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\target.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\wellington.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\wellington.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\westerncalendar.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\westerncalendar.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Calendars\zurich.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Calendars\zurich.hpp
# End Source File
# End Group
# Begin Group "Day Counters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\DayCounters\actual360.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\DayCounters\actual365.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\DayCounters\actualactual.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\DayCounters\actualactual.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\DayCounters\thirty360.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\DayCounters\thirty360.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\DayCounters\thirty360european.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\DayCounters\thirty360italian.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\DayCounters\thirty360italian.hpp
# End Source File
# End Group
# Begin Group "Finite Differences"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\backwardeuler.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\boundarycondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\FiniteDifferences\bsmoperator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\bsmoperator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\cranknicolson.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\dminus.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\dplus.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\dplusdminus.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\dzero.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\finitedifferencemodel.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\forwardeuler.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\identity.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\operator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\operatortraits.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\standardfdmodel.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\standardstepcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\stepcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\FiniteDifferences\tridiagonaloperator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\tridiagonaloperator.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\FiniteDifferences\valueatcenter.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\FiniteDifferences\valueatcenter.hpp
# End Source File
# End Group
# Begin Group "Instruments"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sources\Instruments\simpleswap.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Instruments\simpleswap.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Instruments\stock.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Instruments\stock.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Instruments\swap.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Instruments\swap.hpp
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\Math\cubicspline.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\interpolation.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\lexicographicalview.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\linearinterpolation.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\matrix.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\multivariateaccumulator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\multivariateaccumulator.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\normaldistribution.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\normaldistribution.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\riskmeasure.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\riskmeasures.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\statistics.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\symmetriceigenvalues.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Math\symmetricschurdecomposition.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Math\symmetricschurdecomposition.hpp
# End Source File
# End Group
# Begin Group "MonteCarlo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sources\MonteCarlo\avgpriceasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\avgpriceasianpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\avgstrikeasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\avgstrikeasianpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\basketpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\basketpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\boxmuller.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\centrallimitgaussian.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\controlvariatedpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\controlvariatedpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\europeanpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\europeanpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\everestpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\everestpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\gaussianarraygenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\gaussianrandomgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\generalmontecarlo.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\geometricasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\geometricasianpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\getcovariance.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\getcovariance.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\himalayapathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\himalayapathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\knuthrandomgenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\knuthrandomgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\lecuyerrandomgenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\lecuyerrandomgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\mcoptionsample.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\mcpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\multifactormontecarlooption.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\multifactorpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\multipath.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\multipathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\multipathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\onefactormontecarlooption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\MonteCarlo\pagodapathpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\pagodapathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\path.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\pathmontecarlo.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\pathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\randomarraygenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\standardmultipathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\standardpathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\MonteCarlo\uniformrandomgenerator.hpp
# End Source File
# End Group
# Begin Group "Patterns"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\Patterns\observable.hpp
# End Source File
# End Group
# Begin Group "Pricers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\Pricers\americancondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\americanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\averagepriceasian.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\averagepriceasian.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\averagestrikeasian.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\averagestrikeasian.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\barrieroption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\barrieroption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bermudanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\bermudanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\binaryoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\binaryoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bsmeuropeanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\bsmeuropeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bsmnumericaloption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\bsmnumericaloption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\bsmoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\bsmoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\cliquetoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\cliquetoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendamericanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\dividendamericanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendeuropeanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\dividendeuropeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\dividendoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\dividendshoutoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\dividendshoutoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\everestoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\everestoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\finitedifferenceeuropean.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\finitedifferenceeuropean.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\geometricasianoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\himalaya.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\himalaya.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\mceuropeanpricer.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\mceuropeanpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\multiperiodoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\multiperiodoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\pagodaoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\pagodaoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\plainbasketoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\plainbasketoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\shoutcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\shoutoption.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Pricers\stepconditionoption.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Pricers\stepconditionoption.hpp
# End Source File
# End Group
# Begin Group "Solvers 1D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Sources\Solvers1D\bisection.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\bisection.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\brent.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\brent.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\falseposition.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\falseposition.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\newton.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\newton.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\newtonsafe.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\newtonsafe.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\ridder.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\ridder.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Solvers1D\secant.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Solvers1D\secant.hpp
# End Source File
# End Group
# Begin Group "Term Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\TermStructures\flatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\TermStructures\piecewiseconstantforwards.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\TermStructures\piecewiseconstantforwards.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\TermStructures\piecewiseflatforward.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\TermStructures\piecewiseflatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\TermStructures\ratehelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\TermStructures\ratehelpers.hpp
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\Utilities\combiningiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Utilities\couplingiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Utilities\filteringiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Utilities\iteratorcategories.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Utilities\processingiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Utilities\steppingiterator.hpp
# End Source File
# End Group
# Begin Group "Indexes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\Indexes\euribor.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Indexes\libor.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Indexes\usdlibor.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Indexes\xibor.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Indexes\xibor.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\Indexes\xibormanager.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\Indexes\xibormanager.hpp
# End Source File
# End Group
# Begin Group "CashFlows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Include\ql\CashFlows\accruingcoupon.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\CashFlows\cashflowvectors.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\CashFlows\cashflowvectors.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\CashFlows\fixedratecoupon.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\CashFlows\floatingratecoupon.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\CashFlows\floatingratecoupon.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\CashFlows\indexlinkedcoupon.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\CashFlows\Makefile.am
# End Source File
# Begin Source File

SOURCE=.\Include\ql\CashFlows\simplecashflow.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\Include\ql\array.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\calendar.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\calendar.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\cashflow.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\config.ansi.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\config.bcc.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\config.decc.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\config.msvc.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\config.mwcw.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\currency.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\dataformatters.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\dataformatters.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\date.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\date.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\daycounter.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\depositrate.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\errors.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\expressiontemplates.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\forwardvolsurface.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\handle.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\history.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\index.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\instrument.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\null.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\options.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\qldefines.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\quantlib.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\riskstatistics.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\scheduler.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\scheduler.hpp
# End Source File
# Begin Source File

SOURCE=.\Sources\solver1d.cpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\solver1d.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\swaptionvolsurface.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\termstructure.hpp
# End Source File
# Begin Source File

SOURCE=.\Include\ql\types.hpp
# End Source File
# End Target
# End Project
