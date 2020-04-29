!include FileFunc.nsh
!include LogicLib.nsh
!define SCS_64BIT_BINARY 6

Unicode true
SetCompress off
Name "getExeArch"
OutFile "GetExeArch.exe"
SilentInstall silent
RequestExecutionLevel user

# Remove quotes at the start and end of the string in $R0, if there are any.
# $R0: The string.
Function TrimQuotes
    Exch $R0
    Push $R1

    StrCpy $R1 $R0 1
    StrCmp $R1 `"` 0 +2
    StrCpy $R0 $R0 `` 1
    StrCpy $R1 $R0 1 -1
    StrCmp $R1 `"` 0 +2
    StrCpy $R0 $R0 -1

    Pop $R1
    Exch $R0
FunctionEnd
!macro _TrimQuotes Input Output
    Push `${Input}`
    Call TrimQuotes
    Pop ${Output}
!macroend
!define TrimQuotes `!insertmacro _TrimQuotes`

Section
    ${GetParameters} $0
    ${TrimQuotes} $0 $0
    GetFullPathName $0 $0
    IfFileExists "$0" ExeExists
        MessageBox MB_ICONSTOP "Given executable file does not exist: $0"
        setErrorLevel 2
        Quit
    ExeExists:
    StrCpy $1 0
    System::Call "kernel32::GetBinaryType(t r0, *i .r1) i.r2"
    ${If} $2 == 0
        MessageBox MB_ABORTRETRYIGNORE|MB_ICONSTOP \
            "Unable to determine executable type of $0: GetBinaryType FAILED (return code: $2)" \
             IDRETRY ExeExists IDIGNORE IgnoreError
        setErrorLevel 2
        Quit
        IgnoreError:
    ${EndIf}
    FileOpen $0 "$EXEDIR\arch.nsh" w
    ${If} $1 == ${SCS_64BIT_BINARY}
        FileWrite $0 '!define ARCH "x64"'
    ${Else}
        FileWrite $0 '!define ARCH "x86"'
    ${EndIf}
    FileClose $0
SectionEnd
