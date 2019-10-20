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
set exePath="%~1"
if not exist "%exePath%" (
    echo Birdtray executable not found at %exePath% 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
set libSqlitePath="%~2"
if not exist "%libSqlitePath%" (
    echo Sqlite library not found at %libSqlitePath% 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)

for /F "tokens=* USEBACKQ" %%f in (`dir /b "%~3" 2^>nul ^| findstr libcrypto ^| findstr .dll` ) do (
    set openSSLCryptoPath="%~3\%%f"
)
for /F "tokens=* USEBACKQ" %%f in (`dir /b "%~3" 2^>nul ^| findstr libssl ^| findstr .dll`) do (
    set openSSLPath="%~3\%%f"
)
if not exist "%openSSLCryptoPath%" (
    echo OpenSSL crypto library not found at %~3\libcrypto*.dll 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
)
if not exist "%openSSLPath%" (
    echo OpenSSL library not found at %~3\libssl*.dll 1>&2
    exit /b %ERROR_FILE_NOT_FOUND%
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
set deploymentFolder="%~dp0winDeploy"
echo Creating deployment folder at %deploymentFolder%...
rem  Set the cwd to the directory of the batch file.
cd /D "%~dp0"
rem  Clear the old deployment folder.
if exist "%deploymentFolder%" (
    rmdir /s /q "%deploymentFolder%" 1>nul
    if exist "%deploymentFolder%" (
        rmdir /s /q "%deploymentFolder%" 1>nul
        if exist "%deploymentFolder%" (
            echo Failed to delete the old deployment folder at %deploymentFolder% 1>&2
            exit /b %ERROR_DIR_NOT_EMPTY%
        )
    )
)
mkdir "%deploymentFolder%"
if errorLevel 1 (
    echo Failed to delete the old deployment folder at %deploymentFolder% 1>&2
    exit /b %errorLevel%
)
xcopy "%exePath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the Birdtray executable from %exePath% 1>&2
    echo to the deployment folder at %deploymentFolder% 1>&2
    exit /b %errorLevel%
)
xcopy "%libSqlitePath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the Sqlite library from %libSqlitePath% 1>&2
    echo to the deployment folder at %deploymentFolder% 1>&2
    exit /b %errorLevel%
)
xcopy "%openSSLCryptoPath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the OpenSSL crypto library from %openSSLCryptoPath% 1>&2
    echo to the deployment folder at %deploymentFolder% 1>&2
    exit /b %errorLevel%
)
xcopy "%openSSLPath%" "%deploymentFolder%" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the OpenSSL library from %openSSLPath% 1>&2
    echo to the deployment folder at %deploymentFolder% 1>&2
    exit /b %errorLevel%
)
for /f "delims=" %%i in ("%exePath%") do set "exeFileName=%%~nxi"
"%winDeployQtExe%" --release --no-system-d3d-compiler --no-quick-import --no-webkit2 ^
        --no-angle --no-opengl-sw -no-svg "%deploymentFolder%\%exeFileName%"
if errorLevel 1 (
    echo Failed to create deployment folder: windeployqt.exe failed 1>&2
    exit /b %errorLevel%
)
if exist "%deploymentFolder%\imageformats" (
    for /f %%F in ('dir "%deploymentFolder%\imageformats" /b /a-d ^| findstr /vile "qico.dll"') do (
        del "%deploymentFolder%\imageformats\%%F" 1>nul
    )
)

rem  #### Download the installer dependencies ####
echo Downloading installer dependencies...
rem  Clear the old dependencies folder.
set dependencyFolder="%~dp0nsisDependencies"
if exist "%dependencyFolder%" (
    rmdir /s /q "%dependencyFolder%" 1>nul
    if exist "%dependencyFolder%" (
        rmdir /s /q "%dependencyFolder%" 1>nul
        if exist "%dependencyFolder%" (
            echo Failed to delete the old nsis dependencyFolder folder at %dependencyFolder% 1>&2
            exit /b %ERROR_DIR_NOT_EMPTY%
        )
    )
)

"%gitExe%" clone -q "https://github.com/Drizin/NsisMultiUser.git" "nsisDependencies" 1>nul
if errorLevel 1 (
    echo Failed to clone NsisMultiUser 1>&2
    exit /b %errorLevel%
)
rmdir /s /q "%dependencyFolder%\.git" 1>nul
set nsProcessUrl="https://nsis.sourceforge.io/mediawiki/images/1/18/NsProcess.zip"
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
    echo Failed to copy the NsProcess library from %TEMP%\NsProcess\nsProcess.nsh 1>&2
    echo to the deployment folder at %dependencyFolder%\Include 1>&2
    exit /b %errorLevel%
)
xcopy "%TEMP%\NsProcess\nsProcess.dll" "%dependencyFolder%\Plugins\x86-ansi" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from %TEMP%\NsProcess\nsProcess.dll 1>&2
    echo to the deployment folder at %dependencyFolder%\Plugins\x86-ansi 1>&2
    exit /b %errorLevel%
)
xcopy "%TEMP%\NsProcess\nsProcessW.dll" "%dependencyFolder%\Plugins\x86-unicode" /q /y 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from %TEMP%\NsProcess\nsProcessW.dll 1>&2
    echo to the deployment folder at %dependencyFolder%\Plugins\x86-unicode 1>&2
    exit /b %errorLevel%
)
rmdir /s /q "%TEMP%\NsProcess" 1>nul
rename "%dependencyFolder%\Plugins\x86-unicode\nsProcessW.dll" nsProcess.dll 1>nul
if errorLevel 1 (
    echo Failed to copy the NsProcess library from %TEMP%\NsProcess\nsProcessW.dll 1>&2
    echo to the deployment folder at %dependencyFolder%\Plugins\x86-unicode 1>&2
    exit /b %errorLevel%
)

rem  #### Create the actual installer ####
echo Creating the installer...
"%makeNsisExe%" "%~dp0installer.nsi"
if errorLevel 1 (
    echo Failed to create installer: makensis.exe failed 1>&2
    exit /b %errorLevel%
)

echo Successfully created the installer
goto :eof

: Usage
echo Creates the Birdtray installer. - Usage:
echo buildInstaller.bat exePath libSqlitePath openSSLPath
echo:
echo exePath:       The path to the birdtray.exe to include in the installer
echo libSqlitePath: The path to the libsqlite.dll that was used to compile the Birdtray exe
echo openSSLPath:   The path to the OpenSSL directory containing libcrypto*.dll and libssl*.ddl
echo:
echo The following programs must be on the PATH: windeployqt, makensis, g++, git, curl and 7z.
goto :eof
