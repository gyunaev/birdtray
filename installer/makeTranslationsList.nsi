SetCompress off
Name "makeTranslationsList"
OutFile "makeTranslationsList.exe"
SilentInstall silent
RequestExecutionLevel user


# Create a file containing NSIS sections for translations in a directory.
# $R0 - OUTPUT_FILE: The file to write the sections to.
# $R1 - TRANSLATION_FOLDER: The directory that contains the translations.
Function CreateTranslationsList
	Pop $R1  # translations folder
	Pop $R0	 # output file

    FileOpen $R4 $R0 w
    FileWrite $R4 '!macro insert_translation_section name$\r$\n'
    FileWrite $R4 '  Section "$${name}"$\r$\n'
    FileWrite $R4 '    SetOutPath "$$INSTDIR\translations"$\r$\n'
    FileWrite $R4 '    File /nonfatal "$R1\*_$${name}.qm"$\r$\n'
    FileWrite $R4 '  SectionEnd$\r$\n'
    FileWrite $R4 '!macroend$\r$\n'

	ClearErrors
	FindFirst $R2 $R3 "$R1\main_*.qm"

CreateTranslationsList_Loop:
	IfErrors CreateTranslationsList_Done

	# Add section for translation
	FileSeek $R4 0 END
	StrCpy $R5 $R3 -3 5
	FileWrite $R4 '!insertmacro insert_translation_section "$R5"$\r$\n'

	ClearErrors
	FindNext $R2 $R3
	Goto CreateTranslationsList_Loop

CreateTranslationsList_Done:
	FindClose $R2
	FileClose $R4
FunctionEnd


!macro CreateTranslationsList DIR OUTPUT_FILE_PATH
    Push ${OUTPUT_FILE_PATH} # output file
    Push ${DIR}              # dir
    Call CreateTranslationsList
!macroend
!define CreateTranslationsList '!insertmacro CreateTranslationsList'

Section
    ${CreateTranslationsList} "${DIST_DIR}\translations" "${TRANSLATIONS_LIST_FILENAME}"
SectionEnd
