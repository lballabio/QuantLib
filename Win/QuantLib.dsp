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
# ADD CPP /nologo /MT /W3 /GR /GX /Od /Ob2 /I "..\Sources" /I "..\Sources\PDE" /I "..\Sources\Operators" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "QL_RELEASE" /YX /FD /c
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
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "..\Sources" /I "..\Sources\PDE" /I "..\Sources\Operators" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /YX /FD /GZ /c
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
# Begin Group "Sources"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Day Counters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\DayCounters\actual360.h
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\actual365.h
# End Source File
# Begin Source File

SOURCE=..\Sources\daycounters\actualactual.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\actualactual.h
# End Source File
# Begin Source File

SOURCE=..\Sources\daycounters\thirty360.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360.h
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360european.h
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360italian.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360italian.h
# End Source File
# End Group
# Begin Group "Calendars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Calendars\frankfurt.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\frankfurt.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\london.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\london.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\milan.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\milan.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\newyork.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\newyork.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\target.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\target.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\westerncalendar.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\westerncalendar.h
# End Source File
# End Group
# Begin Group "Term Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\TermStructures\deposit.h
# End Source File
# Begin Source File

SOURCE=..\Sources\TermStructures\piecewiseconstantforwards.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\TermStructures\piecewiseconstantforwards.h
# End Source File
# End Group
# Begin Group "Operators"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Operators\blackscholesmerton.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Operators\blackscholesmerton.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Operators\tridiagonaloperator.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Operators\tridiagonaloperator.h
# End Source File
# End Group
# Begin Group "Pricers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Pricers\americancondition.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmamericanoption.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmamericanoption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmeuropeanoption.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmeuropeanoption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmnumericaloption.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmnumericaloption.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Pricers\bsmoption.h
# End Source File
# End Group
# Begin Group "Solvers1D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Solvers1D\bisection.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\bisection.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\brent.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\brent.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\falseposition.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\falseposition.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newton.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newton.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newtonsafe.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newtonsafe.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\ridder.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\ridder.h
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\secant.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\secant.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\Sources\array.h
# End Source File
# Begin Source File

SOURCE=..\Sources\calendar.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\calendar.h
# End Source File
# Begin Source File

SOURCE=..\Sources\currency.h
# End Source File
# Begin Source File

SOURCE=..\Sources\date.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\date.h
# End Source File
# Begin Source File

SOURCE=..\Sources\daycounter.h
# End Source File
# Begin Source File

SOURCE=..\Sources\discountfactor.h
# End Source File
# Begin Source File

SOURCE=..\Sources\expressiontemplates.h
# End Source File
# Begin Source File

SOURCE=..\Sources\formats.h
# End Source File
# Begin Source File

SOURCE=..\Sources\forwardvolsurface.h
# End Source File
# Begin Source File

SOURCE=..\Sources\handle.h
# End Source File
# Begin Source File

SOURCE=..\Sources\instrument.h
# End Source File
# Begin Source File

SOURCE=..\Sources\normaldistribution.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\normaldistribution.h
# End Source File
# Begin Source File

SOURCE=..\Sources\null.h
# End Source File
# Begin Source File

SOURCE=..\Sources\observable.h
# End Source File
# Begin Source File

SOURCE=..\Sources\options.h
# End Source File
# Begin Source File

SOURCE=..\Sources\qldefines.h
# End Source File
# Begin Source File

SOURCE=..\Sources\qlerrors.h
# End Source File
# Begin Source File

SOURCE=..\Sources\solver1d.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\solver1d.h
# End Source File
# Begin Source File

SOURCE=..\Sources\spread.h
# End Source File
# Begin Source File

SOURCE=..\Sources\statistics.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\statistics.h
# End Source File
# Begin Source File

SOURCE=..\Sources\stringconverters.h
# End Source File
# Begin Source File

SOURCE=..\Sources\swaptionvolsurface.h
# End Source File
# Begin Source File

SOURCE=..\Sources\termstructure.h
# End Source File
# Begin Source File

SOURCE=..\Sources\yield.h
# End Source File
# End Group
# End Target
# End Project
