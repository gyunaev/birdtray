Unicode true
!addplugindir /x86-unicode nsisDependencies\Plugins\x86-unicode
!addincludedir nsisDependencies\Include
!addincludedir .

SetCompress off

!include nsArray.nsh
!include LogicLib.nsh
!include Log.nsh
!include StrFunc.nsh
${StrCase}
${StrRep}
${StrLoc}

!define TRANSLATION_MAP "TRANSLATION_MAP"
!define UTF8 65001
!define /math SIZEOF_REGISTER ${NSIS_MAX_STRLEN} - 24
!define /redef  /Math SIZEOF_REGISTER ${SIZEOF_REGISTER} * ${NSIS_CHAR_SIZE}
Name "makeTranslationsList"
OutFile "${INSTALLER_TRANSLATIONS_BUILDER_FILE}"
SilentInstall silent
RequestExecutionLevel user

# Write the string as Utf-8 to the given file.
# Sets error flag if the write failed.
# $0 - handle: The handle to the file to write.
# $1 - string: The string to write to the file.
!macro WriteUtf8ToFile handle string
    Push $0
    Push $1
    Push ${handle}
    System::Call \
        "KERNEL32::WideCharToMultiByte(i${UTF8},i0,ws,i-1,@r0,i${SIZEOF_REGISTER},p0,p0)i.s" \
        `${string}`
    Pop $1
    ${if} $1 == 0
    ${orIf} $1 > ${SIZEOF_REGISTER}
        SetErrors
        Pop ${handle}
    ${else}
        IntOp $1 $1 - 1 # Don't write \0
        System::Call "KERNEL32::WriteFile(ps,pr0,ir1,*i,p0)i.r1"
        ${if} $1 == 0
            SetErrors
        ${endif}
    ${endif}
    Pop $1
    Pop $0
!macroend
!define WriteUtf8ToFile '!insertmacro WriteUtf8ToFile'


# Parse the xml node at the given path as a message node.
# $0 - XML: The xml document.
# $R0 - X_PATH: The path to the message node.
# Returns:
# $1 - SUCCESS: 0 if source and translation were not read successfully.
# $2 - SOURCE: The text in the source element.
# $3 - TRANSLATION: The text in the translation element.
Function ParseMessageNode
    Exch $R0
    Exch
    Exch $0
    Push $1
    Push $2
    Push $3
    nsisXML::select "$R0/source"
    ${if} $2 != 0
        nsisXML::getText
        nsisXML::release $2
        Push $3
        nsisXML::select "$R0/translation"
        ${if} $2 != 0
            nsisXML::getText
            nsisXML::release $2
            Push $3
        ${else}
            Push $2
        ${endif}
    ${else}
        Push $2
        Push $2
    ${endif}
    Exch 6
    Pop $R0
    Exch 4
    Pop $0
    Pop $3
    Pop $2
    Exch $1
FunctionEnd

!macro ParseMessageNode XML X_PATH SUCCESS SOURCE TRANSLATION
    Push ${XML}
    Push "${X_PATH}"
    Call ParseMessageNode
    Pop ${SUCCESS}
    Pop ${SOURCE}
    Pop ${TRANSLATION}
!macroend
!define ParseMessageNode '!insertmacro ParseMessageNode'


# Load a translation XML file. The translation file contains a <!DOCTYPE TS> line,
# which nsisXML can not deal with.
# $0 - TRANSLATION_FILE: The translation file to load.
# Returns:
# $0 - TRANSLATION_XML: The loaded translation xml or 0 on failure.
Function LoadTranslationXml
    Exch $0
    Push $1
    Push $2
    ${Print} "LoadTranslationXml from $0"
    FileOpen $1 $0 r
    ${if} ${Errors}
        ${Fatal} "Unable to open translation file $0"
    ${endif}
    FileOpen $2 "$TEMP\translation.ts" w
    ${if} ${Errors}
        FileClose $1
        ${Fatal} "Unable to open temporary translation file $TEMP\translation.ts"
    ${endif}
    ${Do}
        FileRead $1 $0
        StrCmp $0 "<!DOCTYPE TS>$\r$\n" skip
        StrCmp $0 "<!DOCTYPE TS>$\n" skip
        FileWrite $2 $0
        skip:
    ${LoopUntil} ${Errors}
    FileClose $1
    FileClose $2
    ${if} ${Errors}
        ${Fatal} "Unable to copy translation to temporary translation file $TEMP\translation.ts"
    ${endif}
    nsisXML::create
    nsisXML::load "$TEMP\translation.ts"
    Delete "$TEMP\translation.ts"
    Pop $2
    Pop $1
    Exch $0
FunctionEnd

!macro LoadTranslationXml TRANSLATION_FILE TRANSLATION_XML
    Push ${TRANSLATION_FILE}
    Call LoadTranslationXml
    Pop ${TRANSLATION_XML}
!macroend
!define LoadTranslationXml '!insertmacro LoadTranslationXml'


# The installer_en.ts translation file is special, because it contains a mapping of
# English source translations to translation ids. Load and create this mapping, as well as a list of
# The mapping can be accessed via the TRANSLATION_MAP name.
# $R0 - MAPPING_FILE: The installer_en.ts file that contains the mapping.
Function LoadTranslationMapping
    Exch $R0  # Mapping file
    Push $R1
    Push $0
    Push $1
    Push $2
    ${Print} "LoadTranslationMapping from $R0"
    nsisXML::create
    ${LoadTranslationXml} $R0 $0
    ${if} $0 == 0
        ${Fatal} "Unable to load translation mapping file from $R0"
    ${endif}
    StrCpy $R1 1
    ${ParseMessageNode} $0 "/TS/context/message[$R1]" $1 $2 $3
    ${if} $1 == 0
        ${Fatal} "Unable to parse first message node in $R0"
    ${endif}
    ${do}
        nsArray::Set TRANSLATION_MAP /key=$2 $3
        IntOp $R1 $R1 + 1
        ${ParseMessageNode} $0 "/TS/context/message[$R1]" $1 $2 $3
    ${loopWhile} $1 != 0
    nsArray::Remove TRANSLATION_MAP "__LANGUAGE_NAME__"
    nsisXML::release $0
    Pop $2
    Pop $1
    Pop $0
    Pop $R1
    Pop $R0
FunctionEnd

!macro LoadTranslationMapping MAPPING_FILE
    Push ${MAPPING_FILE}
    Call LoadTranslationMapping
!macroend
!define LoadTranslationMapping '!insertmacro LoadTranslationMapping'

# Create a file containing NSIS translations for the installer.
# $R0 - OUTPUT_FILE: The file to write the translations to.
# $R1 - TRANSLATION_FOLDER: The directory that contains the translations.
Function CreateTranslations
    Pop $R1  # translations folder
    ${LoadTranslationMapping} "$R1\installer_en.ts"
    Pop $R0	 # output file
    FileOpen $R4 $R0 w
    ${if} ${Errors}
        ${Fatal} "Unable to open translation output file $R0"
    ${endif}

    FindFirst $R2 $R3 "$R1\installer_*.ts"
    ${doUntil} ${Errors}
        nsisXML::create
        ${LoadTranslationXml} "$R1\$R3" $0
        ${if} $0 == 0
            ${Fatal} "Failed to open translation at $R1\$R3"
        ${endif}
        ${ParseMessageNode} $0 "/TS/context/message/source[text()='__LANGUAGE_NAME__']/.." $1 $2 $3
        ${if} $1 == 0
            ${Fatal} "Failed to find the '__LANGUAGE_NAME__' child in $R1\$R3"
        ${endif}
        FileWrite $R4 '!insertmacro MUI_LANGUAGE "$3"$\r$\n'
        ${StrCase} $4 $3 "U"

        ${forEachIn} TRANSLATION_MAP $5 $6
            ${StrLoc} $1 $5 "'" ">"
            ${if} $1 == ""
                StrCpy $1 `'`
            ${else}
                ${StrLoc} $1 $5 '"' ">"
                ${if} $1 != ""
                    ${Fatal} "Source contains both single and double quotes, not supported"
                ${endif}
                StrCpy $1 `"`
            ${endif}
            ${ParseMessageNode} $0 `/TS/context/message/source[normalize-space(text())= \
                    normalize-space($1$5$1)]/..` $1 $2 $3
            ${if} $4 == "ENGLISH"
                StrCpy $3 $2  # The source is the English translation
            ${elseIf} $1 == 0
                StrCpy $3 $5  # Use the English translation as a fallback
                ${Print} "Warning: Falling back to english in $4 for '$5'"
            ${endif}
            StrCpy $1 `\r`
            StrCpy $2 `\n`
            ${StrRep} $3 "$3" "$\r$\n" "$$$1$$$2"
            ${StrRep} $3 "$3" "$\r" "$$$1$$$2"
            ${StrRep} $3 "$3" "$\n" "$$$1$$$2"
            ${WriteUtf8ToFile} $R4 "LangString $6 $${LANG_$4} $\`$3$\`$\r$\n"
            ${if} ${Errors}
                ${Fatal} "Failed to write '$3' to translation output file $R0"
            ${endif}
        ${next}
        nsisXML::release $0
        ClearErrors
        FindNext $R2 $R3
    ${loop}
    FindClose $R2
    FileClose $R4
    nsArray::Clear TRANSLATION_MAP
FunctionEnd

!macro CreateTranslations TRANSLATIONS_FOLDER OUTPUT_FILE_PATH
    Push ${OUTPUT_FILE_PATH} # output file
    Push ${TRANSLATIONS_FOLDER} # translations folder
    Call CreateTranslations
!macroend
!define CreateTranslations '!insertmacro CreateTranslations'


Section
    ${InitLog}
    System::Call "kernel32::GetCurrentDirectory(i ${NSIS_MAX_STRLEN}, t .r0)"
    GetFullPathName $0 "$0\..\src\translations"
    ${CreateTranslations} $0 "${INSTALLER_TRANSLATIONS_FILENAME}"
SectionEnd
