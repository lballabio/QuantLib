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
!MESSAGE "QuantLib - Win32 OnTheEdgeRelease" (based on "Win32 (x86) Static Library")
!MESSAGE "QuantLib - Win32 OnTheEdgeDebug" (based on "Win32 (x86) Static Library")
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
# PROP Output_Dir "build\Release"
# PROP Intermediate_Dir "build\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /Od /Ob2 /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR /YX /FD /c
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
# PROP Output_Dir "build\Debug"
# PROP Intermediate_Dir "build\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR /YX /FD"lib\Win32\VisualStudio\QuantLib.pbd" /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "QuantLib___Win32_OnTheEdgeRelease0"
# PROP BASE Intermediate_Dir "QuantLib___Win32_OnTheEdgeRelease0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "build\OnTheEdgeRelease"
# PROP Intermediate_Dir "build\OnTheEdgeRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /Od /Ob2 /I "Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FR /YX /FD /c
# ADD CPP /nologo /MD /W3 /Gi /GR /GX /Od /Ob2 /I ".\\" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib.lib"
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib.lib"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "QuantLib___Win32_OnTheEdgeDebug0"
# PROP BASE Intermediate_Dir "QuantLib___Win32_OnTheEdgeDebug0"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "build\OnTheEdgeDebug"
# PROP Intermediate_Dir "build\OnTheEdgeDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GR /GX /ZI /Od /I "Include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QL_DEBUG" /FR /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gi /GR /GX /ZI /Od /I ".\\" /D "_DEBUG" /D "QL_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D "NOMINMAX" /FR /YX /FD"lib\Win32\VisualStudio\QuantLib.pbd" /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib"
# ADD LIB32 /nologo /out:"lib\Win32\VisualStudio\QuantLib_d.lib"

!ENDIF 

# Begin Target

# Name "QuantLib - Win32 Release"
# Name "QuantLib - Win32 Debug"
# Name "QuantLib - Win32 OnTheEdgeRelease"
# Name "QuantLib - Win32 OnTheEdgeDebug"
# Begin Group "Calendars"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Calendars\frankfurt.cpp
DEP_CPP_FRANK=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\frankfurt.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\frankfurt.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\helsinki.cpp
DEP_CPP_HELSI=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\helsinki.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\helsinki.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\london.cpp
DEP_CPP_LONDO=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\london.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\london.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\milan.cpp
DEP_CPP_MILAN=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\milan.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\milan.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\newyork.cpp
DEP_CPP_NEWYO=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\newyork.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\newyork.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\target.cpp
DEP_CPP_TARGE=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\target.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\target.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\wellington.cpp
DEP_CPP_WELLI=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\wellington.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\wellington.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\zurich.cpp
DEP_CPP_ZURIC=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\zurich.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Calendars\zurich.hpp
# End Source File
# End Group
# Begin Group "CashFlows"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\CashFlows\cashflowvectors.cpp
DEP_CPP_CASHF=\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\CashFlows\cashflowvectors.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\CashFlows\floatingratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\scheduler.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\CashFlows\cashflowvectors.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\CashFlows\fixedratecoupon.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\CashFlows\floatingratecoupon.cpp
DEP_CPP_FLOAT=\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\floatingratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\history.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\Indexes\xibormanager.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\filteringiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\CashFlows\floatingratecoupon.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\CashFlows\simplecashflow.hpp
# End Source File
# End Group
# Begin Group "Day Counters"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\DayCounters\actual360.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\DayCounters\actual365.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\DayCounters\actualactual.cpp
DEP_CPP_ACTUA=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\DayCounters\actualactual.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\DayCounters\actualactual.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\DayCounters\thirty360.cpp
DEP_CPP_THIRT=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\DayCounters\thirty360.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\DayCounters\thirty360.hpp
# End Source File
# End Group
# Begin Group "Finite Differences"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\FiniteDifferences\americancondition.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\boundarycondition.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\bsmoperator.cpp
DEP_CPP_BSMOP=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\bsmoperator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\cranknicolson.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\dminus.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\dplus.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\dplusdminus.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\dzero.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\expliciteuler.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\fdtypedefs.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\finitedifferencemodel.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\impliciteuler.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\onefactoroperator.cpp
DEP_CPP_ONEFA=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\onefactoroperator.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\onefactoroperator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\shoutcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\stepcondition.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\tridiagonaloperator.cpp
DEP_CPP_TRIDI=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\tridiagonaloperator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\valueatcenter.cpp
DEP_CPP_VALUE=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\FiniteDifferences\valueatcenter.hpp
# End Source File
# End Group
# Begin Group "Indexes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Indexes\euribor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Indexes\gbplibor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Indexes\usdlibor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Indexes\xibor.cpp
DEP_CPP_XIBOR=\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\history.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\Indexes\xibormanager.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\filteringiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Indexes\xibor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Indexes\xibormanager.cpp
DEP_CPP_XIBORM=\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\history.hpp"\
	".\ql\Indexes\xibormanager.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\filteringiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Indexes\xibormanager.hpp
# End Source File
# End Group
# Begin Group "Instruments"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Instruments\capfloor.cpp
DEP_CPP_CAPFL=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\CashFlows\floatingratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\capfloor.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\capfloor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\plainoption.cpp
DEP_CPP_PLAIN=\
	".\ql\argsandresults.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\plainoption.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\plainoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\simpleswap.cpp
DEP_CPP_SIMPL=\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\CashFlows\cashflowvectors.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\simpleswap.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\stock.cpp
DEP_CPP_STOCK=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\stock.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\stock.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\swap.cpp
DEP_CPP_SWAP_=\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\swap.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\swaption.cpp
DEP_CPP_SWAPT=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\CashFlows\floatingratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\exercise.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\Instruments\swaption.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Instruments\swaption.hpp
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Math\cubicspline.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\interpolation.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\lexicographicalview.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\linearinterpolation.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\matrix.cpp
DEP_CPP_MATRI=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\symmetricschurdecomposition.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Math\matrix.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\multivariateaccumulator.cpp
DEP_CPP_MULTI=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\multivariateaccumulator.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Math\multivariateaccumulator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\normaldistribution.cpp
DEP_CPP_NORMA=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Math\normaldistribution.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\riskmeasures.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\segmentintegral.cpp
DEP_CPP_SEGME=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\Math\segmentintegral.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Math\segmentintegral.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\statistics.cpp
DEP_CPP_STATI=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Math\statistics.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\symmetriceigenvalues.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Math\symmetricschurdecomposition.cpp
DEP_CPP_SYMME=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\symmetricschurdecomposition.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Math\symmetricschurdecomposition.hpp
# End Source File
# End Group
# Begin Group "MonteCarlo"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\MonteCarlo\arithmeticapopathpricer.cpp
DEP_CPP_ARITH=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\arithmeticapopathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\arithmeticapopathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\arithmeticasopathpricer.cpp
DEP_CPP_ARITHM=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\arithmeticasopathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\arithmeticasopathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\basketpathpricer.cpp
DEP_CPP_BASKE=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\basketpathpricer.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\basketpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\europeanpathpricer.cpp
DEP_CPP_EUROP=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\europeanpathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\europeanpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\everestpathpricer.cpp
DEP_CPP_EVERE=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\everestpathpricer.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\everestpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\geometricapopathpricer.cpp
DEP_CPP_GEOME=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\geometricapopathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\geometricapopathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\geometricasopathpricer.cpp
DEP_CPP_GEOMET=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\geometricasopathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\geometricasopathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\getcovariance.cpp
DEP_CPP_GETCO=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\MonteCarlo\getcovariance.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\getcovariance.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\himalayapathpricer.cpp
DEP_CPP_HIMAL=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\himalayapathpricer.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\himalayapathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\maxbasketpathpricer.cpp
DEP_CPP_MAXBA=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\maxbasketpathpricer.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\maxbasketpathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\mctypedefs.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\montecarlomodel.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\multipath.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\multipathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\pagodapathpricer.cpp
DEP_CPP_PAGOD=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\pagodapathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\pagodapathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\path.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\pathgenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\pathpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\MonteCarlo\sample.hpp
# End Source File
# End Group
# Begin Group "Patterns"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Patterns\observable.hpp
# End Source File
# End Group
# Begin Group "Pricers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Pricers\analyticalcapfloor.cpp
DEP_CPP_ANALY=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\capfloor.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\analyticalcapfloor.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\analyticalcapfloor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\barrieroption.cpp
DEP_CPP_BARRI=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\barrieroption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\barrieroption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\binaryoption.cpp
DEP_CPP_BINAR=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\binaryoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\binaryoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\cliquetoption.cpp
DEP_CPP_CLIQU=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\cliquetoption.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\cliquetoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\continuousgeometricapo.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\discretegeometricapo.cpp
DEP_CPP_DISCR=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\discretegeometricapo.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\discretegeometricapo.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\discretegeometricaso.cpp
DEP_CPP_DISCRE=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\discretegeometricaso.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\discretegeometricaso.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\europeanengine.cpp
DEP_CPP_EUROPE=\
	".\ql\argsandresults.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\plainoption.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanengine.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\europeanengine.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\europeanoption.cpp
DEP_CPP_EUROPEA=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\europeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdamericanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdbermudanoption.cpp
DEP_CPP_FDBER=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\fdbermudanoption.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fdmultiperiodoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdbermudanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdbsmoption.cpp
DEP_CPP_FDBSM=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\FiniteDifferences\valueatcenter.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdbsmoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendamericanoption.cpp
DEP_CPP_FDDIV=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fddividendamericanoption.hpp"\
	".\ql\Pricers\fddividendoption.hpp"\
	".\ql\Pricers\fdmultiperiodoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendamericanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendeuropeanoption.cpp
DEP_CPP_FDDIVI=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\fddividendeuropeanoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendeuropeanoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendoption.cpp
DEP_CPP_FDDIVID=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\FiniteDifferences\valueatcenter.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\cubicspline.hpp"\
	".\ql\Math\interpolation.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fddividendeuropeanoption.hpp"\
	".\ql\Pricers\fddividendoption.hpp"\
	".\ql\Pricers\fdmultiperiodoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendshoutoption.cpp
DEP_CPP_FDDIVIDE=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\shoutcondition.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fddividendoption.hpp"\
	".\ql\Pricers\fddividendshoutoption.hpp"\
	".\ql\Pricers\fdmultiperiodoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fddividendshoutoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdeuropean.cpp
DEP_CPP_FDEUR=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\FiniteDifferences\valueatcenter.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fdeuropean.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdeuropean.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdmultiperiodoption.cpp
DEP_CPP_FDMUL=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\americancondition.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\FiniteDifferences\valueatcenter.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fdmultiperiodoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdmultiperiodoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdshoutoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdstepconditionoption.cpp
DEP_CPP_FDSTE=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\bsmoperator.hpp"\
	".\ql\FiniteDifferences\cranknicolson.hpp"\
	".\ql\FiniteDifferences\fdtypedefs.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\FiniteDifferences\valueatcenter.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\europeanoption.hpp"\
	".\ql\Pricers\fdbsmoption.hpp"\
	".\ql\Pricers\fdstepconditionoption.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\fdstepconditionoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\jamshidianswaption.cpp
DEP_CPP_JAMSH=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\exercise.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\Instruments\swaption.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\jamshidianswaption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\jamshidianswaption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcbasket.cpp
DEP_CPP_MCBAS=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\basketpathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\mcbasket.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcbasket.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcdiscretearithmeticapo.cpp
DEP_CPP_MCDIS=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\arithmeticapopathpricer.hpp"\
	".\ql\MonteCarlo\geometricapopathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\discretegeometricapo.hpp"\
	".\ql\Pricers\mcdiscretearithmeticapo.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcdiscretearithmeticapo.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcdiscretearithmeticaso.cpp
DEP_CPP_MCDISC=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\arithmeticasopathpricer.hpp"\
	".\ql\MonteCarlo\geometricasopathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\discretegeometricaso.hpp"\
	".\ql\Pricers\mcdiscretearithmeticaso.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcdiscretearithmeticaso.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mceuropean.cpp
DEP_CPP_MCEUR=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\europeanpathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\mceuropean.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mceuropean.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mceverest.cpp
DEP_CPP_MCEVE=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\everestpathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\mceverest.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mceverest.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mchimalaya.cpp
DEP_CPP_MCHIM=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\himalayapathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\mchimalaya.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mchimalaya.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcmaxbasket.cpp
DEP_CPP_MCMAX=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\maxbasketpathpricer.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\mcmaxbasket.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcmaxbasket.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcpagoda.cpp
DEP_CPP_MCPAG=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\Math\statistics.hpp"\
	".\ql\MonteCarlo\mctypedefs.hpp"\
	".\ql\MonteCarlo\montecarlomodel.hpp"\
	".\ql\MonteCarlo\multipath.hpp"\
	".\ql\MonteCarlo\multipathgenerator.hpp"\
	".\ql\MonteCarlo\pagodapathpricer.hpp"\
	".\ql\MonteCarlo\path.hpp"\
	".\ql\MonteCarlo\pathgenerator.hpp"\
	".\ql\MonteCarlo\pathpricer.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\mcpagoda.hpp"\
	".\ql\Pricers\mcpricer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\boxmullergaussianrng.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	".\ql\RandomNumbers\randomarraygenerator.hpp"\
	".\ql\RandomNumbers\rngtypedefs.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcpagoda.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\mcpricer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\singleassetoption.cpp
DEP_CPP_SINGL=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\singleassetoption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\singleassetoption.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\treecapfloor.cpp
DEP_CPP_TREEC=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\capfloor.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\treecapfloor.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\treecapfloor.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\treeswaption.cpp
DEP_CPP_TREES=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\exercise.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\Instruments\swaption.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\treeswaption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Pricers\treeswaption.hpp
# End Source File
# End Group
# Begin Group "RandomNumbers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\RandomNumbers\boxmullergaussianrng.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\centrallimitgaussianrng.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\inversecumulativegaussianrng.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\knuthuniformrng.cpp
DEP_CPP_KNUTH=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\knuthuniformrng.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\knuthuniformrng.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\lecuyeruniformrng.cpp
DEP_CPP_LECUY=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\MonteCarlo\sample.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\RandomNumbers\lecuyeruniformrng.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\lecuyeruniformrng.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\randomarraygenerator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\RandomNumbers\rngtypedefs.hpp
# End Source File
# End Group
# Begin Group "Solvers 1D"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Solvers1D\bisection.cpp
DEP_CPP_BISEC=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\bisection.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\bisection.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\brent.cpp
DEP_CPP_BRENT=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\brent.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\falseposition.cpp
DEP_CPP_FALSE=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\falseposition.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\falseposition.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\newton.cpp
DEP_CPP_NEWTO=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\newton.hpp"\
	".\ql\Solvers1D\newtonsafe.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\newton.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\newtonsafe.cpp
DEP_CPP_NEWTON=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\newtonsafe.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\newtonsafe.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\ridder.cpp
DEP_CPP_RIDDE=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\ridder.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\ridder.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\secant.cpp
DEP_CPP_SECAN=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\secant.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Solvers1D\secant.hpp
# End Source File
# End Group
# Begin Group "Term Structures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\TermStructures\flatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\TermStructures\piecewiseflatforward.cpp
DEP_CPP_PIECE=\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\TermStructures\piecewiseflatforward.hpp"\
	".\ql\TermStructures\ratehelpers.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\TermStructures\piecewiseflatforward.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\TermStructures\ratehelpers.cpp
DEP_CPP_RATEH=\
	".\ql\calendar.hpp"\
	".\ql\Calendars\target.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\DayCounters\actual360.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\euribor.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\TermStructures\ratehelpers.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\TermStructures\ratehelpers.hpp
# End Source File
# End Group
# Begin Group "Utilities"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Utilities\combiningiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Utilities\couplingiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Utilities\filteringiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Utilities\iteratorcategories.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Utilities\processingiterator.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Utilities\steppingiterator.hpp
# End Source File
# End Group
# Begin Group "InterestRateModellling"

# PROP Default_Filter ""
# Begin Group "OneFactorModels"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\InterestRateModelling\OneFactorModels\blackkarasinski.cpp
DEP_CPP_BLACK=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\OneFactorModels\blackkarasinski.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\timefunction.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\cubicspline.hpp"\
	".\ql\Math\interpolation.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\OneFactorModels\blackkarasinski.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\OneFactorModels\coxingersollrossplus.cpp
DEP_CPP_COXIN=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\OneFactorModels\coxingersollrossplus.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\OneFactorModels\coxingersollrossplus.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\OneFactorModels\hullwhite.cpp
DEP_CPP_HULLW=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\OneFactorModels\hullwhite.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\OneFactorModels\hullwhite.hpp
# End Source File
# End Group
# Begin Group "CalibrationHelpers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\InterestRateModelling\CalibrationHelpers\caphelper.cpp
DEP_CPP_CAPHE=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\CashFlows\floatingratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\capfloor.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\InterestRateModelling\calibrationhelper.hpp"\
	".\ql\InterestRateModelling\CalibrationHelpers\caphelper.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\analyticalcapfloor.hpp"\
	".\ql\Pricers\treecapfloor.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\CalibrationHelpers\caphelper.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\CalibrationHelpers\swaptionhelper.cpp
DEP_CPP_SWAPTI=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\cashflow.hpp"\
	".\ql\cashflows\coupon.hpp"\
	".\ql\CashFlows\fixedratecoupon.hpp"\
	".\ql\CashFlows\floatingratecoupon.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\exercise.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\index.hpp"\
	".\ql\Indexes\xibor.hpp"\
	".\ql\instrument.hpp"\
	".\ql\Instruments\simpleswap.hpp"\
	".\ql\Instruments\swap.hpp"\
	".\ql\Instruments\swaption.hpp"\
	".\ql\InterestRateModelling\calibrationhelper.hpp"\
	".\ql\InterestRateModelling\CalibrationHelpers\swaptionhelper.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\Pricers\jamshidianswaption.hpp"\
	".\ql\Pricers\treeswaption.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\CalibrationHelpers\swaptionhelper.hpp
# End Source File
# End Group
# Begin Group "TwoFactorModels"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\InterestRateModelling\TwoFactorModels\g2.cpp
DEP_CPP_G2_CP=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\twofactormodel.hpp"\
	".\ql\InterestRateModelling\TwoFactorModels\g2.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\normaldistribution.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\TwoFactorModels\g2.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ql\InterestRateModelling\calibrationhelper.cpp
DEP_CPP_CALIB=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\calibrationhelper.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\calibrationhelper.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\grid.cpp
DEP_CPP_GRID_=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\grid.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\grid.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\model.cpp
DEP_CPP_MODEL=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\calibrationhelper.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\matrix.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\Optimization\armijo.hpp"\
	".\ql\Optimization\conjugategradient.hpp"\
	".\ql\Optimization\costfunction.hpp"\
	".\ql\Optimization\criteria.hpp"\
	".\ql\Optimization\leastsquare.hpp"\
	".\ql\Optimization\linesearch.hpp"\
	".\ql\Optimization\optimizer.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	".\ql\Utilities\steppingiterator.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\model.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\onefactormodel.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\shortrateprocess.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\timefunction.cpp
DEP_CPP_TIMEF=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\FiniteDifferences\boundarycondition.hpp"\
	".\ql\FiniteDifferences\expliciteuler.hpp"\
	".\ql\FiniteDifferences\finitedifferencemodel.hpp"\
	".\ql\FiniteDifferences\onefactoroperator.hpp"\
	".\ql\FiniteDifferences\stepcondition.hpp"\
	".\ql\FiniteDifferences\tridiagonaloperator.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\grid.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\onefactormodel.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\timefunction.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\Math\cubicspline.hpp"\
	".\ql\Math\interpolation.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\timefunction.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\trinomialtree.cpp
DEP_CPP_TRINO=\
	".\ql\argsandresults.hpp"\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\constraint.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\diffusionprocess.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\InterestRateModelling\model.hpp"\
	".\ql\InterestRateModelling\shortrateprocess.hpp"\
	".\ql\InterestRateModelling\trinomialtree.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\minimizer.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\Solvers1D\brent.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\trinomialtree.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\InterestRateModelling\twofactormodel.hpp
# End Source File
# End Group
# Begin Group "Optimization"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Optimization\armijo.cpp
DEP_CPP_ARMIJ=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\Optimization\armijo.hpp"\
	".\ql\Optimization\costfunction.hpp"\
	".\ql\Optimization\criteria.hpp"\
	".\ql\Optimization\linesearch.hpp"\
	".\ql\Optimization\optimizer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\armijo.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\conjugategradient.cpp
DEP_CPP_CONJU=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\Optimization\armijo.hpp"\
	".\ql\Optimization\conjugategradient.hpp"\
	".\ql\Optimization\costfunction.hpp"\
	".\ql\Optimization\criteria.hpp"\
	".\ql\Optimization\linesearch.hpp"\
	".\ql\Optimization\optimizer.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\conjugategradient.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\costfunction.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\criteria.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\leastsquare.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\linesearch.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\optimizer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\steepestdescent.cpp
DEP_CPP_STEEP=\
	".\ql\array.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\Optimization\armijo.hpp"\
	".\ql\Optimization\costfunction.hpp"\
	".\ql\Optimization\criteria.hpp"\
	".\ql\Optimization\linesearch.hpp"\
	".\ql\Optimization\optimizer.hpp"\
	".\ql\Optimization\steepestdescent.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Optimization\steepestdescent.hpp
# End Source File
# End Group
# Begin Group "Lattices"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\ql\Lattices\node.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Lattices\timegrid.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\Lattices\tree.cpp
DEP_CPP_TREE_=\
	".\ql\array.hpp"\
	".\ql\asset.hpp"\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\date.hpp"\
	".\ql\daycounter.hpp"\
	".\ql\errors.hpp"\
	".\ql\expressiontemplates.hpp"\
	".\ql\handle.hpp"\
	".\ql\Lattices\node.hpp"\
	".\ql\Lattices\timegrid.hpp"\
	".\ql\Lattices\tree.hpp"\
	".\ql\marketelement.hpp"\
	".\ql\null.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\relinkablehandle.hpp"\
	".\ql\termstructure.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\Lattices\tree.hpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\ql\argsandresults.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\array.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\asset.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\calendar.cpp
DEP_CPP_CALEN=\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\calendar.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\cashflow.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\config.ansi.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\config.bcc.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\config.decc.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\config.msvc.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\config.mwcw.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\constraint.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\currency.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\dataformatters.cpp
DEP_CPP_DATAF=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\dataformatters.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\date.cpp
DEP_CPP_DATE_=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\date.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\daycounter.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\diffusionprocess.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\errors.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\exercise.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\expressiontemplates.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\forwardvolsurface.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\handle.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\history.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\index.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\instrument.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\marketelement.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\minimizer.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\null.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\option.cpp
DEP_CPP_OPTIO=\
	".\ql\argsandresults.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\instrument.hpp"\
	".\ql\null.hpp"\
	".\ql\option.hpp"\
	".\ql\Patterns\observable.hpp"\
	".\ql\qldefines.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\option.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\qldefines.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\quantlib.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\relinkablehandle.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\riskstatistics.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\scheduler.cpp
DEP_CPP_SCHED=\
	".\ql\calendar.hpp"\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\handle.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\scheduler.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\scheduler.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\solver1d.cpp
DEP_CPP_SOLVE=\
	".\ql\config.bcc.hpp"\
	".\ql\config.msvc.hpp"\
	".\ql\config.mwcw.hpp"\
	".\ql\currency.hpp"\
	".\ql\dataformatters.hpp"\
	".\ql\date.hpp"\
	".\ql\errors.hpp"\
	".\ql\null.hpp"\
	".\ql\qldefines.hpp"\
	".\ql\solver1d.hpp"\
	".\ql\types.hpp"\
	

!IF  "$(CFG)" == "QuantLib - Win32 Release"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 Debug"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeRelease"

!ELSEIF  "$(CFG)" == "QuantLib - Win32 OnTheEdgeDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ql\solver1d.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\swaptionvolsurface.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\termstructure.hpp
# End Source File
# Begin Source File

SOURCE=.\ql\types.hpp
# End Source File
# End Target
# End Project
