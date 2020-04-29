!addplugindir /x86-ansi nsisDependencies\Plugins\x86-ansi
!addplugindir /x86-unicode nsisDependencies\Plugins\x86-unicode
!addincludedir nsisDependencies\Include
!addincludedir .

#Unicode true # TODO: xml plugin does not support unicode, maybe use http://wiz0u.free.fr/prog/nsisXML/
SetCompress off

!include XML.nsh
!include LogicLib.nsh
!include StrFunc.nsh
!Include StrUtils.nsh
${StrCase}

Name "makeTranslationsList"
OutFile "${INSTALLER_TRANSLATIONS_BUILDER_FILE}"
SilentInstall silent
RequestExecutionLevel user


# Gets the text contents of a child node of the current xml node .
# $0 - NAME: The name of the child node.
# Returns:
# $0 - TEXT: The text of the child node.
# $1 - RESULT: 0 on success, -1 on failure.
Function GetNodeText
    Exch $1
    Push $0
    ${xml::FirstChildElement} "$1" $0 $0
    ${if} $0 != -1
        ${xml::GetText} $1 $0
        ${if} $0 != -1
            ${xml::Parent} $0 $0
        ${else}
            ${xml::Parent} $1 $1 # Preserve the error in $0
        ${endif}
    ${endif}
    Exch $0
    Exch 1
    Exch $1
FunctionEnd

!macro GetNodeText NODE RESULT TEXT
    Push ${NODE}
    Call GetNodeText
    Pop ${TEXT}
    Pop ${RESULT}
!macroend
!define GetNodeText '!insertmacro GetNodeText'

# Parse the current xml node as a message node.
# Returns:
# $0 - RESULT: 0 on success, -1 on failure.
# $1 - SOURCE: The text in the source element.
# $2 - TRANSLATION: The text in the translation element.
Function ParseMessageNode
    ${GetNodeText} "source" $0 $1
    ${if} $0 != -1
        ${GetNodeText} "translation" $0 $2
    ${endif}
    Push $2
    Push $1
    Push $0
FunctionEnd

!macro ParseMessageNode RESULT SOURCE TRANSLATION
    Call ParseMessageNode
    Pop ${RESULT}
    Pop ${SOURCE}
    Pop ${TRANSLATION}
!macroend
!define ParseMessageNode '!insertmacro ParseMessageNode'



# Create a file containing NSIS translations for the installer.
# $R0 - OUTPUT_FILE: The file to write the translations to.
# $R1 - TRANSLATION_FOLDER: The directory that contains the translations.
Function CreateTranslations
    Pop $R1  # translations folder
    Pop $R0	 # output file

    FileOpen $R4 $R0 w
    ClearErrors

    FindFirst $R2 $R3 "$R1\installer_*.ts"

CreateTranslations_Loop:
    IfErrors CreateTranslations_Done

    ${xml::LoadFile} "$R1\$R3" $0
    ${if} $0 = -1
        Abort "Failed to open translation at $R1\$R3"
    ${endif}
    ${xml::RootElement} $1 $0
    ${if} $0 = -1
        Abort "Failed to find the root element in $R1\$R3"
    ${endif}
    ${xml::FirstChildElement} "context" $1 $0
    ${if} $0 = -1
        Abort "Failed to find the 'context' child in $R1\$R3"
    ${endif}

    ${xml::FirstChildElement} "message" $1 $0
    ${doWhile} $0 != -1
        ${ParseMessageNode} $0 $1 $2
        ${if} $0 = -1
            Abort "Failed to parse message node"
        ${endif}
        ${if} $1 == "__LANGUAGE_NAME__"
            FileWrite $R4 '!insertmacro MUI_LANGUAGE "$2"$\r$\n'
            ${StrCase} $3 $2 "U"
            ${xml::RemoveNode} $0
            ${break}
        ${endif}
        ${xml::NextSiblingElement} "message" $1 $0
    ${loop}
    ${if} $0 = -1
        Abort "Failed to find the '__LANGUAGE_NAME__' child in $R1\$R3"
    ${endif}

    ${xml::FirstChildElement} "message" $1 $0
    ${doWhile} $0 != -1
        ${ParseMessageNode} $0 $1 $2
        ${if} $0 = -1
            Abort "Failed to parse message node"
        ${endif}
        ${StrContains} $0 "$${BAD_PATH_CHARS}" $2
        ${if} $0 == ""
            ${StrContains} $0 '"' $2
        ${endif}
        ${if} $0 != ""
            FileWrite $R4 "LangString $1 $${LANG_$3} '$2'$\r$\n"
        ${else}
            FileWrite $R4 'LangString $1 $${LANG_$3} "$2"$\r$\n'
        ${endif}
        ${xml::NextSiblingElement} "message" $1 $0
    ${loop}
    ClearErrors
    FindNext $R2 $R3
    Goto CreateTranslations_Loop

CreateTranslations_Done:
    FindClose $R2
    FileClose $R4
FunctionEnd


!macro CreateTranslations TRANSLATIONS_FOLDER OUTPUT_FILE_PATH
    Push ${OUTPUT_FILE_PATH} # output file
    Push ${TRANSLATIONS_FOLDER} # translations folder
    Call CreateTranslations
!macroend
!define CreateTranslations '!insertmacro CreateTranslations'

Section
    System::Call "kernel32::GetCurrentDirectory(i ${NSIS_MAX_STRLEN}, t .r0)"
    GetFullPathName $0 "$0\..\src\translations"
    ${CreateTranslations} $0 "${INSTALLER_TRANSLATIONS_FILENAME}"
SectionEnd
