
# HEADER CONFIGURATION COMMANDS
Name "QuantLib"
OutFile "QuantLib-inst.exe"
SilentInstall normal
CRCCheck on
UninstallText "This will uninstall QuantLib. Hit next to continue."
UninstallExeName "QuantLibUninstall.exe"
LicenseText "This installer will install QuantLib. Please read the license below."
LicenseData ..\QuantLib\License.txt
ComponentText "Select which optional components you want installed."
DirShow show
DirText "Choose a directory to install into:"
InstallDir $PROGRAMFILES\QuantLib
InstallDirRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib "Install_Dir"
AutoCloseWindow false
ShowInstDetails hide

# INSTALLATION EXECUTION COMMANDS
Section "-QuantLib"
SetOutPath $INSTDIR
File "..\QuantLib\README.txt"
File "..\QuantLib\LICENSE.txt"
File "..\QuantLib\Contributors.txt"
File "..\QuantLib\Authors.txt"
SetOutPath $INSTDIR\lib\Win32\VisualStudio
File "..\QuantLib\lib\Win32\VisualStudio\*.lib"
SetOutPath $INSTDIR\lib\Win32\Borland
File "..\QuantLib\lib\Win32\Borland\*.lib"
SetOutPath  $INSTDIR\Include\ql
File /r "..\QuantLib\Include\ql\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Calendars
File /r "..\QuantLib\Include\ql\Calendars\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Currencies
File /r "..\QuantLib\Include\ql\Currencies\*.hpp"
SetOutPath  $INSTDIR\Include\ql\DayCounters
File /r "..\QuantLib\Include\ql\DayCounters\*.hpp"
SetOutPath  $INSTDIR\Include\ql\FiniteDifferences
File /r "..\QuantLib\Include\ql\FiniteDifferences\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Instruments
File /r "..\QuantLib\Include\ql\Instruments\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Math
File /r "..\QuantLib\Include\ql\Math\*.hpp"
SetOutPath  $INSTDIR\Include\ql\MonteCarlo
File /r "..\QuantLib\Include\ql\MonteCarlo\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Patterns
File /r "..\QuantLib\Include\ql\Patterns\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Pricers
File /r "..\QuantLib\Include\ql\Pricers\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Solvers1D
File /r "..\QuantLib\Include\ql\Solvers1D\*.hpp"
SetOutPath  $INSTDIR\Include\ql\TermStructures
File /r "..\QuantLib\Include\ql\TermStructures\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Utilities
File /r "..\QuantLib\Include\ql\Utilities\*.hpp"

Section "SWIG wrapper files"
SetOutPath $INSTDIR\Swig
File /r "..\QuantLib\Swig\*.i"

Section "Documentation"
SetOutPath $INSTDIR\Docs\html
;File "..\QuantLib\Docs\html\*.*"
SetOutPath $INSTDIR\Docs\pdf
;File "..\QuantLib\Docs\latex\*.pdf"

;Section "Examples"
;SetOutPath $INSTDIR\Examples
;File "..\QuantLib\Examples\*.cpp"
;File "..\QuantLib\Examples\*.hpp"

Section "Sources"
SetOutPath $INSTDIR\Sources
File /r "..\QuantLib\Sources\*.cpp"
SetOutPath $INSTDIR\Sources\Calendars
File /r "..\QuantLib\Sources\Calendars\*.cpp"
SetOutPath $INSTDIR\Sources\DayCounters
File /r "..\QuantLib\Sources\DayCounters\*.cpp"
SetOutPath $INSTDIR\Sources\FiniteDifferences
File /r "..\QuantLib\Sources\FiniteDifferences\*.cpp"
SetOutPath $INSTDIR\Sources\Math
File /r "..\QuantLib\Sources\Math\*.cpp"
SetOutPath $INSTDIR\Sources\MonteCarlo
File /r "..\QuantLib\Sources\MonteCarlo\*.cpp"
SetOutPath $INSTDIR\Sources\Pricers
File /r "..\QuantLib\Sources\Pricers\*.cpp"
SetOutPath $INSTDIR\Sources\Solvers1D
File /r "..\QuantLib\Sources\Solvers1D\*.cpp"
SetOutPath $INSTDIR\Sources\TermStructures
File /r "..\QuantLib\Sources\TermStructures\*.cpp"

Section "Start Menu Shortcuts"
CreateDirectory "$SMPROGRAMS\QuantLib"
CreateShortCut "$SMPROGRAMS\QuantLib\Uninstall.lnk" "$INSTDIR\QuantLibUninstall.exe" "" "$INSTDIR\QuantLibUninstall.exe" 0
;CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib (notepad).lnk" "$INSTDIR\notepad.exe" "" "$INSTDIR\notepad.exe" 0

Section "Uninstall"
; remove registry keys
DeleteRegKey HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib"
DeleteRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib
; remove shortcuts, if any.
Delete "$SMPROGRAMS\QuantLib\*.*"
; remove directories used.
RMDir "$SMPROGRAMS\QuantLib"
RMDir /r $INSTDIR\Docs
RMDir /r $INSTDIR\Examples
RMDir /r $INSTDIR\Include
RMDir /r $INSTDIR\Sources
RMDir /r $INSTDIR\lib
RMDir /r "$INSTDIR"

WriteRegStr HKEY_LOCAL_MACHINE SOFTWARE\QuantLib "Install_Dir" "$INSTDIR"
; Write the uninstall keys for Windows
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib" "DisplayName" "QuantLib (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib" "UninstallString" '"QuantLibUninstall.exe"'

