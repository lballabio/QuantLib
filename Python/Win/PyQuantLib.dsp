# Microsoft Developer Studio Project File - Name="PyQuantLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=PyQuantLib - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PyQuantLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PyQuantLib.mak" CFG="PyQuantLib - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PyQuantLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "PyQuantLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PyQuantLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\Release"
# PROP Intermediate_Dir ".\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MD /W3 /GX /Od /I "..\..\Include" /I "..\..\Include\Calendars" /I "..\..\Include\Currencies" /I "..\..\Include\DayCounters" /I "..\..\Include\FiniteDifferences" /I "..\..\Include\Instruments" /I "..\..\Include\Math" /I "..\..\Include\MonteCarlo" /I "..\..\Include\Patterns" /I "..\..\Include\Pricers" /I "..\..\Include\Solvers1D" /I "..\..\Include\TermStructures" /I "..\..\Include\Utilities" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 python20.lib ..\..\Release\QuantLib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:".\Release/QuantLibc.dll" /export:initQuantLibc
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "PyQuantLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\Debug"
# PROP Intermediate_Dir ".\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MDd /W3 /GX /ZI /Od /I "..\..\Include" /I "..\..\Include\Calendars" /I "..\..\Include\Currencies" /I "..\..\Include\DayCounters" /I "..\..\Include\FiniteDifferences" /I "..\..\Include\Instruments" /I "..\..\Include\Math" /I "..\..\Include\MonteCarlo" /I "..\..\Include\Patterns" /I "..\..\Include\Pricers" /I "..\..\Include\Solvers1D" /I "..\..\Include\TermStructures" /I "..\..\Include\Utilities" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 python20_d.lib ..\..\Debug\QuantLib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:".\Debug/QuantLibc_d.dll" /export:initQuantLibc
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PyQuantLib - Win32 Release"
# Name "PyQuantLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\quantlib_wrap.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\PyQuantLibWrap.bat
# End Source File
# Begin Source File

SOURCE=.\QuantLib.py
# End Source File
# End Group
# Begin Group "Interfaces"

# PROP Default_Filter ".i"
# Begin Source File

SOURCE=..\..\SWIG\Barrier.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\BoundaryConditions.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Calendars.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Currencies.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Date.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\DayCounters.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Distributions.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Financial.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\History.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Instruments.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Interpolation.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Matrix.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\MontecarloPricers.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\MontecarloTools.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Operators.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Options.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Pricers.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\QLArray.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\QuantLib.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\RandomGenerators.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\RiskStatistics.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Solvers1D.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Statistics.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\String.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\TermStructures.i
# End Source File
# Begin Source File

SOURCE=..\..\SWIG\Vectors.i
# End Source File
# End Group
# End Target
# End Project
