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
# ADD CPP /nologo /MT /W3 /GR /GX /Od /Ob2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "QL_RELEASE" /YX /FD /c
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
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /YX /FD /GZ /c
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

SOURCE=..\Sources\daycounters\actualactual.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\daycounters\thirty360.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\DayCounters\thirty360italian.cpp
# ADD CPP /I "..\Sources"
# End Source File
# End Group
# Begin Group "Calendars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Calendars\frankfurt.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\london.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\milan.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\newyork.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\target.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Calendars\westerncalendar.cpp
# ADD CPP /I "..\Sources"
# End Source File
# End Group
# Begin Group "Term Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\TermStructures\piecewiseconstantforwards.cpp
# ADD CPP /I "..\Sources"
# End Source File
# End Group
# Begin Group "Operators"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Operators\blackscholesmerton.cpp
# ADD CPP /I "..\Sources" /I "..\Sources\PDE"
# End Source File
# Begin Source File

SOURCE=..\Sources\Operators\tridiagonaloperator.cpp
# ADD CPP /I "..\Sources" /I "..\Sources\PDE"
# End Source File
# End Group
# Begin Group "Pricers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Pricers\bsmeuropeanoption.cpp
# ADD CPP /I "..\Sources" /I "..\Sources\Operators" /I "..\Sources\PDE"
# End Source File
# End Group
# Begin Group "Solvers1D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Sources\Solvers1D\bisection.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\brent.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\falseposition.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newton.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\newtonsafe.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\ridder.cpp
# ADD CPP /I "..\Sources"
# End Source File
# Begin Source File

SOURCE=..\Sources\Solvers1D\secant.cpp
# ADD CPP /I "..\Sources"
# End Source File
# End Group
# Begin Source File

SOURCE=..\Sources\calendar.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\date.cpp
# End Source File
# Begin Source File

SOURCE=..\Sources\solver1d.cpp
# End Source File
# End Group
# End Target
# End Project
