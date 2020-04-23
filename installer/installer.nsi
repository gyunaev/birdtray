# This script relies on NSIS 3.0, with the nsProcess and NsisMultiUser plugin.

# If uncommented, this script will build an intermediate uninstall_builder.exe
# which generates an uninstaller when run. This extra step allows us to sign the
# uninstaller. See also: http://nsis.sourceforge.net/Signing_an_Uninstaller
# NOTE: This variable is automatically defined during the build process,
# there is no need to uncomment this to generate the uninstaller.
#!define UNINSTALL_BUILDER

# If uncommented, installs the licence in the installation directory.
!define INSTALL_LICENSE

!addplugindir /x86-ansi nsisDependencies\Plugins\x86-ansi
!addplugindir /x86-unicode nsisDependencies\Plugins\x86-unicode
!addincludedir nsisDependencies\Include
!addincludedir .

Unicode true
!ifndef UNINSTALL_BUILDER
SetCompressor /SOLID lzma
!endif # UNINSTALL_BUILDER

# Included files
!include NsisMultiUser.nsh
!include Sections.nsh
!include MUI2.nsh
!include x64.nsh
!include LogicLib.nsh
!include WinVer.nsh
!include StrFunc.nsh
!include StdUtils.nsh
!Include Utils.nsh
# StrFunc.nsh requires priming the commands which actually get used later
!ifdef UNINSTALL_BUILDER
${UnStrCase}
!else
${StrCase}
!endif # UNINSTALL_BUILDER

# Variables
Var skipLicense # If 1, skip the license (we saw it already).
Var startMenuFolder # The name of the start menu folder the user chose.
Var currentUserString # Is "" for if the installation is for all users, else " (current user)".
!ifdef UNINSTALL_BUILDER
Var SemiSilentMode # Installer started uninstaller in semi-silent mode using /SS parameter.
Var RunningFromInstaller # Installer started uninstaller using /uninstall parameter.
!endif # UNINSTALL_BUILDER


# ======================================= Configuration ========================================== #

# General product information
!define PRODUCT_NAME "Birdtray"
!define COMPANY_NAME "UlduzSoft"
!define URL_HELP_LINK "https://github.com/gyunaev/birdtray/wiki" # "Support Information" link
!define URL_UPDATE_INFO "https://github.com/gyunaev/birdtray/releases" # "Product Updates" link
!define URL_INFO_ABOUT "https://www.ulduzsoft.com/" # "Publisher" link
!define MIN_WINDOWS_VER "XP"

# Paths
!define EXE_NAME "birdtray.exe"
!define PROGEXE ${EXE_NAME}  # For the MultiUser plugin
!define ICON_NAME "birdtray.ico"
!define ICON_PATH "..\src\res\${ICON_NAME}"
!define LICENSE_FILE "LICENSE.txt"
!define LICENSE_PATH "..\${LICENSE_FILE}"
!define DIST_DIR "winDeploy"
!define UNINSTALL_REG_PATH \
        "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}$currentUserString"
!define USER_REG_PATH "Software\ulduzsoft"
!define USER_SETTINGS_REG_PATH "${USER_REG_PATH}\${PRODUCT_NAME}"
!define DEFAULT_INSTALL_PATH "$PROGRAMFILES\${PRODUCT_NAME}"
!define UNINSTALL_FILENAME "uninstall.exe"
!define UNINSTALL_BUILDER_FILE "uninstall_builder.exe"
!define UNINSTALL_LIST_BUILDER_FILE "makeUninstallList.exe"
!define UNINSTALL_LIST_FILENAME "uninstall_list.nsh"
!define INSTALLER_TRANSLATIONS_BUILDER_FILE "makeInstallerTranslations.exe"
!define INSTALLER_TRANSLATIONS_FILENAME "installerTranslations.nsi"
!define TRANSLATIONS_LIST_BUILDER_FILE "makeTranslationsList.exe"
!define TRANSLATIONS_LIST_FILENAME "translations_list.nsh"
!define HEADER_IMG_FILE "assets\header.bmp"
!define SIDEBAR_IMG_FILE "assets\sidebar.bmp"
!define INSTALL_CONFIG_FILE ".installConfig.ini"

# Other
!define BAD_PATH_CHARS '?%*:|"<>!;'
!define SETUP_MUTEX "${COMPANY_NAME} ${PRODUCT_NAME} Setup Mutex" # Don't change this

# === Automatic configuration based on the birdtray executable === #
# Version
!getdllversion "${DIST_DIR}\${EXE_NAME}" VERSION_
!define VERSION_MAJOR ${VERSION_1}
!define VERSION_MINOR ${VERSION_2}
!define VERSION_PATCH ${VERSION_3}
!define VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}

# Architecture
!makensis "GetExeArch.nsi" = 0
!system '"GetExeArch.exe" "${DIST_DIR}\${EXE_NAME}"' = 0
!include "arch.nsh"
!delfile "GetExeArch.exe"
!delfile "arch.nsh"

# === Installer/Uninstaller configuration === #

# User Interface Symbol Definitions
!define MUI_ICON ${ICON_PATH}
!define MUI_ABORTWARNING
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP ${HEADER_IMG_FILE}
!define MUI_WELCOMEFINISHPAGE_BITMAP ${SIDEBAR_IMG_FILE}
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_REGISTRY_ROOT SHCTX
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${UNINSTALL_REG_PATH}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}"
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "$(FinishPageRunBirdtray)"
!define MUI_FINISHPAGE_RUN_FUNCTION StartAppBirdTray
!define MUI_LANGDLL_ALLLANGUAGES
!define MUI_LANGDLL_REGISTRY_ROOT SHCTX
!define MUI_LANGDLL_REGISTRY_KEY "${UNINSTALL_REG_PATH}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"
!ifdef UNINSTALL_BUILDER
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ${SIDEBAR_IMG_FILE}
!define MUI_UNICON ${MUI_ICON} # Same icon for installer and un-installer.
!define MUI_UNABORTWARNING
!define MUI_UNCONFIRMPAGE_TEXT_TOP "$(UninstallerConfirmation)"
!define MUI_CUSTOMFUNCTION_UNGUIINIT "un.onGuiInitialisation"
!endif # UNINSTALL_BUILDER

# MultiUser config
!define MULTIUSER_INSTALLMODE_DISPLAYNAME ${PRODUCT_NAME}
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION 1
!define MULTIUSER_INSTALLMODE_ALLOW_ELEVATION_IF_SILENT 0
!if ${ARCH} == x64
    !define MULTIUSER_INSTALLMODE_64_BIT 1
!endif

# Installer/Uninstaller attributes
Name "${PRODUCT_NAME}"
!ifdef UNINSTALL_BUILDER
    OutFile "${UNINSTALL_BUILDER_FILE}"
    SetCompress off
!else
    OutFile "${PRODUCT_NAME}-${VERSION}-Win-${ARCH}.exe"

    !makensis '/DUNINSTALL_BUILDER "${__FILE__}"' = 0
    !system "${UNINSTALL_BUILDER_FILE}" = 0
    !delfile "${UNINSTALL_BUILDER_FILE}"

    # Sign the uninstaller
    # !system "SIGNCODE <signing options> uninstaller\${UNINSTALL_FILENAME}" = 0
!endif # UNINSTALL_BUILDER
CRCCheck on
XPStyle on
BrandingText " " # Branding footer disabled
ShowInstDetails show
!ifdef UNINSTALL_BUILDER
ShowUninstDetails show
!endif # UNINSTALL_BUILDER
AllowSkipFiles off
SetOverwrite on
RequestExecutionLevel user # Admin escalation is done during install

# Installer exe file information
VIProductVersion ${VERSION}.0
VIAddVersionKey ProductName "${PRODUCT_NAME}"
VIAddVersionKey ProductVersion "${VERSION}"
VIAddVersionKey CompanyName "${COMPANY_NAME}"
VIAddVersionKey CompanyWebsite "${URL_INFO_ABOUT}"
VIAddVersionKey FileVersion "${VERSION}"
!ifdef UNINSTALL_BUILDER
VIAddVersionKey FileDescription "${PRODUCT_NAME} uninstaller"
!else
VIAddVersionKey FileDescription "${PRODUCT_NAME} installer"
!endif # UNINSTALL_BUILDER
VIAddVersionKey LegalCopyright ""


# =========================================== Pages ============================================== #

# === Installer Pages === #
!define MUI_PAGE_CUSTOMFUNCTION_PRE WelcomePagePre
!insertmacro MUI_PAGE_WELCOME

!define MUI_PAGE_CUSTOMFUNCTION_PRE LicensePagePre
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE LicensePageLeave
!insertmacro MUI_PAGE_LICENSE "${LICENSE_PATH}"

!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION InstallModePageChangeMode
!insertmacro MULTIUSER_PAGE_INSTALLMODE

!define MUI_PAGE_CUSTOMFUNCTION_PRE DirectoryPagePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW DirectoryPageShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE DirectoryPageLeave
!insertmacro MUI_PAGE_DIRECTORY

!define MUI_PAGE_CUSTOMFUNCTION_PRE ComponentsPagePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW ComponentsPageShow
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_CUSTOMFUNCTION_PRE StartMenuPagePre
!insertmacro MUI_PAGE_STARTMENU "" $startMenuFolder
# The MUI_PAGE_STARTMENU macro un-defines MUI_STARTMENUPAGE_DEFAULTFOLDER, but we need it.
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}"

!define MUI_PAGE_CUSTOMFUNCTION_PRE InstallerPagePre
!insertmacro MUI_PAGE_INSTFILES

!define MUI_PAGE_CUSTOMFUNCTION_SHOW FinishPagePre
!insertmacro MUI_PAGE_FINISH

# === Uninstaller pages ===#
!ifdef UNINSTALL_BUILDER
!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION un.InstallModePageChangeMode
!insertmacro MULTIUSER_UNPAGE_INSTALLMODE

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.ComponentsPagePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.ComponentsPageShow
!insertmacro MUI_UNPAGE_COMPONENTS

!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.ConfirmPageShow
!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.FinishPageShow
!insertmacro MUI_UNPAGE_FINISH

# Installer/Uninstaller languages
!makensis '/DINSTALLER_TRANSLATIONS_BUILDER_FILE="${INSTALLER_TRANSLATIONS_BUILDER_FILE}" \
        /DINSTALLER_TRANSLATIONS_FILENAME="${INSTALLER_TRANSLATIONS_FILENAME}" \
        makeInstallerTranslations.nsi' = 0
!system "${INSTALLER_TRANSLATIONS_BUILDER_FILE}" = 0
!include "${INSTALLER_TRANSLATIONS_FILENAME}"
!delfile "${INSTALLER_TRANSLATIONS_BUILDER_FILE}"
!else
!include "${INSTALLER_TRANSLATIONS_FILENAME}"
!delfile "${INSTALLER_TRANSLATIONS_FILENAME}"
!endif # UNINSTALL_BUILDER

!insertmacro MULTIUSER_LANGUAGE_INIT
!insertmacro MUI_RESERVEFILE_LANGDLL # Make language data faster to decompress at startup


# ========================================== Sections ============================================ #

# === Installer sections === #
Section "${PRODUCT_NAME}" SectionBirdTray
    SectionIn RO # Can't deselect this section

    !ifndef UNINSTALL_BUILDER
    !insertmacro UAC_AsUser_Call Function CheckInstallation ${UAC_SYNCREGISTERS}
    ${if} $0 != ""
        DetailPrint "$(UninstallPreviousVersion)"
        !insertmacro STOP_PROCESS ${EXE_NAME} "$(StopBirdtray)" "$(StopBirdtrayError)"
        ClearErrors
        ${if} $0 == "AllUsers"
            Call RunUninstaller
        ${else}
            !insertmacro UAC_AsUser_Call Function RunUninstaller ${UAC_SYNCREGISTERS}
        ${endif}
        ${if} ${errors} # Stay in installer
            MessageBox MB_OKCANCEL|MB_ICONSTOP "$(UninstallPreviousVersionError)" \
                /SD IDCANCEL IDOK Ignore
            SetErrorLevel 2 # Installation aborted by script
            BringToFront
            Abort "$(UninstallExecutionError)"
            Ignore:
            RMDir /r "$3"
        ${else}
            ${Switch} $0
                ${case} 0 # Uninstaller completed successfully - continue with installation
                    BringToFront
                    Sleep 1000 # Wait for the cmd.exe
                    BringToFront
                    ${break}
                ${case} 1 # Installation aborted by user (cancel button)
                ${case} 2 # Installation aborted by script
                    SetErrorLevel $0
                    Quit # Uninstaller was started, but completed with errors - Quit installer
                ${default} # All other error codes - Abort installer
                    SetErrorLevel $0
                    BringToFront
                    Abort "$(UninstallExecutionError)"
            ${EndSwitch}
        ${endif}

        # Just a failsafe.
        # The uninstaller doesn't delete itself when not copied to the temp directory.
        !insertmacro DeleteRetryAbort "$3\${UNINSTALL_FILENAME}"
        RMDir "$3"
    ${endif}

    SetOutPath $INSTDIR

    # Write uninstaller and registry uninstall info as the first step,
    # so that the user has the option to run the uninstaller if sth. goes wrong.
    File "uninstaller\${UNINSTALL_FILENAME}"
    !insertmacro MULTIUSER_RegistryAddInstallInfo

    WriteRegDWORD SHCTX "${UNINSTALL_REG_PATH}" "VersionMajor" ${VERSION_MAJOR}
    WriteRegDWORD SHCTX "${UNINSTALL_REG_PATH}" "VersionMinor" ${VERSION_MINOR}

    File /r /x translations "${DIST_DIR}\*"

    !ifdef INSTALL_LICENSE
        File "${LICENSE_PATH}"
    !endif # INSTALL_LICENSE

    !endif # UNINSTALL_BUILDER
SectionEnd

!ifndef UNINSTALL_BUILDER

SectionGroup /e "$(WinIntegrationSectionName)" SectionGroupWinIntegration
Section "$(ProgramGroupSectionName)" SectionProgramGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN ""

    CreateDirectory "$SMPROGRAMS\$startMenuFolder"
    CreateShortCut "$SMPROGRAMS\$startMenuFolder\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
    CreateShortCut "$SMPROGRAMS\$startMenuFolder\$(SettingsLink).lnk" "$INSTDIR\${EXE_NAME}" \
        "--settings"

    !ifdef INSTALL_LICENSE
        CreateShortCut "$SMPROGRAMS\$startMenuFolder\$(LicenseStartMenuLinkName).lnk" \
            "$INSTDIR\${LICENSE_FILE}"
    !endif
    ${if} $MultiUser.InstallMode == "AllUsers"
        CreateShortCut "$SMPROGRAMS\$startMenuFolder\Uninstall.lnk" \
            "$INSTDIR\${UNINSTALL_FILENAME}" "/allusers"
    ${else}
        CreateShortCut "$SMPROGRAMS\$startMenuFolder\Uninstall.lnk" \
            "$INSTDIR\${UNINSTALL_FILENAME}" "/currentuser"
    ${endif}

    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "$(DesktopEntrySectionName)" SectionDesktopEntry
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
SectionEnd

Section /o "$(StartMenuSectionName)" SectionStartMenuEntry
    CreateShortCut "$STARTMENU\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
SectionEnd

Section /o "$(AutoRunSectionName)" SectionAutoRun
    WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Run" \
        "${PRODUCT_NAME}" "$\"$INSTDIR\${EXE_NAME}$\""
    AddSize 1
SectionEnd
SectionGroupEnd

Section "$(AutoCheckUpdateSectionName)" SectionAutoCheckUpdate
    ${StrCase} $0 "${COMPANY_NAME}" "L"
    ${StrCase} $1 "${PRODUCT_NAME}" "L"
    DeleteRegValue HKCU "${USER_SETTINGS_REG_PATH}" "hasReadInstallConfig"
    CreateDirectory "$LOCALAPPDATA\$0"
    CreateDirectory "$LOCALAPPDATA\$0\$1"
    FileOpen $2 "$LOCALAPPDATA\$0\$1\${INSTALL_CONFIG_FILE}" a
    FileSeek $2 0 END
    FileWrite $2 "updateOnStartup = true$\r$\n"
SectionEnd

SectionGroup /e "$(TranslationsSectionName)" SectionGroupTranslations
    !makensis '/DDIST_DIR="${DIST_DIR}" \
                /DTRANSLATIONS_LIST_FILENAME="${TRANSLATIONS_LIST_FILENAME}" \
                makeTranslationsList.nsi' = 0
    !system "${TRANSLATIONS_LIST_BUILDER_FILE}" = 0
    !include "${TRANSLATIONS_LIST_FILENAME}"
    !delfile "${TRANSLATIONS_LIST_BUILDER_FILE}"
    !delfile "${TRANSLATIONS_LIST_FILENAME}"
SectionGroupEnd

Section "-Write Install Size" # Hidden section, write install size as the final step
    !insertmacro MULTIUSER_RegistryAddInstallSizeInfo
SectionEnd

Section "-Check VC Installed" # Hidden section, check if the Visual Redistributable is installed
    ${IsVisualRedistributableInstalled} $R0 ${ARCH}
    ${if} $R0 != 1
        DetailPrint "$(MissingVcRuntime)"
        MessageBox MB_YESNO|MB_ICONQUESTION "$(MissingVcRuntime_Dialog)" /SD IDNO IDNO VCNotFound
        ${OpenURL} "https://aka.ms/vs/16/release/vc_redist.${ARCH}.exe"
        MessageBox MB_OK|MB_ICONQUESTION "$(MissingVcRuntime_Retry)"
        ${IsVisualRedistributableInstalled} $R0 ${ARCH}
        StrCmp $R0 1 0 +3
            MessageBox MB_OK|MB_ICONINFORMATION "$(MissingVcRuntime_Found)"
            Goto Done
        MessageBox MB_OK|MB_ICONEXCLAMATION "$(MissingVcRuntime_StillNotFound)"
        VCNotFound:
        DetailPrint "$(MissingVcRuntime_UnableToRun)"
        MessageBox MB_OK|MB_ICONEXCLAMATION "$(MissingVcRuntime_UnableToRunDialog)" /SD IDOK
        SetErrorLevel 1157 # ERROR_DLL_NOT_FOUND

        Done:
    ${endif}
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionBirdTray} "$(BirdtraySectionDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionAutoRun} "$(AutoRunDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionAutoCheckUpdate} "$(AutoCheckUpdateDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionGroupWinIntegration} \
            "$(WinIntegrationGroupDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionGroupTranslations} "$(TranslationsDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionProgramGroup} "$(ProgramGroupDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionDesktopEntry} "$(DesktopEntryDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionStartMenuEntry} "$(StartMenuDescription)"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!endif # UNINSTALL_BUILDER

# === Uninstaller sections === #
!ifdef UNINSTALL_BUILDER
Section "un.${PRODUCT_NAME}" UNSectionBirdTray
    SectionIn RO # Can't deselect this section

    # Try to delete the EXE as the first step - if it's in use, don't remove anything else
    !insertmacro DeleteRetryAbort "$INSTDIR\${EXE_NAME}"

    !makensis '/DDIST_DIR="${DIST_DIR}" /DUNINSTALL_LIST_FILENAME="${UNINSTALL_LIST_FILENAME}" \
            /DEXE_NAME="${EXE_NAME}" makeUninstallList.nsi' = 0
    !system "${UNINSTALL_LIST_BUILDER_FILE}" = 0
    !include "uninstaller\${UNINSTALL_LIST_FILENAME}"
    !delfile "${UNINSTALL_LIST_BUILDER_FILE}"
    !delfile "uninstaller\${UNINSTALL_LIST_FILENAME}"

    !ifdef LICENSE_FILE
        !insertmacro DeleteRetryAbort "$INSTDIR\${LICENSE_FILE}"
    !endif

    # Clean up "Auto Update-Check"
    ${UnStrCase} $0 "${COMPANY_NAME}" "L"
    ${UnStrCase} $1 "${PRODUCT_NAME}" "L"
    !insertmacro DeleteRetryAbort "$LOCALAPPDATA\$0\$1\${INSTALL_CONFIG_FILE}"
    RMDir /r "$LOCALAPPDATA\$0\$1"
    RMDir /r "$LOCALAPPDATA\$0"

    # Clean up "AutoRun"
    DeleteRegValue SHCTX "Software\Microsoft\Windows\CurrentVersion\Run" "${PRODUCT_NAME}"

    # Clean up "Program Group" - we check that we created a StartMenu folder,
    # or else the whole StartMenu directory will be removed!
    ${if} "$StartMenuFolder" != ""
        RMDir /r "$SMPROGRAMS\$StartMenuFolder"
    ${endif}
    # Clean up "Start Menu Icon"
    !insertmacro DeleteRetryAbort "$STARTMENU\${PRODUCT_NAME}.lnk"

    # Clean up "Desktop Icon"
    !insertmacro DeleteRetryAbort "$DESKTOP\${PRODUCT_NAME}.lnk"

SectionEnd

Section /o "un.$(UserSettingsSectionName)" UNSectionUserSettings
    DeleteRegKey HKCU "${USER_SETTINGS_REG_PATH}"
    DeleteRegKey /ifempty HKCU "${USER_REG_PATH}"
SectionEnd

Section -un.Post UNSectionSystem

    Delete "$INSTDIR\${UNINSTALL_FILENAME}"

    # Remove the directory only if it is empty - the user might have saved some files in it.
    # Don't set REBOOTOK here, or else we may end up installing a new version
    # in this path which gets removed on reboot!
    RMDir "$INSTDIR"

    # Remove the uninstaller from registry as the very last step.
    # If sth. goes wrong, let the user run it again.
    !insertmacro MULTIUSER_RegistryRemoveInstallInfo

    # If the uninstaller still exists, use cmd.exe on exit to remove it (along with $INSTDIR
    # if it's empty). This is always the case when started with the _? parameter.
    ${if} ${FileExists} "$INSTDIR\${UNINSTALL_FILENAME}"
        Exec 'cmd.exe /c (del /f /q "$INSTDIR\${UNINSTALL_FILENAME}") && (rmdir "$INSTDIR")'
    ${endif}
SectionEnd

!insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${UNSectionBirdTray} "$(BirdtraySectionDescription)"
    !insertmacro MUI_DESCRIPTION_TEXT ${UNSectionUserSettings} "$(UserSettingsDescription)"
!insertmacro MUI_UNFUNCTION_DESCRIPTION_END
!endif # UNINSTALL_BUILDER

# ===================================== Macros / Functions ======================================= #

# Called when we switch from single user to multi-user installation or vice versa.
Function InstallModePageChangeMode
    !insertmacro MUI_STARTMENU_GETFOLDER "" $startMenuFolder
    !insertmacro MULTIUSER_GetCurrentUserString $0
    StrCpy $currentUserString "$0"
FunctionEnd

# === Installer functions === #
Function .onInit
    !ifdef UNINSTALL_BUILDER
        SetSilent silent
        System::Call "kernel32::GetCurrentDirectory(i ${NSIS_MAX_STRLEN}, t .r0)"
        CreateDirectory "$0\uninstaller"
        WriteUninstaller "$0\uninstaller\${UNINSTALL_FILENAME}"
        SetErrorLevel 0
        Quit
    !endif

    !insertmacro CheckMinWinVer ${MIN_WINDOWS_VER}
    ${ifNot} ${UAC_IsInnerInstance}
        !insertmacro CheckPlatform ${Arch}
        !insertmacro CheckSingleInstance "$(SetupAlreadyRunning)" "Global" "${SETUP_MUTEX}"
    ${endif}

    !insertmacro MULTIUSER_INIT

    ${if} $IsInnerInstance == 0
        !insertmacro MUI_LANGDLL_DISPLAY
    ${endif}
FunctionEnd

# Called when entering the welcome page.
Function WelcomePagePre
    ${if} $InstallShowPagesBeforeComponents == 0
        Abort
    ${endif}
FunctionEnd

# Called when entering the license page.
Function LicensePagePre
    ${if} $SkipLicense == 1
    ${orIf} $InstallShowPagesBeforeComponents == 0
        Abort
    ${endif}
FunctionEnd

# Called when leaving the license page.
Function LicensePageLeave
    StrCpy $SkipLicense 1
FunctionEnd

# Called when entering the install directory page
Function DirectoryPagePre
    GetDlgItem $0 $HWNDPARENT 1
    SendMessage $0 ${BCM_SETSHIELD} 0 0 # Hide SHIELD (Windows Vista and above)
FunctionEnd

# Called when the install directory page is visible
Function DirectoryPageShow
    ${if} $CmdLineDir != ""
        FindWindow $R1 "#32770" "" $HWNDPARENT

        # Make the directory input read only
        GetDlgItem $0 $R1 1019
        SendMessage $0 ${EM_SETREADONLY} 1 0

        # Disable the browse button
        GetDlgItem $0 $R1 1001
        EnableWindow $0 0
    ${endif}
FunctionEnd

# Called when leaving the installation directory selection page.
Function DirectoryPageLeave
    ${ValidPath} $0 $INSTDIR ${BAD_PATH_CHARS}
    ${if} $0 != 0
        MessageBox MB_OK|MB_ICONEXCLAMATION "$(BadInstallPath)" /SD IDOK
        Abort
    ${endif}
    ${IsDirEmpty} $0 $INSTDIR
    ${if} $0 == 0
        IfFileExists $INSTDIR\${EXE_NAME} 0 DirNotEmptyNoBirdtray
        IfFileExists $INSTDIR\${UNINSTALL_FILENAME} 0 DirNotEmptyNoBirdtray
        MessageBox MB_OKCANCEL|MB_ICONINFORMATION \
                        "$(InstallPathContainsPreviousVersion)" /SD IDOK IDOK Ignore
        Abort
        DirNotEmptyNoBirdtray:
        MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION "$(InstallPathNotEmpty)" /SD IDOK IDOK Ignore
        Abort
        Ignore:
    ${endif}
FunctionEnd

# Called when entering the component page
Function ComponentsPagePre
    GetDlgItem $1 $HWNDPARENT 1
    !ifndef UNINSTALL_BUILDER
    ${if} ${SectionIsSelected} ${SectionProgramGroup}
        SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^NextBtn)" # Not the last page before installing
        SendMessage $1 ${BCM_SETSHIELD} 0 0 # Hide SHIELD (Windows Vista and above)
    ${else}
        SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^InstallBtn)" # The last page before installing
        Call MultiUser.CheckPageElevationRequired
        ${if} $0 == 2
            SendMessage $1 ${BCM_SETSHIELD} 0 1 # Display SHIELD (Windows Vista and above)
        ${endif}
    ${endif}

    ${if} $MultiUser.InstallMode == "AllUsers"
        # Add "(current user only)" text to sections "Start Menu Icon"
        ${if} ${AtLeastWin7}
            SectionGetText ${SectionStartMenuEntry} $0
            SectionSetText ${SectionStartMenuEntry} "$(CurrentUserOnly)"
        ${endif}
    ${endif}
    !endif # UNINSTALL_BUILDER
FunctionEnd

Function ComponentsPageShow
    !insertmacro SORT_SECTION_GROUP "$(TranslationsSectionName)"
FunctionEnd

# Called when changing selections on the component page
Function .onSelChange
    GetDlgItem $1 $HWNDPARENT 1
    !ifndef UNINSTALL_BUILDER
    ${if} ${SectionIsSelected} ${SectionProgramGroup}
        SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^NextBtn)" # Not the last page before installing
        SendMessage $1 ${BCM_SETSHIELD} 0 0 # Hide SHIELD (Windows Vista and above)
    ${else}
        SendMessage $1 ${WM_SETTEXT} 0 "STR:$(^InstallBtn)" # The last page before installing
        Call MultiUser.CheckPageElevationRequired
        ${if} $0 == 2
            SendMessage $1 ${BCM_SETSHIELD} 0 1 # Display SHIELD (Windows Vista and above)
        ${endif}
    ${endif}
    !endif # UNINSTALL_BUILDER
FunctionEnd

# Called when entering the start menu page
Function StartMenuPagePre
    !ifndef UNINSTALL_BUILDER
    ${ifNot} ${SectionIsSelected} ${SectionProgramGroup}
        Abort # Don't display this dialog if SectionProgramGroup is not selected
    ${else}
        GetDlgItem $1 $HWNDPARENT 1
        Call MultiUser.CheckPageElevationRequired
        ${if} $0 == 2
            SendMessage $1 ${BCM_SETSHIELD} 0 1 # Display SHIELD (Windows Vista and above)
        ${endif}
    ${endif}
    !endif # UNINSTALL_BUILDER
FunctionEnd

# Called before installing
Function InstallerPagePre
    GetDlgItem $0 $HWNDPARENT 1
    SendMessage $0 ${BCM_SETSHIELD} 0 0 # Hide SHIELD (Windows Vista and above)
FunctionEnd

# Called when the finish page is shown
Function FinishPagePre
    !ifndef UNINSTALL_BUILDER
    ${IsVisualRedistributableInstalled} $R0 ${ARCH}
    ${if} $R0 != 1
        SendMessage $mui.FinishPage.Run ${BM_SETCHECK} ${BST_UNCHECKED} 0 # uncheck 'run software'
        ShowWindow $mui.FinishPage.Run 0 # Hide the checkbox
    ${endif}
    !endif # UNINSTALL_BUILDER
FunctionEnd

Function .onInstFailed
    MessageBox MB_ICONSTOP "$(InstallFailed)" /SD IDOK
FunctionEnd

# === Uninstaller functions === #
!ifdef UNINSTALL_BUILDER
Function un.onInit
    ${GetParameters} $R0

    ${GetOptions} $R0 "/uninstall" $R1
    ${ifNot} ${errors}
        StrCpy $RunningFromInstaller 1
    ${else}
        StrCpy $RunningFromInstaller 0
    ${endif}

    ${GetOptions} $R0 "/SS" $R1
    ${ifNot} ${errors}
        StrCpy $SemiSilentMode 1
        StrCpy $RunningFromInstaller 1
        SetAutoClose true # Auto close (if no errors) if we are called from the installer
    ${else}
        StrCpy $SemiSilentMode 0
    ${endif}

    # We always get the language, since the outer and inner instance might have different language
    !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Function un.onGuiInitialisation
    ${ifNot} ${UAC_IsInnerInstance}
    ${andIf} $RunningFromInstaller == 0
        ${if} ${UAC_IsAdmin}
            ReadRegStr $0 HKCU "${MULTIUSER_INSTALLMODE_INSTALL_REGISTRY_KEY_PATH}" \
                "${MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME}"
            ${GetParameters} $R0
            ${GetOptions} $R0 "/currentuser" $R1
            ${ifnot} ${errors}
            ${andif} $0 == ""
                MessageBox MB_YESNO|MB_ICONQUESTION "$(UninstallRestartAsUserQuestion)" /SD IDNO \
                    IDNO Continue
                ${StdUtils.ExecShellAsUser} $0 "$INSTDIR\${UNINSTALL_FILENAME}" "open" $R0
                Quit
                Continue:
            ${endif}
        ${endif}
        !insertmacro CheckSingleInstance "$(UninstallAlreadyRunning)" "Global" "${SETUP_MUTEX}"
    ${endif}

    !insertmacro MULTIUSER_UNINIT

    !insertmacro STOP_PROCESS ${EXE_NAME} "$(StopBirdtrayUninstall)" "$(StopBirdtrayUninstallError)"
FunctionEnd

Function un.InstallModePageChangeMode
    !insertmacro MUI_STARTMENU_GETFOLDER "" $startMenuFolder
FunctionEnd

# Called when the "Confirm Uninstall" page is visible
Function un.ConfirmPageShow
    ${if} $SemiSilentMode == 1
        # Show/hide the Back button
        GetDlgItem $0 $HWNDPARENT 3
        ShowWindow $0 $UninstallShowBackButton
    ${endif}
FunctionEnd

# Called when we enter the components page
Function un.ComponentsPagePre
    ${if} $SemiSilentMode == 1
        Abort # If user is installing, no use to remove program settings anyway
    ${endif}
FunctionEnd

# Called when showing the component page
Function un.ComponentsPageShow
    # Show/hide the Back button
    GetDlgItem $0 $HWNDPARENT 3
    ShowWindow $0 $UninstallShowBackButton
FunctionEnd

# Called when showing the finish page
Function un.FinishPageShow
    ${if} $SemiSilentMode == 1
        Abort # If user is installing
    ${endif}
FunctionEnd

Function un.onUninstFailed
    ${if} $SemiSilentMode == 0
        MessageBox MB_ICONSTOP "$(UninstallFailed)" /SD IDOK
    ${else}
        MessageBox MB_ICONSTOP "$(InstallFailedCauseUninstallerFailed)" /SD IDOK
    ${endif}
FunctionEnd

!endif # UNINSTALL_BUILDER

# === Helper function ===#

# Function invoked by MUI_FINISHPAGE_RUN
Function StartAppBirdTray
    # The installer might exit too soon before the application starts and
    # it loses the right to be the foreground window and starts in the background.
    # However, if there's no active window when the application starts,
    # it will become the active window, so we hide the installer.
    HideWindow
    # The installer will show itself again quickly before closing (w/o TaskBar button),
    # so we move it offscreen.
    !define SWP_NOSIZE 0x0001
    !define SWP_NOZORDER 0x0004
    System::Call "User32::SetWindowPos(i, i, i, i, i, i, i) b \
        ($HWNDPARENT, 0, -1000, -1000, 0, 0, ${SWP_NOZORDER}|${SWP_NOSIZE})"

    !insertmacro UAC_AsUser_ExecShell "open" "$INSTDIR\${EXE_NAME}" "" "$INSTDIR" ""
FunctionEnd

!ifndef UNINSTALL_BUILDER
# Check if a previous installation exists.
# Returns:
# $0 - INSTALL_MODE: The installation mode of a previous installation or "" if none was found.
# $1 - UNISTALL_EXE_PATH_WITH_PARAMETER: The path to the uninstall exe with the appropriate
#                                        /currentuser or /allusers parameter.
# $2 - SILENT_PARAM: The silent parameter for the uninstaller.
# $3 - INSTALLETION_DIR: The directory of the previous installation.
Function CheckInstallation
    # If there's an installed version, uninstall it first.
    # If both per-user and per-machine versions are installed,
    # uninstall the one that matches $MultiUser.InstallMode.
    StrCpy $0 ""
    ${if} $HasCurrentModeInstallation == 1
        StrCpy $0 "$MultiUser.InstallMode"
    ${else}
        !if ${MULTIUSER_INSTALLMODE_ALLOW_BOTH_INSTALLATIONS} == 0
            ${if} $HasPerMachineInstallation == 1
                # If there's no per-user installation, but there is a
                # per-machine installation, uninstall it.
                StrCpy $0 "AllUsers"
            ${elseif} $HasPerUserInstallation == 1
                # If there's no per-machine installation, but there is a
                # per-user installation, uninstall it.
                StrCpy $0 "CurrentUser"
            ${endif}
        !endif
    ${endif}

    ${if} "$0" != ""
        ${if} $0 == "AllUsers"
            StrCpy $1 "$PerMachineUninstallString"
            StrCpy $3 "$PerMachineInstallationFolder"
        ${else}
            StrCpy $1 "$PerUserUninstallString"
            StrCpy $3 "$PerUserInstallationFolder"
        ${endif}
        ${if} ${silent}
            StrCpy $2 "/S"
        ${else}
            StrCpy $2 ""
        ${endif}
    ${endif}
FunctionEnd
!endif # UNINSTALL_BUILDER
