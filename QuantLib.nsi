
# $Id$
# $Source$
# $Log$
# Revision 1.10  2001/05/22 14:43:19  nando
# comment added
#
# Revision 1.9  2001/05/22 13:43:23  nando
# 80 columns enforced
#
# Revision 1.8  2001/05/21 13:12:44  nando
# upgraded to NSIS 1.41
#
# Revision 1.7  2001/05/11 07:29:56  nando
# added CVS tag.
# Swig files are back (for a while)
#

# HEADER CONFIGURATION COMMANDS
Name "QuantLib"
#do not change the name below
OutFile "..\QuantLib-inst.exe"
SilentInstall normal
CRCCheck on
UninstallText "This will uninstall QuantLib. Hit next to continue."
UninstallExeName "QuantLibUninstall.exe"
LicenseText "This installer will install QuantLib. Please read the license."
LicenseData License.txt
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
File "Authors.txt"
File "Contributors.txt"
File "README.txt"
File "LICENSE.txt"
File "News.txt"
SetOutPath $INSTDIR\lib\Win32\VisualStudio
File "lib\Win32\VisualStudio\*.lib"
SetOutPath $INSTDIR\lib\Win32\Borland
File "lib\Win32\Borland\*.lib"
SetOutPath  $INSTDIR\Include\ql
File /r "Include\ql\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Calendars
File /r "Include\ql\Calendars\*.hpp"
SetOutPath  $INSTDIR\Include\ql\Currencies
File /r "Include\ql\Currencies\*.hpp"
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
            "SOFTWARE\QuantLib" \
            "Install_Dir" \
            "$INSTDIR"
WriteRegStr HKEY_LOCAL_MACHINE \
            "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib" \
            "DisplayName" \
            "QuantLib (remove only)"
WriteRegStr HKEY_LOCAL_MACHINE \
            "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib" \
            "UninstallString" \
            '"QuantLibUninstall.exe"'
SectionEnd

Section "Documentation"
SetOutPath $INSTDIR\Docs\html
File "Docs\html\*.*"
SetOutPath $INSTDIR\Docs\pdf
File "Docs\latex\refma*.pdf"
SectionEnd

Section "Start Menu Shortcuts"
CreateDirectory "$SMPROGRAMS\QuantLib"
CreateShortCut "$SMPROGRAMS\QuantLib\Uninstall.lnk" \
               "$INSTDIR\QuantLibUninstall.exe" \
               "" "$INSTDIR\QuantLibUninstall.exe" 0
SectionEnd

Section "Uninstall"
DeleteRegKey HKEY_LOCAL_MACHINE \
             "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib"
DeleteRegKey HKEY_LOCAL_MACHINE \
             SOFTWARE\QuantLib
Delete "$SMPROGRAMS\QuantLib\*.*"
RMDir "$SMPROGRAMS\QuantLib"
RMDir /r $INSTDIR\Docs
RMDir /r $INSTDIR\Include
RMDir /r $INSTDIR\lib
RMDir /r "$INSTDIR"
SectionEnd
