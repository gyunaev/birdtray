# This script relies on NSIS 3.0, with the nsProcess and NsisMultiUser plugin.

# If uncommented, this script will build an intermediate uninstall_builder.exe
# which generates an uninstaller when run. This extra step allows us to sign the
# uninstaller. See also: http://nsis.sourceforge.net/Signing_an_Uninstaller
# NOTE: This variable is automatically defined during the build process,
# there is no need to uncomment this to generate the uninstaller.
#!define UNINSTALL_BUILDER

# If uncommented, installs the licence in the installation directory.
!define INSTALL_LICENSE

!addplugindir /x86-ansi nsisExtras\Plugins\x86-ansi
!addplugindir /x86-unicode nsisExtras\Plugins\x86-unicode
!addincludedir nsisExtras\Include
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
!Include Utils.nsh
# StrFunc.nsh requires priming the commands which actually get used later
${StrRep}
!ifdef UNINSTALL_BUILDER
${UnStrRep}
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
!define HELP_URL "https://github.com/gyunaev/birdtray/wiki" # "Support Information" link
!define UPDATE_URL "https://github.com/gyunaev/birdtray/releases" # "Product Updates" link
!define ABOUT_URL "http://www.ulduzsoft.com/" # "Publisher" link
!define MIN_WINDOWS_VER "XP"

# Section descriptions
!define BIRDTRAY_SECTION_DESCRIPTION "A free system tray notification for new mail for Thunderbird."
!define WIN_INTEGRATION_GROUP_DESCRIPTION "Select how to integrate the program in Windows."
!define AUTO_RUN_DESCRIPTION "Automatically start ${PRODUCT_NAME} after login."
!define PROGRAM_GROUP_DESCRIPTION \
        "Create a ${PRODUCT_NAME} program group under Start Menu > Programs."
!define DESKTOP_ENTRY_DESCRIPTION "Create a ${PRODUCT_NAME} icon on the Desktop."
!define START_MENU_DESCRIPTION "Create a ${PRODUCT_NAME} icon in the Start Menu."
!define USER_SETTINGS_DESCRIPTION "Your settings and configuration made within ${PRODUCT_NAME}."

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
!define HEADER_IMG_FILE "assets\header.bmp"
!define SIDEBAR_IMG_FILE "assets\sidebar.bmp"

# Other
!define BAD_PATH_CHARS '?%*:|"<>!;'
!define LICENSE_START_MENU_LINK_NAME "License Agreement"
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
!define MUI_FINISHPAGE_RUN_TEXT "Start ${PRODUCT_NAME}"
!define MUI_FINISHPAGE_RUN_FUNCTION StartAppBirdTray
!define MUI_LANGDLL_ALLLANGUAGES
!define MUI_LANGDLL_REGISTRY_ROOT SHCTX
!define MUI_LANGDLL_REGISTRY_KEY "${UNINSTALL_REG_PATH}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Language"
!ifdef UNINSTALL_BUILDER
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ${SIDEBAR_IMG_FILE}
!define MUI_UNICON ${MUI_ICON} # Same icon for installer and un-installer.
!define MUI_UNABORTWARNING
!define MUI_UNCONFIRMPAGE_TEXT_TOP "Click uninstall to begin the process."
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
VIAddVersionKey CompanyWebsite "${ABOUT_URL}"
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
!insertmacro MUI_PAGE_COMPONENTS

!define MUI_PAGE_CUSTOMFUNCTION_PRE StartMenuPagePre
!insertmacro MUI_PAGE_STARTMENU "" $startMenuFolder
# The MUI_PAGE_STARTMENU macro un-defines MUI_STARTMENUPAGE_DEFAULTFOLDER, but we need it.
!define MUI_STARTMENUPAGE_DEFAULTFOLDER "${PRODUCT_NAME}"

!define MUI_PAGE_CUSTOMFUNCTION_PRE InstallerPagePre
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

# === Uninstaller pages ===#
!ifdef UNINSTALL_BUILDER
!define MULTIUSER_INSTALLMODE_CHANGE_MODE_FUNCTION un.InstallModePageChangeMode
!insertmacro MULTIUSER_UNPAGE_INSTALLMODE

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.ComponentsPagePre
!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.ComponentsPageShow
!insertmacro MUI_UNPAGE_COMPONENTS

!define MUI_PAGE_CUSTOMFUNCTION_SHOW un.ConfirmPageShow
!define MUI_PAGE_CUSTOMFUNCTION_LEAVE un.ConfirmPageLeave
!insertmacro MUI_UNPAGE_CONFIRM

!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_PAGE_CUSTOMFUNCTION_PRE un.FinishPageShow
!insertmacro MUI_UNPAGE_FINISH
!endif # UNINSTALL_BUILDER


# Installer/Uninstaller languages
!insertmacro MUI_LANGUAGE "English"
# Add more languages here

!insertmacro MULTIUSER_LANGUAGE_INIT
!insertmacro MUI_RESERVEFILE_LANGDLL # Make language data faster to decompress at startup


# ========================================== Sections ============================================ #

# === Installer sections === #
Section "${PRODUCT_NAME}" SectionBirdTray
    SectionIn RO # Can't deselect this section

    !ifndef UNINSTALL_BUILDER
    !insertmacro UAC_AsUser_Call Function CheckInstallation ${UAC_SYNCREGISTERS}
    ${if} $0 != ""
        DetailPrint "Uninstalling previous version of ${PRODUCT_NAME}"
        !insertmacro STOP_PROCESS ${EXE_NAME} "${PRODUCT_NAME} is currently running. \
                Press OK to stop ${PRODUCT_NAME} to continue with the installation." \
                "Failed to close ${PRODUCT_NAME}. Please retry or quit ${PRODUCT_NAME} manually \
                to continue with the installation."
        ClearErrors
        ${if} $0 == "AllUsers"
            Call RunUninstaller
        ${else}
            !insertmacro UAC_AsUser_Call Function RunUninstaller ${UAC_SYNCREGISTERS}
        ${endif}
        ${if} ${errors} # Stay in installer
            SetErrorLevel 2 # Installation aborted by script
            BringToFront
            Abort "Error executing uninstaller."
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
                    Abort "Error executing uninstaller."
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

    ReadRegStr $0 SHCTX "${UNINSTALL_REG_PATH}" "UninstallString"
    WriteRegStr SHCTX "${UNINSTALL_REG_PATH}" "QuietUninstallString" "$0 /S"
    WriteRegStr SHCTX "${UNINSTALL_REG_PATH}" "HelpLink" "${HELP_URL}"
    WriteRegStr SHCTX "${UNINSTALL_REG_PATH}" "URLUpdateInfo" "${UPDATE_URL}"
    WriteRegStr SHCTX "${UNINSTALL_REG_PATH}" "URLInfoAbout" "${ABOUT_URL}"
    WriteRegDWORD SHCTX "${UNINSTALL_REG_PATH}" "VersionMajor" ${VERSION_MAJOR}
    WriteRegDWORD SHCTX "${UNINSTALL_REG_PATH}" "VersionMinor" ${VERSION_MINOR}
    ${if} ${silent} # MUI doesn't write language in silent mode
        WriteRegStr "${MUI_LANGDLL_REGISTRY_ROOT}" "${MUI_LANGDLL_REGISTRY_KEY}" \
            "${MUI_LANGDLL_REGISTRY_VALUENAME}" $LANGUAGE
    ${endif}

    File /r "${DIST_DIR}\*"

    !ifdef INSTALL_LICENSE
        File "${LICENSE_PATH}"
    !endif # INSTALL_LICENSE

    !endif # UNINSTALL_BUILDER
SectionEnd

!ifndef UNINSTALL_BUILDER

SectionGroup /e "Windows integration" SectionGroupWinIntegration
Section "Program Group Entry" SectionProgramGroup
    !insertmacro MUI_STARTMENU_WRITE_BEGIN ""

    CreateDirectory "$SMPROGRAMS\$startMenuFolder"
    CreateShortCut "$SMPROGRAMS\$startMenuFolder\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}"

    !ifdef INSTALL_LICENSE
        CreateShortCut "$SMPROGRAMS\$startMenuFolder\${LICENSE_START_MENU_LINK_NAME}.lnk" \
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

Section "Desktop Entry" SectionDesktopEntry
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
SectionEnd

Section /o "Start Menu Entry" SectionStartMenuEntry
    CreateShortCut "$STARTMENU\${PRODUCT_NAME}.lnk" "$INSTDIR\${EXE_NAME}"
SectionEnd

Section /o "AutoRun" SectionAutoRun
    WriteRegStr SHCTX "Software\Microsoft\Windows\CurrentVersion\Run" \
        "${PRODUCT_NAME}" "$\"$INSTDIR\${EXE_NAME}$\""
    AddSize 1
SectionEnd
SectionGroupEnd

Section "-Write Install Size" # Hidden section, write install size as the final step
    !insertmacro MULTIUSER_RegistryAddInstallSizeInfo
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionBirdTray} "${BIRDTRAY_SECTION_DESCRIPTION}"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionAutoRun} "${AUTO_RUN_DESCRIPTION}"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionGroupWinIntegration} \
            "${WIN_INTEGRATION_GROUP_DESCRIPTION}"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionProgramGroup} "${PROGRAM_GROUP_DESCRIPTION}"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionDesktopEntry} "${DESKTOP_ENTRY_DESCRIPTION}"
    !insertmacro MUI_DESCRIPTION_TEXT ${SectionStartMenuEntry} "${START_MENU_DESCRIPTION}"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

!endif # UNINSTALL_BUILDER

# === Uninstaller sections === #
!ifdef UNINSTALL_BUILDER
Section "un.${PRODUCT_NAME}" UNSectionBirdTray
    SectionIn RO # Can't deselect this section

    # TODO: Automate this part, so that new files don't have to be added manually here
    # Try to delete the EXE as the first step - if it's in use, don't remove anything else
    !insertmacro DeleteRetryAbort "$INSTDIR\${EXE_NAME}"
    !insertmacro DeleteRetryAbort "$INSTDIR\*.dll"
    RMDir /r "$INSTDIR\translations"
    RMDir /r "$INSTDIR\imageformats"
    RMDir /r "$INSTDIR\styles"
    RMDir /r "$INSTDIR\platforms"
    !ifdef LICENSE_FILE
        !insertmacro DeleteRetryAbort "$INSTDIR\${LICENSE_FILE}"
    !endif

    # Clean up "AutoRun"
    DeleteRegValue SHCTX "Software\Microsoft\Windows\CurrentVersion\Run" "${PRODUCT_NAME}"

    # Clean up "Program Group" - we check that we created a StartMenu folder,
    # or else the whole StartMenu directory will be removed!
    ${if} "$StartMenuFolder" != ""
        RMDir /r "$SMPROGRAMS\$StartMenuFolder"
    ${endif}
    # Clean up "Start Menu Icon"
    !insertmacro DeleteRetryAbort "$STARTMENU\${PRODUCT_NAME}.lnk"

    # Clean up "Dektop Icon"
    !insertmacro DeleteRetryAbort "$DESKTOP\${PRODUCT_NAME}.lnk"

SectionEnd

Section /o "un.${PRODUCT_NAME} User Settings" UNSectionUserSettings
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
    !insertmacro MUI_DESCRIPTION_TEXT ${UNSectionBirdTray} "${BIRDTRAY_SECTION_DESCRIPTION}"
    !insertmacro MUI_DESCRIPTION_TEXT ${UNSectionUserSettings} "${USER_SETTINGS_DESCRIPTION}"
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
        !insertmacro CheckSingleInstance "The setup of ${PRODUCT_NAME} is already running.$\r$\n \
            Please, close all instances of it and click Retry to continue, or Cancel to exit." \
            "Global" "${SETUP_MUTEX}"
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
        MessageBox MB_OK|MB_ICONEXCLAMATION \
                'The install path must not contain any of ${BAD_PATH_CHARS}.' /SD IDOK
        Abort
    ${endif}
    ${IsDirEmpty} $0 $INSTDIR
    ${if} $0 == 0
        MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
                'The install path is not empty. The content will get overwritten.' \
                /SD IDOK IDOK Ignore
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
        # Add "(current user only)" text to sections "Start Menu Icon" and "Quick Launch Icon"
        ${if} ${AtLeastWin7}
            SectionGetText ${SectionStartMenuEntry} $0
            SectionSetText ${SectionStartMenuEntry} "$0 (current user only)"
        ${endif}
    ${endif}
    !endif # UNINSTALL_BUILDER
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
            SendMessage $1 ${BCM_SETSHIELD} 0 1 # Hisplay SHIELD (Windows Vista and above)
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

Function .onInstFailed
    MessageBox MB_ICONSTOP "${PRODUCT_NAME} ${VERSION} could not be fully installed.$\r$\n\
        Please, restart Windows and run the setup program again." /SD IDOK
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

    ${ifNot} ${UAC_IsInnerInstance}
    ${andIf} $RunningFromInstaller == 0
        !insertmacro CheckSingleInstance "The uninstall of ${PRODUCT_NAME} is already running.$\r\
            $\n Please, close all instances of it and click Retry to continue, or Cancel to exit." \
            "Global" "${SETUP_MUTEX}"
    ${endif}

    !insertmacro MULTIUSER_UNINIT

    # We always get the language, since the outer and inner instance might have different language
    !insertmacro MUI_UNGETLANGUAGE

    !insertmacro STOP_PROCESS ${EXE_NAME} "${PRODUCT_NAME} is currently running. \
        Press OK to stop ${PRODUCT_NAME} to continue with the uninstall." \
        "Failed to close ${PRODUCT_NAME}. Please retry or quit ${PRODUCT_NAME} manually \
        to continue with the uninstall."
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

# Called when leaving the "Confirm Uninstall" page
Function un.ConfirmPageLeave
    ${ConfirmUninstallPath} $INSTDIR $R0
    ${if} $R0 == 0
        ${if} $SemiSilentMode == 1
            Quit
        ${else}
            Abort
        ${endif}
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
        MessageBox MB_ICONSTOP "${PRODUCT_NAME} ${VERSION} could not be fully uninstalled.$\r$\n\
            Please, restart Windows and run the uninstaller again." /SD IDOK
    ${else}
        MessageBox MB_ICONSTOP "${PRODUCT_NAME} could not be fully installed.$\r$\n\
            Please, restart Windows and run the setup program again." /SD IDOK
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
