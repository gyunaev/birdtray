!include LogicLib.nsh
!include x64.nsh
!include nsProcess.nsh

!define ERROR_ALREADY_EXISTS 0x000000b7
!define ERROR_ACCESS_DENIED 0x5
!define ERROR_SIGNAL_REFUSED 0x9C

# === Functions === #

# Create a function with the given prefix to check for another running installer instance.
# PREFIX: The prefix of the function.
!macro CheckSingleInstanceFunc PREFIX
    # Check if there is another running instance of the installer.
    # $0 - ERR_DIALOG: The message of the dialog to display when another instance is running.
    # $1 - SCOPE: "Global" or "Local" (default if empty).
    # $2 - MUTEX_NAME: unique mutex name.
    Function ${PREFIX}CheckSingleInstance
        Push $3
        Push $4

        !if $1 == ""
            StrCpy $1 "Local"
        !endif

        try:
        System::Call 'kernel32::CreateMutex(i 0, i 0, t "$1\$2") i .r3 ?e'
        Pop $4 # The stack contains the result of GetLastError

        ${if} $4 = ${ERROR_ALREADY_EXISTS}
        ${orIf} $4 = ${ERROR_ACCESS_DENIED}
            # ERROR_ACCESS_DENIED means the mutex was created by another user
            # and we don't have access to open it, so application is running.
            MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "$0" /SD IDCANCEL IDCANCEL cancel
            System::Call 'kernel32::CloseHandle(i $3)' # For next CreateMutex call to succeed
            Goto try

            cancel:
            Quit
        ${endif}

        Pop $4
        Pop $3
    FunctionEnd
!macroend

# Create a function with the given prefix to try to delete a file or abort if not successful.
# PREFIX: The prefix of the function.
!macro DeleteRetryAbortFunc PREFIX
    # Delete a file and allow the user to choose to retry or abort in case of a failure.
    # $0 - FILE_PATH: The file to delete.
    Function ${PREFIX}DeleteRetryAbort
        # Unlike the File instruction, Delete doesn't abort (un)installation on error,
        # it just sets the error flag and skips the file as if nothing happened.
        try:
        ClearErrors
        Delete $0
        ${if} ${errors}
            MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION \
                "Error deleting file:$\r$\n$\r$\n$0$\r$\n$\r$\nClick Retry to try again, or$\r$\n\
                Cancel to stop the uninstall." /SD IDCANCEL IDRETRY try
            Abort "Error deleting file $0"
        ${endif}
    FunctionEnd
!macroend

# See [un.]DeleteRetryAbort.
!macro DeleteRetryAbort FILE_PATH
    Push $0
    StrCpy $0 "${FILE_PATH}"
    !ifdef __UNINSTALL__
        Call un.DeleteRetryAbort
    !else
        Call DeleteRetryAbort
    !endif
    Pop $0
!macroend

# See [un.]CheckSingleInstance.
!macro CheckSingleInstance TYPE SCOPE MUTEX_NAME
    Push $0
    Push $1
    Push $2

    StrCpy $0 "${TYPE}"
    StrCpy $1 "${SCOPE}"
    StrCpy $2 "${MUTEX_NAME}"
    !ifdef __UNINSTALL__
        Call un.CheckSingleInstance
    !else
        Call CheckSingleInstance
    !endif

    Pop $2
    Pop $1
    Pop $0
!macroend

!insertmacro CheckSingleInstanceFunc ""
!ifdef UNINSTALL_BUILDER
    !insertmacro DeleteRetryAbortFunc "un."
    !insertmacro CheckSingleInstanceFunc "un."
!else
    !insertmacro DeleteRetryAbortFunc ""

    # Run the uninstaller of a previous installation.
    # $1 - UNINSTALLER_PATH: The quoted path to the uninstaller.
    Function RunUninstaller
        StrCpy $0 0
        # The _? param stops the uninstaller from copying itself to the temporary directory,
        # which is the only way for ExecWait to work
        ExecWait '$1 /SS $2 _?=$3' $0
FunctionEnd
!endif # UNINSTALL_BUILDER

Var STR_HAYSTACK
Var STR_NEEDLE
Var STR_CONTAINS_VAR_1
Var STR_CONTAINS_VAR_2
Var STR_CONTAINS_VAR_3
Var STR_CONTAINS_VAR_4
Var STR_RETURN_VAR


# This function does a case sensitive searches for an occurrence of a substring in a string.
# Written by kenglish_hi, adapted from StrReplace written by dandaman32
# $0 - STR_NEEDLE: The string to search for.
# $1 - STR_HAYSTACK: The string to search in.
# Returns:
# $0 The substring if found, else "".
Function StrContains
    Exch $STR_NEEDLE
    Exch 1
    Exch $STR_HAYSTACK
    StrCpy $STR_RETURN_VAR ""
    StrCpy $STR_CONTAINS_VAR_1 -1
    StrLen $STR_CONTAINS_VAR_2 $STR_NEEDLE
    StrLen $STR_CONTAINS_VAR_4 $STR_HAYSTACK
    loop:
        IntOp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_1 + 1
        StrCpy $STR_CONTAINS_VAR_3 $STR_HAYSTACK $STR_CONTAINS_VAR_2 $STR_CONTAINS_VAR_1
        StrCmp $STR_CONTAINS_VAR_3 $STR_NEEDLE found
        StrCmp $STR_CONTAINS_VAR_1 $STR_CONTAINS_VAR_4 done
        Goto loop
    found:
        StrCpy $STR_RETURN_VAR $STR_NEEDLE
        Goto done
    done:
    Pop $STR_NEEDLE
    Exch $STR_RETURN_VAR
FunctionEnd
!macro _StrContainsConstructor OUT NEEDLE HAYSTACK
    Push `${HAYSTACK}`
    Push `${NEEDLE}`
    Call StrContains
    Pop `${OUT}`
!macroend
!define StrContains '!insertmacro "_StrContainsConstructor"'

Var STR_CAO_CHARACTERS
Var STR_CAO_STRING
Var STR_CAO_RETURN_VAR

# This function does a case sensitive searches for an occurrence of
# any of the given characters in a string.
# $0 - STRING: The string to search trough.
# $1 - CHARACTERS: The characters to search for.
# Returns:
# $0 - FOUND: 1 if any character was found, 0 otherwise.
Function StrContainsAnyOf
    Pop $STR_CAO_STRING
    Pop $STR_CAO_CHARACTERS
    StrCpy $STR_CAO_RETURN_VAR 0
    StrCpy $R1 0
    loop:
    StrCpy $R2 $STR_CAO_CHARACTERS 1 $R1
    StrCmp $R2 '' end
    IntOp $R1 $R1 + 1
    ${StrContains} $R0 $R2 $STR_CAO_STRING
    ${if} $R0 != ""
        StrCpy $STR_CAO_RETURN_VAR 1
        goto end
    ${endif}
    goto loop
    end:
    Push $STR_CAO_RETURN_VAR
FunctionEnd
!macro _StrContainsAnyOfConstructor OUT STRING CHARACTERS
    Push `${CHARACTERS}`
    Push `${STRING}`
    Call StrContainsAnyOf
    Pop `${OUT}`
!macroend
!define StrContainsAnyOf '!insertmacro "_StrContainsAnyOfConstructor"'

Var validPath_path
Var validPath_bad_chars

# Check if a path is valid.
# $0 - BAD_CHARS: Characters that are not allowed in the path.
# $1 - PATH: The path to check.
# Returns:
# $0 - VALID: 1 if PATH contains only valid chars, 0 otherwise.
Function ValidPath
    Pop $validPath_bad_chars
    Pop $validPath_path
    Push $R0
    StrLen $R0 $validPath_path
    StrCpy $validPath_path $validPath_path $R0 2 # Skip drive (e.g. 'C:')
    ${StrContainsAnyOf} $R0 $validPath_path $validPath_bad_chars
    Exch $R0
FunctionEnd
!macro ValidPath VAR PATH BAD_CHARACTERS
    Push `${PATH}`
    Push `${BAD_CHARACTERS}`
    call ValidPath
    Pop `${VAR}`
!macroend
!define ValidPath "!insertmacro ValidPath"

# Check whether the given directory is empty.
# $0 - DIR: The path of the directory.
# Returns:
# $0 - IS_EMPTY: 1 (true) if empty and 0 (false) if not empty.
Function IsDirEmpty
    Exch $0
    Push $1
    Push $2
    StrCpy $2 1

    FindFirst  $0 $1 "$0\*.*" # $0 <= find handle, $1 <= first match
    ${doWhile} $1 != "" # Find returns "" when done
        ${if}    $1 != "."
        ${andIf} $1 != ".."
            StrCpy $2 0 # Result is false (not empty)
            ${break}
        ${endif}
        FindNext $0 $1
    ${loop}

    ClearErrors
    FindClose $0
    StrCpy $0 $2
    Pop  $2
    Pop  $1
    Exch $0
FunctionEnd
!macro IsDirEmpty VAR DIR
    Push ${DIR}
    call IsDirEmpty
    Pop ${VAR}
!macroend
!define IsDirEmpty "!insertmacro IsDirEmpty"

# Helper defines for Sections.nsh
!define SelectSection '!insertmacro SelectSection'
!define UnselectSection '!insertmacro UnselectSection'

# === Macros === #

# Check that the program is suitable for the platform.
# PLATFORM: The target platform of the installed executable.
!macro CheckPlatform PLATFORM
    ${if} ${RunningX64}
        !if ${PLATFORM} == x86
            MessageBox MB_OKCANCEL|MB_ICONINFORMATION "You are about to install the 32-bit version \
                of ${PRODUCT_NAME} on your 64-bit Windows. There is a 64-bit version of \
                ${PRODUCT_NAME} available for download." /SD IDOK IDOK Continue
            Quit
            Continue:
        !endif
    ${else}
        !if ${PLATFORM} == x64
            MessageBox MB_ICONSTOP "This installer contains the 64-bit version of ${PRODUCT_NAME}. \
                Your computer is running a 32-bit version of Windows, which can not execute 64-bit \
                programs. Please download the 32-bit installer of ${PRODUCT_NAME}." /SD IDOK
            Quit
        !endif
    ${endif}
!macroend

# Check that the minimum windows version requirement is met.
# MIN_WIN_VER: The minimal windows version.
!macro CheckMinWinVer MIN_WIN_VER
    ${ifNot} ${AtLeastWin${MIN_WIN_VER}}
        MessageBox MB_ICONSTOP "This program requires at least Windows ${MIN_WIN_VER}." /SD IDOK
        Quit
    ${endif}
!macroend

# Check if a registry key exists.
# ROOT: The registry root key.
# KEY: The key to check for.
# ELSE_JMP: The jump destination to jump to if the key does not exist.
!macro IfKeyExists ROOT KEY ELSE_JMP
    ClearErrors
    EnumRegKey $R0 ${ROOT} ${KEY} 0
    IfErrors ${ELSE_JMP} 0
!macroend
!define IfKeyExists '!insertmacro "IfKeyExists"'


# Hide a section and make sure it's unselected.
# SECTION_ID: The id of the section to exclude.
!macro EXCLUDE_SECTION SECTION_ID
    # In NSIS, the way you hide a section is by clearing its text
    ${UnselectSection} ${SECTION_ID}
    SectionSetText ${SECTION_ID} ""
!macroend

# Ensures that exe is not running anymore.
# EXE_NAME: The name of the executable to check for.
# DIALOG_FIRST: The dialog text to show if the executable was found the first time.
# DIALOG_KILL_FAILED: The dialog text to show after a kill attempt was unsuccessful.
!macro STOP_PROCESS EXE_NAME DIALOG_FIRST DIALOG_KILL_FAILED
    ${nsProcess::FindProcess} ${EXE_NAME} $R0 # Sets $R0 to 0 if found
    ${if} $R0 == 0
        MessageBox MB_OKCANCEL|MB_ICONQUESTION "${DIALOG_FIRST}" \
            /SD IDOK IDOK tryKill IDCANCEL stopFailed
        stopFailed:
            MessageBox MB_OK|MB_ICONSTOP \
                "Uninstalling ${PRODUCT_NAME} has been aborted. Please try again later." /SD IDOK
            SetErrorLevel ${ERROR_SIGNAL_REFUSED}
            Quit
        tryKill:
        ${doWhile} $R0 == 0
            DetailPrint "Closing process ${EXE_NAME}"
            ${nsProcess::CloseProcess} ${EXE_NAME} $R0
            ${if} $R0 == 0 # Successfully killed the process
                Sleep 100
                ${nsProcess::FindProcess} ${EXE_NAME} $R0 # Check to be sure
            ${else}
                StrCpy $R0 0
            ${endif}
            ${if} $R0 == 0
                MessageBox MB_RETRYCANCEL|MB_ICONEXCLAMATION "${DIALOG_KILL_FAILED}" \
                    /SD IDCANCEL IDRETRY 0 IDCANCEL stopFailed
                ${nsProcess::FindProcess} ${EXE_NAME} $R0 # Check again
            ${endif}
        ${loop}
    ${endif}
!macroend
