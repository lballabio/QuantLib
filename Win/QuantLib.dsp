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
# ADD CPP /nologo /MT /W3 /GR /GX /Od /I "..\Include" /I "..\Include\Calendars" /I "..\Include\Currencies" /I "..\Include\FiniteDifferences" /I "..\Include\Instruments" /I "..\Include\Math" /I "..\Include\Patterns" /I "..\Include\Pricers" /I "..\Include\Solvers1D" /I "..\Include\TermStructures" /I "..\Include\DayCounters" /I "..\Include\MonteCarlo" /I "..\Include\Utilities" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "QL_RELEASE" /YX /FD /c
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
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\Include" /I "..\Include\Calendars" /I "..\Include\Currencies" /I "..\Include\FiniteDifferences" /I "..\Include\Instruments" /I "..\Include\Math" /I "..\Include\Patterns" /I "..\Include\Pricers" /I "..\Include\Solvers1D" /I "..\Include\TermStructures" /I "..\Include\DayCounters" /I "..\Include\MonteCarlo" /I "..\Include\Utilities" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /YX /FD /GZ /c
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
# Begin Group "Day Counters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\DayCounters\actual360.h
# End Source File
# Begin Source File

SOURCE=..\Include\DayCounters\actual365.h
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\actualactual.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\DayCounters\actualactual.h
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\DayCounters\thirty360.h
# End Source File
# Begin Source File

SOURCE=..\Include\DayCounters\thirty360european.h
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360italian.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\DayCounters\thirty360italian.h
# End Source File
# End Group
# Begin Group "Calendars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Calendars\frankfurt.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\frankfurt.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\london.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\london.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\milan.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\milan.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\newyork.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\newyork.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\ortodoxcalendar.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\ortodoxcalendar.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\target.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\target.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\westerncalendar.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\westerncalendar.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\zurich.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\zurich.h
# End Source File
# End Group
# Begin Group "Term Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\TermStructures\piecewiseconstantforwards.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\TermStructures\piecewiseconstantforwards.h
# End Source File
# End Group
# Begin Group "Pricers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\Pricers\americancondition.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmamericanoption.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\bsmamericanoption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmeuropeanoption.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\bsmeuropeanoption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmnumericaloption.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\bsmnumericaloption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmoption.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\bsmoption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\dividendamericanoption.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\dividendamericanoption.h
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\dividendeuropeanoption.h
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\geometricasianoption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\mcasianpricer.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\mcasianpricer.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\mceuropeanpricer.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Pricers\mceuropeanpricer.h
# End Source File
# End Group
# Begin Group "Solvers1D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Solvers1D\bisection.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\bisection.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\brent.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\brent.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\falseposition.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\falseposition.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newton.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\newton.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newtonsafe.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\newtonsafe.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\ridder.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\ridder.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\secant.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Solvers1D\secant.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\Math\location.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Math\newcubicspline.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Math\newcubicspline.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Math\normaldistribution.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Math\normaldistribution.h
# End Source File
# Begin Source File

SOURCE=..\Include\Math\randomgenerator.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Math\statistics.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Math\statistics.h
# End Source File
# End Group
# Begin Group "Currencies"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\Currencies\aud.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\cad.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\chf.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\dem.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\dkk.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\eur.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\gbp.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\itl.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\jpy.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\sek.h
# End Source File
# Begin Source File

SOURCE=..\Include\Currencies\usd.h
# End Source File
# End Group
# Begin Group "FiniteDifferences"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\FiniteDifferences\backwardeuler.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\boundarycondition.h
# End Source File
# Begin Source File

SOURCE=..\Sources\FiniteDifferences\bsmoperator.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\bsmoperator.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\cranknicolson.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\evolver.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\finitedifferencemodel.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\forwardeuler.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\identity.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\operator.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\operatortraits.h
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\stepcondition.h
# End Source File
# Begin Source File

SOURCE=..\Sources\FiniteDifferences\tridiagonaloperator.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\FiniteDifferences\tridiagonaloperator.h
# End Source File
# End Group
# Begin Group "Instruments"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\Instruments\stock.h
# End Source File
# End Group
# Begin Group "Patterns"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\Patterns\observable.h
# End Source File
# End Group
# Begin Group "MonteCarlo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\MonteCarlo\antitheticcv.h
# End Source File
# Begin Source File

SOURCE=..\Sources\MonteCarlo\averageasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\averageasianpathpricer.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\boxmuller.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\centrallimitgaussian.h
# End Source File
# Begin Source File

SOURCE=..\Sources\MonteCarlo\controlvariatedpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\controlvariatedpathpricer.h
# End Source File
# Begin Source File

SOURCE=..\Sources\MonteCarlo\europeanpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\europeanpathpricer.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\gaussianrandomgenerator.h
# End Source File
# Begin Source File

SOURCE=..\Sources\MonteCarlo\geometricasianpathpricer.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\geometricasianpathpricer.h
# End Source File
# Begin Source File

SOURCE=..\Sources\MonteCarlo\lecuyerrandomgenerator.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\lecuyerrandomgenerator.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\mcoptionsample.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\mcpricer.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\montecarlo1d.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\path.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\pathgenerator.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\pathpricer.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\standardpathgenerator.h
# End Source File
# Begin Source File

SOURCE=..\Include\MonteCarlo\uniformrandomgenerator.h
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\Utilities\filteringiterator.h
# End Source File
# Begin Source File

SOURCE=..\Include\Utilities\stepiterator.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Include\array.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\athens.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\Calendars\athens.h
# End Source File
# Begin Source File

SOURCE=..\Sources\calendar.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\calendar.h
# End Source File
# Begin Source File

SOURCE=..\Include\currency.h
# End Source File
# Begin Source File

SOURCE=..\Sources\dataformatters.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\dataformatters.h
# End Source File
# Begin Source File

SOURCE=..\Sources\date.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\date.h
# End Source File
# Begin Source File

SOURCE=..\Include\daycounter.h
# End Source File
# Begin Source File

SOURCE=..\Include\deposit.h
# End Source File
# Begin Source File

SOURCE=..\Include\discountfactor.h
# End Source File
# Begin Source File

SOURCE=..\Include\expressiontemplates.h
# End Source File
# Begin Source File

SOURCE=..\Include\forwardvolsurface.h
# End Source File
# Begin Source File

SOURCE=..\Include\handle.h
# End Source File
# Begin Source File

SOURCE=..\Include\history.h
# End Source File
# Begin Source File

SOURCE=..\Include\instrument.h
# End Source File
# Begin Source File

SOURCE=..\Include\null.h
# End Source File
# Begin Source File

SOURCE=..\Include\options.h
# End Source File
# Begin Source File

SOURCE=..\Include\qldefines.h
# End Source File
# Begin Source File

SOURCE=..\Include\qlerrors.h
# End Source File
# Begin Source File

SOURCE=..\Include\quantlib.h
# End Source File
# Begin Source File

SOURCE=..\Include\rate.h
# End Source File
# Begin Source File

SOURCE=..\Sources\solver1d.cpp
# End Source File
# Begin Source File

SOURCE=..\Include\solver1d.h
# End Source File
# Begin Source File

SOURCE=..\Include\spread.h
# End Source File
# Begin Source File

SOURCE=..\Include\swaptionvolsurface.h
# End Source File
# Begin Source File

SOURCE=..\Include\termstructure.h
# End Source File
# End Target
# End Project
