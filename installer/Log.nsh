Var stdout

# Initialize the logging system.
!macro InitLog
    Push $0
    System::Call 'kernel32::GetStdHandle(i -11)i.r0'
    System::Call 'kernel32::AttachConsole(i -1)'
    StrCpy $stdout $0
    Pop $0
!macroend
!define InitLog '!insertmacro InitLog'

# Print the message to stdout.
# $0 - MESSAGE: The message to print.
Function Print
    Exch $0
    DetailPrint "$0"
    Push $1
    StrCpy $1 "0"
    IfErrors 0 +2
    StrCpy $1 "1"
    FileWrite $stdout `$0$\r$\n`
    StrCmp $1 "1" 0 +2
    SetErrors
    Pop $1
    Pop $0
FunctionEnd

!macro Print MESSAGE
    Push "${MESSAGE}"
    Call Print
!macroend
!define Print '!insertmacro Print'

# Displays an error message and aborts the script.
# MESSAGE: The message to display.
!macro Fatal MESSAGE
    ${Print} "${MESSAGE}"
    setErrorLevel 2
    Quit
!macroend
!define Fatal '!insertmacro Fatal'
