# Microsoft Developer Studio Project File - Name="testsuite" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=testsuite - Win32 OnTheEdgeDebug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak" CFG="testsuite - Win32 OnTheEdgeDebug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testsuite - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "testsuite - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build\Release"
# PROP Intermediate_Dir "build\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "$(QL_DIR)" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 cppunit.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\"

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build\Debug"
# PROP Intermediate_Dir "build\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "$(QL_DIR)" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cppunitd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunitd.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"$(QL_DIR)\lib\Win32\VisualStudio\\"

!ELSEIF  "$(CFG)" == "testsuite - Win32 OnTheEdgeDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "OnTheEdgeDebug"
# PROP BASE Intermediate_Dir "OnTheEdgeDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build\OnTheEdgeDebug"
# PROP Intermediate_Dir "build\OnTheEdgeDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cppunitd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\lib\Win32\VisualStudio\\"

!ELSEIF  "$(CFG)" == "testsuite - Win32 OnTheEdgeRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "OnTheEdgeRelease"
# PROP BASE Intermediate_Dir "OnTheEdgeRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build\OnTheEdgeRelease"
# PROP Intermediate_Dir "build\OnTheEdgeRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "NOMINMAX" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 cppunit.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib cppunit.lib /nologo /subsystem:console /machine:I386 /libpath:"..\lib\Win32\VisualStudio\\"

!ENDIF 

# Begin Target

# Name "testsuite - Win32 Release"
# Name "testsuite - Win32 Debug"
# Name "testsuite - Win32 OnTheEdgeDebug"
# Name "testsuite - Win32 OnTheEdgeRelease"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\calendars.cpp
# End Source File
# Begin Source File

SOURCE=.\capfloor.cpp
# End Source File
# Begin Source File

SOURCE=.\covariance.cpp
# End Source File
# Begin Source File

SOURCE=.\dates.cpp
# End Source File
# Begin Source File

SOURCE=.\daycounters.cpp
# End Source File
# Begin Source File

SOURCE=.\distributions.cpp
# End Source File
# Begin Source File

SOURCE=.\europeanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\instruments.cpp
# End Source File
# Begin Source File

SOURCE=.\integrals.cpp
# End Source File
# Begin Source File

SOURCE=.\marketelements.cpp
# End Source File
# Begin Source File

SOURCE=.\mersennetwister.cpp
# End Source File
# Begin Source File

SOURCE=.\old_pricers.cpp
# End Source File
# Begin Source File

SOURCE=.\operators.cpp
# End Source File
# Begin Source File

SOURCE=.\piecewiseflatforward.cpp
# End Source File
# Begin Source File

SOURCE=.\qltestlistener.cpp
# End Source File
# Begin Source File

SOURCE=.\quantlibtestsuite.cpp
# End Source File
# Begin Source File

SOURCE=.\riskstats.cpp
# End Source File
# Begin Source File

SOURCE=.\solvers.cpp
# End Source File
# Begin Source File

SOURCE=.\stats.cpp
# End Source File
# Begin Source File

SOURCE=.\swap.cpp
# End Source File
# Begin Source File

SOURCE=.\swaption.cpp
# End Source File
# Begin Source File

SOURCE=.\termstructures.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\calendars.hpp
# End Source File
# Begin Source File

SOURCE=.\capfloor.hpp
# End Source File
# Begin Source File

SOURCE=.\covariance.hpp
# End Source File
# Begin Source File

SOURCE=.\dates.hpp
# End Source File
# Begin Source File

SOURCE=.\daycounters.hpp
# End Source File
# Begin Source File

SOURCE=.\distributions.hpp
# End Source File
# Begin Source File

SOURCE=.\europeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\instruments.hpp
# End Source File
# Begin Source File

SOURCE=.\integrals.hpp
# End Source File
# Begin Source File

SOURCE=.\marketelements.hpp
# End Source File
# Begin Source File

SOURCE=.\mersennetwister.hpp
# End Source File
# Begin Source File

SOURCE=.\old_pricers.hpp
# End Source File
# Begin Source File

SOURCE=.\operators.hpp
# End Source File
# Begin Source File

SOURCE=.\piecewiseflatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\qltestlistener.hpp
# End Source File
# Begin Source File

SOURCE=.\riskstats.hpp
# End Source File
# Begin Source File

SOURCE=.\solvers.hpp
# End Source File
# Begin Source File

SOURCE=.\stats.hpp
# End Source File
# Begin Source File

SOURCE=.\swap.hpp
# End Source File
# Begin Source File

SOURCE=.\swaption.hpp
# End Source File
# Begin Source File

SOURCE=.\termstructures.hpp
# End Source File
# Begin Source File

SOURCE=.\utilities.hpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
