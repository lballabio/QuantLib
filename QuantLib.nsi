
# $Id$
# $Source$
# $Log$
# Revision 1.19  2001/08/23 14:39:50  nando
# miscellanea
#

#############################
# to be used with NSIS 1.44 #
#############################

# HEADER CONFIGURATION COMMANDS
Name "QuantLib"
#do not change the name below
OutFile "..\QuantLib-inst.exe"
SilentInstall normal
CRCCheck on
UninstallText "This will uninstall QuantLib. Hit next to continue."
UninstallExeName "QuantLibUninstall.exe"
LicenseText "You must read the following license before installing:"
LicenseData License.txt
ComponentText "This will install QuantLib documentation on your computer:"
DirShow show
DirText "Please select a location to install QuantLib (or use the default):"
InstallDir $PROGRAMFILES\QuantLib
InstallDirRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib "Install_Dir"
AutoCloseWindow false
ShowInstDetails hide

# INSTALLATION EXECUTION COMMANDS
Section "-QuantLib"
SetOutPath $INSTDIR
File "Authors.txt"
File "Contributors.txt"
File "README.txt"
File "LICENSE.txt"
File "News.txt"
SetOutPath $INSTDIR\lib\Win32\VisualStudio
File "lib\Win32\VisualStudio\QuantLib.lib"
SetOutPath $INSTDIR\lib\Win32\Borland
File "lib\Win32\Borland\QuantLib.lib"
SetOutPath  $INSTDIR\Include\ql
File /r "Include\ql\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Calendars
File /r "Include\ql\Calendars\*.hpp"
SetOutPath  $INSTDIR\Include\ql\CashFlows
File /r "Include\ql\CashFlows\*.hpp"
SetOutPath  $INSTDIR\Include\ql\DayCounters
File /r "Include\ql\DayCounters\*.hpp"
SetOutPath  $INSTDIR\Include\ql\FiniteDifferences
File /r "Include\ql\FiniteDifferences\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Indexes
File /r "Include\ql\Indexes\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Instruments
File /r "Include\ql\Instruments\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Math
File /r "Include\ql\Math\*.hpp"
SetOutPath  $INSTDIR\Include\ql\MonteCarlo
File /r "Include\ql\MonteCarlo\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Patterns
File /r "Include\ql\Patterns\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Pricers
File /r "Include\ql\Pricers\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Solvers1D
File /r "Include\ql\Solvers1D\*.hpp"
SetOutPath  $INSTDIR\Include\ql\TermStructures
File /r "Include\ql\TermStructures\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Utilities
File /r "Include\ql\Utilities\*.hpp"
WriteRegStr HKEY_LOCAL_MACHINE \
            "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib" \
            "DisplayName" "QuantLib (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE \
            "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib" \
            "UninstallString" '"QuantLibUninstall.exe"'
WriteRegStr HKEY_LOCAL_MACHINE \
            "SOFTWARE\QuantLib" \
            "Install_Dir" "$INSTDIR"
WriteRegStr HKEY_CURRENT_USER \
            "Environment" \
            "QL_DIR" "$INSTDIR"
CreateDirectory "$SMPROGRAMS\QuantLib"
CreateShortCut "$SMPROGRAMS\QuantLib\Uninstall.lnk" \
               "$INSTDIR\QuantLibUninstall.exe" \
               "" "$INSTDIR\QuantLibUninstall.exe" 0
CreateShortCut "$SMPROGRAMS\QuantLib\Readme.txt.lnk" \
             "$INSTDIR\Readme.txt"
SectionEnd

Section "Examples"
SetOutPath $INSTDIR\Examples
File /r "Examples\*.txt"
File /r "Examples\*.mak"
File /r "Examples\*.am"
File /r "Examples\*.dsw"
SetOutPath $INSTDIR\Examples\HedgingError
File /r "Examples\HedgingError\*.cpp"
File /r "Examples\HedgingError\*.dsp"
File /r "Examples\HedgingError\*.txt"
File /r "Examples\HedgingError\*.mak"
SetOutPath $INSTDIR\Examples\Parities
File /r "Examples\Parities\*.cpp"
File /r "Examples\Parities\*.dsp"
File /r "Examples\Parities\*.txt"
File /r "Examples\Parities\*.mak"
IfFileExists $SMPROGRAMS\QuantLib 0 NoSourceShortCuts
CreateShortCut "$SMPROGRAMS\QuantLib\Examples workspace.lnk" \
               "$INSTDIR\Examples\Examples.dsw"
NoSourceShortCuts:
SectionEnd

Function .onInstSuccess
  MessageBox MB_YESNO|MB_ICONQUESTION \
             "Setup has completed. View Readme.txt now?" \
             IDNO NoReadme
    ExecShell open '$INSTDIR\Readme.txt'
  NoReadme:
FunctionEnd


Section "Uninstall"
DeleteRegKey    HKEY_LOCAL_MACHINE \
                "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib"
DeleteRegKey    HKEY_LOCAL_MACHINE SOFTWARE\QuantLib
DeleteRegValue  HKEY_CURRENT_USER  "Environment" "QL_DIR"
Delete "$SMPROGRAMS\QuantLib\*.*"
RMDir "$SMPROGRAMS\QuantLib"
RMDir /r "$INSTDIR\Examples"
RMDir /r "$INSTDIR\Sources"
RMDir /r "$INSTDIR\Docs"
RMDir /r "$INSTDIR\Include"
RMDir /r "$INSTDIR\lib"
RMDir /r "$INSTDIR"
SectionEnd
