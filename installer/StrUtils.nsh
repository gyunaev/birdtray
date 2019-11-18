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
