@echo off
setlocal
set /a ERROR_FILE_NOT_FOUND = 2
set /a ERROR_DIR_NOT_EMPTY = 145

rem  #### Parse command line parameters ####
if "x%~1" == "x" (
    goto Usage
)
if "%~1" == "/?" (
    goto Usage
)
set "exePath=%~1"
if not exist "%exePath%" (
    echo Birdtray executable not found at "%exePath%" 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
set "libSqlitePath=%~2"
if not exist "%libSqlitePath%" (
    echo Sqlite library not found at "%libSqlitePath%" 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)

for /F "tokens=* USEBACKQ" %%f in (`dir /b "%~3" 2^>nul ^| findstr libcrypto ^| findstr .dll` ) do (
    set "openSSLCryptoPath=%~3\%%f"
)
for /F "tokens=* USEBACKQ" %%f in (`dir /b "%~3" 2^>nul ^| findstr libssl ^| findstr .dll`) do (
    set "openSSLPath=%~3\%%f"
)
if not exist "%openSSLCryptoPath%" (
    echo OpenSSL crypto library not found at "%~3\libcrypto*.dll" 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
if not exist "%openSSLPath%" (
    echo OpenSSL library not found at "%~3\libssl*.dll" 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)

if "%~4" == "--install" (
    set "installAfterBuild=1"
)

rem  #### Check if required programs are available ####
for %%x in (windeployqt.exe) do (set winDeployQtExe=%%~$PATH:x)
if not defined winDeployQtExe (
    echo windeployqt.exe is not on the PATH 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
for %%x in (makensis.exe) do (set makeNsisExe=%%~$PATH:x)
if not defined makeNsisExe (
    echo makensis.exe is not on the PATH 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
for %%x in (g++.exe) do (set g++Exe=%%~$PATH:x)
if not defined g++Exe (
    echo g++.exe is not on the PATH 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
for %%x in (git.exe) do (set gitExe=%%~$PATH:x)
if not defined gitExe (
    echo git.exe is not on the PATH 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
for %%x in (curl.exe) do (set curlExe=%%~$PATH:x)
if not defined curlExe (
    echo curl.exe is not on the PATH 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
for %%x in (7z.exe) do (set sevenZExe=%%~$PATH:x)
if not defined sevenZExe (
    echo 7z.exe is not on the PATH 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)

rem  #### Create the deployment folder ####
set "deploymentFolder=%~dp0winDeploy"
echo Creating deployment folder at "%deploymentFolder%"...
rem  Clear the old deployment folder.
if exist "%deploymentFolder%" (
    rmdir /s /q "%deploymentFolder%" 1>nul
    if exist "%deploymentFolder%" (
        rmdir /s /q "%deploymentFolder%" 1>nul
        if exist "%deploymentFolder%" (
            echo Failed to delete the old deployment folder at "%deploymentFolder%" 1>&2
            exit /b %ERROR_DIR_NOT_EMPTY%
        )
    )
)
mkdir "%deploymentFolder%"
if errorLevel 1 (
    echo Failed to delete the old deployment folder at "%deploymentFolder%" 1>&2
    exit /b %errorLevel%
)
xcopy "%exePath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the Birdtray executable from "%exePath%" 1>&2
    echo to the deployment folder at "%deploymentFolder%" 1>&2
    exit /b %errorLevel%
)
xcopy "%libSqlitePath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the Sqlite library from "%libSqlitePath%" 1>&2
    echo to the deployment folder at "%deploymentFolder%" 1>&2
    exit /b %errorLevel%
)
xcopy "%openSSLCryptoPath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the OpenSSL crypto library from "%openSSLCryptoPath%" 1>&2
    echo to the deployment folder at "%deploymentFolder%" 1>&2
    exit /b %errorLevel%
)
xcopy "%openSSLPath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the OpenSSL library from "%openSSLPath%" 1>&2
    echo to the deployment folder at "%deploymentFolder%" 1>&2
    exit /b %errorLevel%
)
for /f "delims=" %%i in ("%exePath%") do (
    set "exeFileName=%%~nxi"
    set "translationDir=%%~di%%~pitranslations"
)
"%winDeployQtExe%" --release --no-system-d3d-compiler --no-quick-import --no-webkit2 ^
        --no-angle --no-opengl-sw "%deploymentFolder%\%exeFileName%"
if errorLevel 1 (
    echo Failed to create deployment folder: windeployqt.exe failed 1>&2
    exit /b %errorLevel%
)
if exist "%deploymentFolder%\imageformats" (
    for /f %%F in ('dir "%deploymentFolder%\imageformats" /b /a-d ^| findstr /vile "qico.dll"') do (
        del "%deploymentFolder%\imageformats\%%F" 1>nul
    )
)
rem  Copy translations
if exist "%translationDir%" (
    xcopy "%translationDir%" "%deploymentFolder%\translations" /q /y 1>nul
    if errorLevel 1 (
        echo Failed to copy the translations from "%translationDir%" 1>&2
        echo to the deployment folder at "%deploymentFolder%\translations" 1>&2
        exit /b %errorLevel%
    )
) else (
    echo Warning: Did not find translations directory at "%translationDir%"
)

rem  #### Download the installer dependencies ####
echo Downloading installer dependencies...
rem  Clear the old dependencies folder.
set "dependencyFolder=%~dp0nsisDependencies"
if exist "%dependencyFolder%" (
    rmdir /s /q "%dependencyFolder%" 1>nul
    if exist "%dependencyFolder%" (
        rmdir /s /q "%dependencyFolder%" 1>nul
        if exist "%dependencyFolder%" (
            echo Failed to delete the old nsis dependency folder at "%dependencyFolder%" 1>&2
            exit /b %ERROR_DIR_NOT_EMPTY%
        )
    )
)

"%gitExe%" clone -q "https://github.com/Drizin/NsisMultiUser.git" "%dependencyFolder%" 1>nul
if errorLevel 1 (
    echo Failed to clone NsisMultiUser 1>&2
    exit /b %errorLevel%
)
rmdir /s /q "%dependencyFolder%\.git" 1>nul
set "nsProcessUrl=https://nsis.sourceforge.io/mediawiki/images/1/18/NsProcess.zip"
"%curlExe%" --silent --output "%TEMP%\NsProcess.zip" "%nsProcessUrl%" 1>nul
if errorLevel 1 (
    echo Failed to download NsProcess 1>&2
    exit /b %errorLevel%
)
"%sevenZExe%" e -y -o"%TEMP%\NsProcess" "%TEMP%\NsProcess.zip" 1>nul
if errorLevel 1 (
    echo Failed to extract NsProcess 1>&2
    exit /b %errorLevel%
)
del "%TEMP%\NsProcess.zip" /F
xcopy "%TEMP%\NsProcess\nsProcess.nsh" "%dependencyFolder%\Include" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from "%TEMP%\NsProcess\nsProcess.nsh" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Include" 1>&2
    exit /b %errorLevel%
)
xcopy "%TEMP%\NsProcess\nsProcess.dll" "%dependencyFolder%\Plugins\x86-ansi" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from "%TEMP%\NsProcess\nsProcess.dll" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Plugins\x86-ansi" 1>&2
    exit /b %errorLevel%
)
xcopy "%TEMP%\NsProcess\nsProcessW.dll" "%dependencyFolder%\Plugins\x86-unicode" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from "%TEMP%\NsProcess\nsProcessW.dll" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Plugins\x86-unicode" 1>&2
    exit /b %errorLevel%
)
rmdir /s /q "%TEMP%\NsProcess" 1>nul
rename "%dependencyFolder%\Plugins\x86-unicode\nsProcessW.dll" nsProcess.dll 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from "%TEMP%\NsProcess\nsProcessW.dll" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Plugins\x86-unicode" 1>&2
    exit /b %errorLevel%
)
set "nsisXMLUrl=https://nsis.sourceforge.io/mediawiki/images/5/55/Xml.zip"
"%curlExe%" --silent --output "%TEMP%\nsisXML.zip" "%nsisXMLUrl%" 1>nul
if errorLevel 1 (
    echo Failed to download nsisXML 1>&2
    exit /b %errorLevel%
)
"%sevenZExe%" e -y -o"%TEMP%\nsisXML" "%TEMP%\nsisXML.zip" 1>nul
if errorLevel 1 (
    echo Failed to extract nsisXML 1>&2
    exit /b %errorLevel%
)
del "%TEMP%\nsisXML.zip" /F
xcopy "%TEMP%\nsisXML\XML.nsh" "%dependencyFolder%\Include" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the nsisXML library from "%TEMP%\nsisXML\XML.nsh" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Include" 1>&2
    exit /b %errorLevel%
)
xcopy "%TEMP%\nsisXML\xml.dll" "%dependencyFolder%\Plugins\x86-ansi" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the nsisXML library from "%TEMP%\nsisXML\xml.dll" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Plugins\x86-ansi" 1>&2
    exit /b %errorLevel%
)
xcopy "%TEMP%\nsisXML\xml.dll" "%dependencyFolder%\Plugins\x86-unicode" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the nsisXML library from "%TEMP%\nsisXML\xml.dll" 1>&2
    echo to the deployment folder at "%dependencyFolder%\Plugins\x86-unicode" 1>&2
    exit /b %errorLevel%
)
rmdir /s /q "%TEMP%\nsisXML" 1>nul

rem  #### Create the actual installer ####
echo Creating the installer...
"%makeNsisExe%" "%~dp0installer.nsi"
if errorLevel 1 (
    echo Failed to create installer: makensis.exe failed 1>&2
    exit /b %errorLevel%
)
echo Successfully created the installer

rem  #### Run the installer, if called with --install ####
if not defined installAfterBuild (
    goto :eof
)
for /F "tokens=* USEBACKQ" %%f in (`dir /b "%~dp0" 2^>nul ^| findstr Birdtray ^| findstr .exe`) do (
    set "installerExe=%~dp0%%f"
)
if "x%installerExe%" == "x" (
    echo Failed to start the installer: Unable to find the generated installer executable 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
echo Executing installer...
"%installerExe%"
exit /b %errorLevel%

goto :eof

: Usage
echo Creates the Birdtray installer. - Usage:
echo buildInstaller.bat exePath libSqlitePath openSSLPath [--install]
echo:
echo exePath:       The path to the birdtray.exe to include in the installer
echo libSqlitePath: The path to the libsqlite.dll that was used to compile the Birdtray exe
echo openSSLPath:   The path to the OpenSSL directory containing libcrypto*.dll and libssl*.dll
echo --install:     Optional parameter. If specified, executes the generated installer.
echo:
echo The following programs must be on the PATH: windeployqt, makensis, g++, git, curl and 7z.
echo The script also searches for translations in translations subdirectory
echo of the directory containing the birdtray executable.
goto :eof
