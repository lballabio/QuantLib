
# $Id$

#############################
# to be used with NSIS 1.44 #
#############################

# HEADER CONFIGURATION COMMANDS
!ifdef LIGHT
    Name "QuantLib Light"
    Caption "QuantLib Light - Setup"
    #do not change the name below
    OutFile "..\QuantLib-light-inst.exe"
    ComponentText "This will install QuantLib Light on your computer. A more complete version including documentation, examples, source code, etc. can be downloaded from http://quantlib.org"
!else
    Name "QuantLib"
    Caption "QuantLib - Setup"
    #do not change the name below
    OutFile "..\QuantLib-full-inst.exe"

    InstType "Full (w/ Source Code)"
    InstType Normal
    InstType Minimal

    ComponentText "This will install QuantLib on your computer:"
!endif

SilentInstall normal
CRCCheck on
UninstallText "This will uninstall QuantLib. Hit next to continue."
UninstallExeName "QuantLibUninstall.exe"
LicenseText "You must read the following license before installing:"
LicenseData License.txt
DirShow show
DirText "Please select a location to install QuantLib (or use the default):"
InstallDir $PROGRAMFILES\QuantLib
InstallDirRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib "Install_Dir"
AutoCloseWindow false
ShowInstDetails hide
SetDateSave on

# INSTALLATION EXECUTION COMMANDS
Section "-QuantLib"
SectionIn 1 2 3
    CreateDirectory "$SMPROGRAMS\QuantLib"
    SetOutPath $INSTDIR
    File "Authors.txt"
    File "Contributors.txt"
    File "History.txt"
    File "README.txt"
    File "LICENSE.txt"
    File "News.txt"
    File "TODO.txt"
# it doesn't work
#    CreateShortCut "$SMPROGRAMS\QuantLib\README.txt.lnk" \
#                 "$INSTDIR\README.txt"

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
    CreateShortCut "$SMPROGRAMS\QuantLib\Uninstall.lnk" \
                   "$INSTDIR\QuantLibUninstall.exe" \
                   "" "$INSTDIR\QuantLibUninstall.exe" 0
SectionEnd

!ifndef LIGHT

Section "Source Code"
SectionIn 2
  SetOutPath $INSTDIR
  File ChangeLog.txt
  File makefile.mak
  File QuantLib.dep
  File QuantLib.dsp
  File QuantLib.dsw
  File QuantLib.mak
  File QuantLib.nsi

  SetOutPath  $INSTDIR\Sources
  File /r "Sources\*.cpp"
  File /r "Sources\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Calendars
  File /r "Sources\Calendars\*.cpp"
  File /r "Sources\Calendars\makefile.mak"

  SetOutPath  $INSTDIR\Sources\CashFlows
  File /r "Sources\CashFlows\*.cpp"
  File /r "Sources\CashFlows\makefile.mak"

  SetOutPath  $INSTDIR\Sources\DayCounters
  File /r "Sources\DayCounters\*.cpp"
  File /r "Sources\DayCounters\makefile.mak"

  SetOutPath  $INSTDIR\Sources\FiniteDifferences
  File /r "Sources\FiniteDifferences\*.cpp"
  File /r "Sources\FiniteDifferences\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Indexes
  File /r "Sources\Indexes\*.cpp"
  File /r "Sources\Indexes\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Instruments
  File /r "Sources\Instruments\*.cpp"
  File /r "Sources\Instruments\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Math
  File /r "Sources\Math\*.cpp"
  File /r "Sources\Math\makefile.mak"

  SetOutPath  $INSTDIR\Sources\MonteCarlo
  File /r "Sources\MonteCarlo\*.cpp"
  File /r "Sources\MonteCarlo\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Pricers
  File /r "Sources\Pricers\*.cpp"
  File /r "Sources\Pricers\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Solvers1D
  File /r "Sources\Solvers1D\*.cpp"
  File /r "Sources\Solvers1D\makefile.mak"

  SetOutPath  $INSTDIR\Sources\TermStructures
  File /r "Sources\TermStructures\*.cpp"
  File /r "Sources\TermStructures\makefile.mak"

  IfFileExists $SMPROGRAMS\QuantLib 0 NoSourceShortCuts
    CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib project workspace.lnk" \
                   "$INSTDIR\QuantLib.dsw"
  NoSourceShortCuts:
SectionEnd

Section "Examples"
SectionIn 1 2
    SetOutPath $INSTDIR\Examples
    File /r "Examples\*.txt"
    File /r "Examples\*.mak"
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




SectionDivider

Section "HTML documentation"
SectionIn 1 2
  SetOutPath "$INSTDIR\Docs\html"
  File "Docs\html\*.*"
  CreateShortCut "$INSTDIR\Docs\refman.html.lnk" "$INSTDIR\Docs\html\index.html"
  CreateShortCut "$SMPROGRAMS\QuantLib\Documentation (HTML).lnk" \
                 "$INSTDIR\Docs\html\index.htm"
SectionEnd

Section "PDF documentation"
SectionIn 2
  SetOutPath "$INSTDIR\Docs"
  File "Docs\latex\*refman.pdf"
  CreateShortCut "$SMPROGRAMS\QuantLib\Documentation (PDF).lnk" \
                 "$INSTDIR\Docs\refman.pdf"
SectionEnd

Section "PS documentation"
SectionIn 2
  SetOutPath "$INSTDIR\Docs"
  File "Docs\latex\*refman.ps"
  CreateShortCut "$SMPROGRAMS\QuantLib\Documentation (PS).lnk" \
                 "$INSTDIR\Docs\refman.ps"
SectionEnd

!endif

Section "Start Menu Group"
SectionIn 1 2 3
  SetOutPath $SMPROGRAMS\QuantLib
#it doesn't work
#  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Home Page.lnk" \
#                 "http://quantlib.org/index.html"
  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Directory.lnk" \
                 "$INSTDIR"
SectionEnd

#it doesn't work
#Function .onInstSuccess
#  MessageBox MB_YESNO|MB_ICONQUESTION \
#             "Setup has completed. View Readme.txt now?" \
#             IDNO NoReadme
#    ExecShell open '$INSTDIR\Readme.txt'
#  NoReadme:
#FunctionEnd


Section "Uninstall"
    DeleteRegKey HKEY_LOCAL_MACHINE \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib"
    DeleteRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib
    DeleteRegValue HKEY_CURRENT_USER  "Environment" "QL_DIR"
    Delete "$SMPROGRAMS\QuantLib\*.*"
    RMDir "$SMPROGRAMS\QuantLib"
    RMDir /r "$INSTDIR\Examples"
    RMDir /r "$INSTDIR\Sources"
    RMDir /r "$INSTDIR\Docs"
    RMDir /r "$INSTDIR\Include"
    RMDir /r "$INSTDIR\lib"
    RMDir /r "$INSTDIR"
SectionEnd
