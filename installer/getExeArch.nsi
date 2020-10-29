Unicode true
!include FileFunc.nsh
!include LogicLib.nsh
!include Log.nsh
!define SCS_64BIT_BINARY 6

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
    ${InitLog}
    ${GetParameters} $0
    ${TrimQuotes} $0 $0
    GetFullPathName $0 $0
    IfFileExists "$0" ExeExists
        ${Fatal} "Given executable file does not exist: $0"
    ExeExists:
    StrCpy $1 0
    System::Call "kernel32::GetBinaryType(t r0, *i .r1) i.r2"
    ${if} $2 == 0
        ${Fatal} "Unable to determine executable type of $0: GetBinaryType FAILED (return code: $2)"
    ${endIf}
    FileOpen $0 "$EXEDIR\arch.nsh" w
    ${if} $1 == ${SCS_64BIT_BINARY}
        FileWrite $0 '!define ARCH "x64"'
    ${else}
        FileWrite $0 '!define ARCH "x86"'
    ${endIf}
    FileClose $0
SectionEnd
