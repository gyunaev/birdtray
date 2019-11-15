SetCompress off
Name "makeUninstallList"
OutFile "makeUninstallList.exe"
SilentInstall silent
RequestExecutionLevel user

# Remove all occurrences of a string from the target string.
# $R0 - SPLITTER: The string to remove.
# $R1 - STR: The string to remove the occurrences of the splitter from.
# Returns:
# $R0 - OUT_STR: The resulting string.
Function StrStrip
    Exch $R0  # splitter
    Exch
    Exch $R1  # string
    Push $R2
    Push $R3
    Push $R4
    Push $R5
    StrLen $R5 $R0
    StrCpy $R2 -1
    IntOp $R2 $R2 + 1
    StrCpy $R3 $R1 $R5 $R2
    StrCmp $R3 "" +9
    StrCmp $R3 $R0 0 -3
    StrCpy $R3 $R1 $R2
    IntOp $R2 $R2 + $R5
    StrCpy $R4 $R1 "" $R2
    StrCpy $R1 $R3$R4
    IntOp $R2 $R2 - $R5
    IntOp $R2 $R2 - 1
    Goto -10
    StrCpy $R0 $R1
    Pop $R5
    Pop $R4
    Pop $R3
    Pop $R2
    Pop $R1
    Exch $R0
FunctionEnd
!macro StrStrip SPLITTER STR OUT_STR
    Push '${STR}'
    Push '${SPLITTER}'
    Call StrStrip
    Pop '${OUT_STR}'
!macroend
!define StrStrip '!insertmacro StrStrip'


# Truncate a file, removing all it's content.
# $R0 - FILE: The path to the file to truncate.
Function TruncateFile
    Pop $R0	 # file
    FileOpen $R1 $R0 w
    FileClose $R1
FunctionEnd


# Create a file containing NSIS delete commands for a folder structure.
# The given filter can be used to exclude a file name.
# $R0 - OUTPUT_FILE: The file to write the delete commands to.
# $R1 - FILTER: A filename that will be excluded from the resulting command list.
# $R2 - LOCAL_FOLDER: The current folder relative to the global folder to examine.
# $R3 - GLOBAL_FOLDER: The global root folder from where the search starts.
Function MakeRecurrentFileList
	Pop $R3  # global folder
	Pop $R2	 # local folder
	Pop $R1	 # filter
	Pop $R0	 # output file

	ClearErrors
	FindFirst $R4 $R5 "$R3\$R2\*.*"

MakeRecurrentFileList_Loop:
	IfErrors MakeRecurrentFileList_Done
	# check if it is folder
	IfFileExists "$R3\$R2\$R5\*.*"  0 MakeRecurrentFileList_file
	# directory
	StrCmp $R5 "." MakeRecurrentFileList_next			# skip current folder
	StrCmp $R5 ".." MakeRecurrentFileList_next			# skip parent folder
	# go Recurrent
	# save current variables
	Push $R5
	Push $R4
	Push $R3
	Push $R2
	Push $R1
	Push $R0

	# set parameters
	Push $R0		# output file
	Push $R1		# filter
	Push "$R2\$R5"	# local folder
	Push $R3		# global folder
	Call MakeRecurrentFileList
	# restore current variables
	Pop $R0
	Pop $R1
	Pop $R2
	Pop $R3
	Pop $R4
	Pop $R5

    Push $R1
	FileOpen $R6 $R0 a
    FileSeek $R6 0 END
    ${StrStrip} ".\" "$R2\$R5" $R1
    FileWrite $R6 "RMDir $\"$$INSTDIR\$R1$\"$\r$\n"
    FileClose $R6
    Pop $R1
	Goto MakeRecurrentFileList_next

MakeRecurrentFileList_file:
	# use filter
	StrCmp $R1 $R5 MakeRecurrentFileList_skip
	# filter found, add file to list
	Push $R1
	FileOpen $R6 $R0 a
	FileSeek $R6 0 END
	${StrStrip} ".\" "$R2\$R5" $R1
	FileWrite $R6 "!insertmacro DeleteRetryAbort $\"$$INSTDIR\$R1$\"$\r$\n"
	FileClose $R6
	Pop $R1

MakeRecurrentFileList_skip:
MakeRecurrentFileList_next:
	ClearErrors
	FindNext $R4 $R5
	Goto MakeRecurrentFileList_Loop

MakeRecurrentFileList_Done:
	FindClose $R4
FunctionEnd


!macro CreateFileList DIR OUTPUT_FILE_PATH FILTER
    Push ${OUTPUT_FILE_PATH} # output file
    Call TruncateFile        # delete output file if exist
    Push ${OUTPUT_FILE_PATH} # output file
    Push ${FILTER} 	         # filter
    Push "."                 # local dir
    Push ${DIR}              # global dir
    Call MakeRecurrentFileList
!macroend
!define CreateFileList '!insertmacro CreateFileList'

Section
    System::Call "kernel32::GetCurrentDirectory(i ${NSIS_MAX_STRLEN}, t .r0)"
    CreateDirectory "$0\uninstaller"
    ${CreateFileList} "${DIST_DIR}" "uninstaller\${UNINSTALL_LIST_FILENAME}" "${EXE_NAME}"
SectionEnd
