# Microsoft Developer Studio Project File - Name="testsuite" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=testsuite - Win32 Debug SingleThread
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "testsuite.mak" CFG="testsuite - Win32 Debug SingleThread"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "testsuite - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Release MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug MTDLL" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Release SingleThread" (based on "Win32 (x86) Console Application")
!MESSAGE "testsuite - Win32 Debug SingleThread" (based on "Win32 (x86) Console Application")
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
# PROP Output_Dir "build\vc6\Release"
# PROP Intermediate_Dir "build\vc6\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I ".." /I "..\functions" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"bin\QuantLib-test-suite-vc6-mt-s-0_3_12.exe" /libpath:"..\lib"
# Begin Special Build Tool
TargetDir=.\bin
TargetName=QuantLib-test-suite-vc6-mt-s-0_3_12
SOURCE="$(InputPath)"
PostBuild_Desc=Auto run test
PostBuild_Cmds="$(TargetDir)\$(TargetName).exe" --log_level=messages --build_info=yes --result_code=no --report_level=short
# End Special Build Tool

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build\vc6\Debug"
# PROP Intermediate_Dir "build\vc6\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I ".." /I "..\functions" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"bin\QuantLib-test-suite-vc6-mt-sgd-0_3_12.exe" /pdbtype:sept /libpath:"..\lib"
# Begin Special Build Tool
TargetDir=.\bin
TargetName=QuantLib-test-suite-vc6-mt-sgd-0_3_12
SOURCE="$(InputPath)"
PostBuild_Desc=Auto run test
PostBuild_Cmds="$(TargetDir)\$(TargetName).exe" --log_level=messages --build_info=yes --result_code=no --report_level=short --catch_system_errors=no
# End Special Build Tool

!ELSEIF  "$(CFG)" == "testsuite - Win32 Release MTDLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseMTDLL"
# PROP BASE Intermediate_Dir "ReleaseMTDLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build\vc6\ReleaseMTDLL"
# PROP Intermediate_Dir "build\vc6\ReleaseMTDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /Ob2 /I ".." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /O2 /Ob2 /I ".." /I "..\functions" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes /debug /machine:I386 /out:"bin\QuantLib-test-suite-vc6-mt-0_3_12.exe" /libpath:"..\lib"
# Begin Special Build Tool
TargetDir=.\bin
TargetName=QuantLib-test-suite-vc6-mt-0_3_12
SOURCE="$(InputPath)"
PostBuild_Desc=Auto run test
PostBuild_Cmds="$(TargetDir)\$(TargetName).exe" --log_level=messages --build_info=yes --result_code=no --report_level=short
# End Special Build Tool

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug MTDLL"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugMTDLL"
# PROP BASE Intermediate_Dir "DebugMTDLL"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build\vc6\DebugMTDLL"
# PROP Intermediate_Dir "build\vc6\DebugMTDLL"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /ZI /Od /I ".." /I "..\functions" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"bin\QuantLib-test-suite-vc6-mt-gd-0_3_12.exe" /pdbtype:sept /libpath:"..\lib"
# Begin Special Build Tool
TargetDir=.\bin
TargetName=QuantLib-test-suite-vc6-mt-gd-0_3_12
SOURCE="$(InputPath)"
PostBuild_Desc=Auto run test
PostBuild_Cmds="$(TargetDir)\$(TargetName).exe" --log_level=messages --build_info=yes --result_code=no --report_level=short --catch_system_errors=no
# End Special Build Tool

!ELSEIF  "$(CFG)" == "testsuite - Win32 Release SingleThread"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseST"
# PROP BASE Intermediate_Dir "ReleaseST"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build\vc6\ReleaseST"
# PROP Intermediate_Dir "build\vc6\ReleaseST"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /Gi /GR /GX /O2 /Ob2 /I ".." /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /c
# ADD CPP /nologo /W3 /Gi /GR /GX /O2 /Ob2 /I ".." /I "..\functions" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"..\lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"bin\QuantLib-test-suite-vc6-s-0_3_12.exe" /libpath:"..\lib"
# Begin Special Build Tool
TargetDir=.\bin
TargetName=QuantLib-test-suite-vc6-s-0_3_12
SOURCE="$(InputPath)"
PostBuild_Desc=Auto run test
PostBuild_Cmds="$(TargetDir)\$(TargetName).exe" --log_level=messages --build_info=yes --result_code=no --report_level=short
# End Special Build Tool

!ELSEIF  "$(CFG)" == "testsuite - Win32 Debug SingleThread"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugST"
# PROP BASE Intermediate_Dir "DebugST"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build\vc6\DebugST"
# PROP Intermediate_Dir "build\vc6\DebugST"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /Gi /GR /GX /ZI /Od /I ".." /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /Gi /GR /GX /ZI /Od /I ".." /I "..\functions" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /Fr /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"..\lib"
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"bin\QuantLib-test-suite-vc6-sgd-0_3_12.exe" /pdbtype:sept /libpath:"..\lib"
# Begin Special Build Tool
TargetDir=.\bin
TargetName=QuantLib-test-suite-vc6-sgd-0_3_12
SOURCE="$(InputPath)"
PostBuild_Desc=Auto run test
PostBuild_Cmds="$(TargetDir)\$(TargetName).exe" --log_level=messages --build_info=yes --result_code=no --report_level=short --catch_system_errors=no
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "testsuite - Win32 Release"
# Name "testsuite - Win32 Debug"
# Name "testsuite - Win32 Release MTDLL"
# Name "testsuite - Win32 Debug MTDLL"
# Name "testsuite - Win32 Release SingleThread"
# Name "testsuite - Win32 Debug SingleThread"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\americanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\array.cpp
# End Source File
# Begin Source File

SOURCE=.\asianoptions.cpp
# End Source File
# Begin Source File

SOURCE=.\barrieroption.cpp
# End Source File
# Begin Source File

SOURCE=.\basketoption.cpp
# End Source File
# Begin Source File

SOURCE=.\batesmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\bermudanswaption.cpp
# End Source File
# Begin Source File

SOURCE=.\bonds.cpp
# End Source File
# Begin Source File

SOURCE=.\calendars.cpp
# End Source File
# Begin Source File

SOURCE=.\capfloor.cpp
# End Source File
# Begin Source File

SOURCE=.\cliquetoption.cpp
# End Source File
# Begin Source File

SOURCE=.\compoundforward.cpp
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

SOURCE=.\digitaloption.cpp
# End Source File
# Begin Source File

SOURCE=.\distributions.cpp
# End Source File
# Begin Source File

SOURCE=.\dividendoption.cpp
# End Source File
# Begin Source File

SOURCE=.\europeanoption.cpp
# End Source File
# Begin Source File

SOURCE=.\exchangerate.cpp
# End Source File
# Begin Source File

SOURCE=.\factorial.cpp
# End Source File
# Begin Source File

SOURCE=.\forwardoption.cpp
# End Source File
# Begin Source File

SOURCE=.\gaussianquadratures.cpp
# End Source File
# Begin Source File

SOURCE=.\hestonmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\instruments.cpp
# End Source File
# Begin Source File

SOURCE=.\integrals.cpp
# End Source File
# Begin Source File

SOURCE=.\interestrates.cpp
# End Source File
# Begin Source File

SOURCE=.\interpolations.cpp
# End Source File
# Begin Source File

SOURCE=.\jumpdiffusion.cpp
# End Source File
# Begin Source File

SOURCE=.\libormarketmodelprocess.cpp
# End Source File
# Begin Source File

SOURCE=.\lowdiscrepancysequences.cpp
# End Source File
# Begin Source File

SOURCE=.\matrices.cpp
# End Source File
# Begin Source File

SOURCE=.\mersennetwister.cpp
# End Source File
# Begin Source File

SOURCE=.\money.cpp
# End Source File
# Begin Source File

SOURCE=.\old_pricers.cpp
# End Source File
# Begin Source File

SOURCE=.\operators.cpp
# End Source File
# Begin Source File

SOURCE=.\pathgenerator.cpp
# End Source File
# Begin Source File

SOURCE=.\piecewiseflatforward.cpp
# End Source File
# Begin Source File

SOURCE=.\piecewiseyieldcurve.cpp
# End Source File
# Begin Source File

SOURCE=.\quantlibtestsuite.cpp
# End Source File
# Begin Source File

SOURCE=.\quantooption.cpp
# End Source File
# Begin Source File

SOURCE=.\quotes.cpp
# End Source File
# Begin Source File

SOURCE=.\riskstats.cpp
# End Source File
# Begin Source File

SOURCE=.\rngtraits.cpp
# End Source File
# Begin Source File

SOURCE=.\rounding.cpp
# End Source File
# Begin Source File

SOURCE=.\sampledcurve.cpp
# End Source File
# Begin Source File

SOURCE=.\shortratemodels.cpp
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
# Begin Source File

SOURCE=.\tqreigendecomposition.cpp
# End Source File
# Begin Source File

SOURCE=.\tracing.cpp
# End Source File
# Begin Source File

SOURCE=.\utilities.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\americanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\array.hpp
# End Source File
# Begin Source File

SOURCE=.\asianoptions.hpp
# End Source File
# Begin Source File

SOURCE=.\barrieroption.hpp
# End Source File
# Begin Source File

SOURCE=.\basketoption.hpp
# End Source File
# Begin Source File

SOURCE=.\batesmodel.hpp
# End Source File
# Begin Source File

SOURCE=.\bermudanswaption.hpp
# End Source File
# Begin Source File

SOURCE=.\bonds.hpp
# End Source File
# Begin Source File

SOURCE=.\calendars.hpp
# End Source File
# Begin Source File

SOURCE=.\capfloor.hpp
# End Source File
# Begin Source File

SOURCE=.\cliquetoption.hpp
# End Source File
# Begin Source File

SOURCE=.\compoundforward.hpp
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

SOURCE=.\digitaloption.hpp
# End Source File
# Begin Source File

SOURCE=.\distributions.hpp
# End Source File
# Begin Source File

SOURCE=.\dividendoption.hpp
# End Source File
# Begin Source File

SOURCE=.\europeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\exchangerate.hpp
# End Source File
# Begin Source File

SOURCE=.\factorial.hpp
# End Source File
# Begin Source File

SOURCE=.\forwardoption.hpp
# End Source File
# Begin Source File

SOURCE=.\gaussianquadratures.hpp
# End Source File
# Begin Source File

SOURCE=.\hestonmodel.hpp
# End Source File
# Begin Source File

SOURCE=.\instruments.hpp
# End Source File
# Begin Source File

SOURCE=.\integrals.hpp
# End Source File
# Begin Source File

SOURCE=.\interestrates.hpp
# End Source File
# Begin Source File

SOURCE=.\interpolations.hpp
# End Source File
# Begin Source File

SOURCE=.\jumpdiffusion.hpp
# End Source File
# Begin Source File

SOURCE=.\libormarketmodelprocess.hpp
# End Source File
# Begin Source File

SOURCE=.\lowdiscrepancysequences.hpp
# End Source File
# Begin Source File

SOURCE=.\matrices.hpp
# End Source File
# Begin Source File

SOURCE=.\mersennetwister.hpp
# End Source File
# Begin Source File

SOURCE=.\money.hpp
# End Source File
# Begin Source File

SOURCE=.\old_pricers.hpp
# End Source File
# Begin Source File

SOURCE=.\operators.hpp
# End Source File
# Begin Source File

SOURCE=.\pathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\piecewiseflatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\piecewiseyieldcurve.hpp
# End Source File
# Begin Source File

SOURCE=.\quantooption.hpp
# End Source File
# Begin Source File

SOURCE=.\quotes.hpp
# End Source File
# Begin Source File

SOURCE=.\riskstats.hpp
# End Source File
# Begin Source File

SOURCE=.\rngtraits.hpp
# End Source File
# Begin Source File

SOURCE=.\rounding.hpp
# End Source File
# Begin Source File

SOURCE=.\shortratemodels.hpp
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

SOURCE=.\tqreigendecomposition.hpp
# End Source File
# Begin Source File

SOURCE=.\tracing.hpp
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
