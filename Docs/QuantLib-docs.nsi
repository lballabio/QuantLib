
# $Id$
# $Source$
# $Log$
# Revision 1.5  2001/08/23 15:13:18  nando
# improving ....
#
# Revision 1.4  2001/08/23 14:39:50  nando
# miscellanea
#
# Revision 1.3  2001/08/23 13:04:08  nando
# added link to index.html


#############################
# to be used with NSIS 1.44 #
#############################

# HEADER CONFIGURATION COMMANDS
Name "QuantLib-docs"
Caption "QuantLib documentation - Setup"
#do not change the name below
OutFile "..\..\QuantLib-docs-inst.exe"
SilentInstall normal
CRCCheck on
LicenseText "You must read the following license before installing:"
LicenseData "..\License.txt"
ComponentText "This will install QuantLib documentation on your computer:"
DirShow show
DirText "Please select a location to install QuantLib documentation (or use the default):"
InstallDir $PROGRAMFILES\QuantLib
InstallDirRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib "Install_Dir"
AutoCloseWindow false
ShowInstDetails hide

# INSTALLATION EXECUTION COMMANDS
Section "QuantLib HTML documentation"
  SetOutPath "$INSTDIR\Docs\html"
  File "html\*.*"
  CreateShortCut "$INSTDIR\Docs\refman.html.lnk" "$INSTDIR\Docs\html\index.html"
  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Documentation (HTML).lnk" \
                 "$INSTDIR\Docs\html\index.htm"
SectionEnd

Section "QuantLib PDF documentation"
  SetOutPath "$INSTDIR\Docs"
  File "latex\*refman.pdf"
  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Documentation (PDF).lnk" \
                 "$INSTDIR\Docs\refman.pdf"
SectionEnd

Section "QuantLib PS documentation"
  SetOutPath "$INSTDIR\Docs"
  File "latex\*refman.ps"
  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Documentation (PS).lnk" \
                 "$INSTDIR\Docs\refman.ps"
SectionEnd

Section "QuantLib Start Menu Group"
  SetOutPath $SMPROGRAMS\QuantLib
  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Home Page.lnk" \
                 "http://quantlib.org/"
  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Directory.lnk" \
                 "$INSTDIR"
SectionEnd

#it should check for header, if missing to install them
#it should check for Authors, etc.
Section "QuantLib Source Code"
  SetOutPath $INSTDIR
  File ..\ChangeLog.txt
  File ..\makefile.mak
  File ..\QuantLib.dsp
  File ..\QuantLib.dsw
  File ..\QuantLib.mak
  File ..\QuantLib.nsi

  SetOutPath  $INSTDIR\Sources
  File /r "..\Sources\*.cpp"
  File /r "..\Sources\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Calendars
  File /r "..\Sources\Calendars\*.cpp"
  File /r "..\Sources\Calendars\makefile.mak"

  SetOutPath  $INSTDIR\Sources\CashFlows
  File /r "..\Sources\CashFlows\*.cpp"
  File /r "..\Sources\CashFlows\makefile.mak"

  SetOutPath  $INSTDIR\Sources\DayCounters
  File /r "..\Sources\DayCounters\*.cpp"
  File /r "..\Sources\DayCounters\makefile.mak"

  SetOutPath  $INSTDIR\Sources\FiniteDifferences
  File /r "..\Sources\FiniteDifferences\*.cpp"
  File /r "..\Sources\FiniteDifferences\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Indexes
  File /r "..\Sources\Indexes\*.cpp"
  File /r "..\Sources\Indexes\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Instruments
  File /r "..\Sources\Instruments\*.cpp"
  File /r "..\Sources\Instruments\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Math
  File /r "..\Sources\Math\*.cpp"
  File /r "..\Sources\Math\makefile.mak"

  SetOutPath  $INSTDIR\Sources\MonteCarlo
  File /r "..\Sources\MonteCarlo\*.cpp"
  File /r "..\Sources\MonteCarlo\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Pricers
  File /r "..\Sources\Pricers\*.cpp"
  File /r "..\Sources\Pricers\makefile.mak"

  SetOutPath  $INSTDIR\Sources\Solvers1D
  File /r "..\Sources\Solvers1D\*.cpp"
  File /r "..\Sources\Solvers1D\makefile.mak"

  SetOutPath  $INSTDIR\Sources\TermStructures
  File /r "..\Sources\TermStructures\*.cpp"
  File /r "..\Sources\TermStructures\makefile.mak"

#add examples if missing
  IfFileExists $SMPROGRAMS\QuantLib 0 NoSourceShortCuts
    CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib project workspace.lnk" \
                   "$INSTDIR\QuantLib.dsw"
  NoSourceShortCuts:
SectionEnd

#this do not work
#Function .onInstSuccess
#  MessageBox MB_YESNO|MB_ICONQUESTION \
#             "Setup has completed. View HTML documentation now?" \
#             IDNO NoReadme
#    ExecShell open '$INSTDIR\Docs\html\index.htm'
#  NoReadme:
#FunctionEnd

Section Uninstall
SectionEnd
