
# to be used with NSIS 2.0 and up

SetCompressor lzma

!define VER_NUMBER "0.3.9"

# HEADER CONFIGURATION COMMANDS
Name "QuantLib"
Caption "QuantLib - Setup"
#do not change the name below
OutFile "..\QuantLib-Win32installer-${VER_NUMBER}.exe"

InstType "Full (w/ WinHelp Documentation)"
InstType Minimal

ComponentText "This will install QuantLib ${VER_NUMBER} on your computer"

SilentInstall normal
CRCCheck on
LicenseText "You must agree with the following license before installing:"
LicenseData License.txt
DirText "Please select a location to install QuantLib (or use the default):"
InstallDir $PROGRAMFILES\QuantLib
InstallDirRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib "Install_Dir"
AutoCloseWindow false
ShowInstDetails hide
SetDateSave on

# INSTALLATION EXECUTION COMMANDS



Section "-QuantLib"
SectionIn 1 2
# this directory must be created first, or the CreateShortCut will not work
    CreateDirectory "$SMPROGRAMS\QuantLib"
    SetOutPath $INSTDIR

    # these MUST be present
    File "README.txt"
    File "LICENSE.txt"
    File "News.txt"
    File "QuantLib.dsw"
    File "QuantLib.sln"
    File "QuantLib.dev"

    File "*.txt"
    File "*.TXT"
    File "makefile.mak"
    File "*.dsp"
    File "*.vcproj"
    File "*.nsi"

    SetOutPath  $INSTDIR\ql
    File /r "ql\*.hpp"
    File /r "ql\*.h"
    File /r "ql\*.cpp"
    File /r "ql\*.c"
    File /r "ql\makefile.mak"

    SetOutPath $INSTDIR\functions
    File /r "functions\makefile.mak"
    File /r "functions\*.cpp"
    File /r "functions\*.hpp"
    File /r "functions\*.dsp"
    File /r "functions\*.dev"
    File /r "functions\*.vcproj"

    SetOutPath $INSTDIR\test-suite
    File /r "test-suite\*.txt"
    File /r "test-suite\makefile.mak"
    File /r "test-suite\*.cpp"
    File /r "test-suite\*.hpp"
    File /r "test-suite\*.dsp"
    File /r "test-suite\*.dev"
    File /r "test-suite\*.vcproj"
    File /r "test-suite\*.bat"

    SetOutPath $INSTDIR\Examples
    File /r "Examples\*.cpp"
    File /r "Examples\*.dsp"
    File /r "Examples\*.txt"
    File /r "Examples\makefile.mak"
    File /r "Examples\*.vcproj"

    SetOutPath $INSTDIR\Docs
    File /r "Docs\*.pdf"
    File /r "Docs\*.am"
    File /r "Docs\*.bmp"
    File /r "Docs\*.css"
    File /r "Docs\*.cpp"
    File /r "Docs\*.docs"
    File /r "Docs\*.doxy"
    File /r "Docs\*.html"
    File /r "Docs\*.jpg"
    File /r "Docs\makefile.mak"
    File /r "Docs\*.png"
    File /r "Docs\*.eps"
    File /r "Docs\*.tex"
    File /r "Docs\*.txt"

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
    CreateShortCut "$SMPROGRAMS\QuantLib\Uninstall QuantLib.lnk" \
                   "$INSTDIR\QuantLibUninstall.exe" \
                   "" "$INSTDIR\QuantLibUninstall.exe" 0
    CreateShortCut "$SMPROGRAMS\QuantLib\README.txt.lnk" \
                   "$INSTDIR\README.txt"
    CreateShortCut "$SMPROGRAMS\QuantLib\LICENSE.txt.lnk" \
                   "$INSTDIR\LICENSE.txt"
    CreateShortCut "$SMPROGRAMS\QuantLib\What's new.lnk" \
                   "$INSTDIR\News.txt"

    CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib VC 6 project workspace.lnk" \
                   "$INSTDIR\QuantLib.dsw"

    CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib VC 7 project workspace.lnk" \
                   "$INSTDIR\QuantLib.sln"

    WriteUninstaller "QuantLibUninstall.exe"

SectionEnd

Section "WinHelp documentation"
SectionIn 1
  SetOutPath "$INSTDIR\Docs"
  File /nonfatal "Docs\QuantLib-docs-${VER_NUMBER}.chm"
  IfFileExists "$INSTDIR\Docs\QuantLib-docs-${VER_NUMBER}.chm" 0 NoWinHelpDoc
      CreateShortCut "$SMPROGRAMS\QuantLib\Documentation (WinHelp).lnk" \
                 "$INSTDIR\Docs\QuantLib-docs-${VER_NUMBER}.chm"
  NoWinHelpDoc:
SectionEnd

Section "Start Menu Group"
SectionIn 1 2
  SetOutPath $SMPROGRAMS\QuantLib

#it doesn't work
#  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Home Page.lnk" \
#                 "http://quantlib.org/index.html"
#this works
  WriteINIStr "$SMPROGRAMS\QuantLib\QuantLib Home Page.url" \
              "InternetShortcut" "URL" "http://quantlib.org/"

  CreateShortCut "$SMPROGRAMS\QuantLib\QuantLib Directory.lnk" \
                 "$INSTDIR"
SectionEnd

Function .onInit
  SetOutPath $TEMP
  File /oname=spltmp.bmp "Docs\images\QL.bmp"
  splash::show 2000 $TEMP\spltmp
  Pop $0 ; $0 has '1' if the user closed the splash screen early,
         ;        '0' if everything closed normal,
         ;        '-1' if some error occured.
  Delete $TEMP\spltmp.bmp
FunctionEnd

UninstallText "This will uninstall QuantLib. Hit next to continue."


Section "Uninstall"
    DeleteRegKey HKEY_LOCAL_MACHINE \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\QuantLib"
    DeleteRegKey HKEY_LOCAL_MACHINE SOFTWARE\QuantLib
    DeleteRegValue HKEY_CURRENT_USER  "Environment" "QL_DIR"
    Delete "$SMPROGRAMS\QuantLib\*.*"
    RMDir "$SMPROGRAMS\QuantLib"
    RMDir /r "$INSTDIR\Examples"
    RMDir /r "$INSTDIR\Docs"
    RMDir /r "$INSTDIR\ql"
    RMDir /r "$INSTDIR\lib"
    RMDir /r "$INSTDIR"
SectionEnd
