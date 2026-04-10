@echo off
setlocal

set "INFINITY_MYSQL_HOST=127.0.0.1"
set "INFINITY_MYSQL_PORT=3306"
set "INFINITY_MYSQL_DATABASE=infinity"
set "INFINITY_MYSQL_USER=app"
set "INFINITY_MYSQL_PASSWORD=dev-password"

set "MYSQL_CONNECTOR_CPP_DIR=C:\MySQL\MySQL Connector C++ 8.4"
set "PATH=%MYSQL_CONNECTOR_CPP_DIR%\lib64;%MYSQL_CONNECTOR_CPP_DIR%\bin;%PATH%"

set "ROOT=%~dp0"
set "SERVER_EXE=%ROOT%InfinityServer\x64\Debug\InfinityServer.exe"
set "MFC_EXE=%ROOT%x64\Debug\MfcTestClientSkeleton.exe"

if not exist "%SERVER_EXE%" (
    echo InfinityServer.exe not found.
    echo Build InfinityServer Debug x64 first:
    echo %SERVER_EXE%
    pause
    exit /b 1
)

if not exist "%MFC_EXE%" (
    echo MfcTestClientSkeleton.exe not found.
    echo Build MfcTestClientSkeleton Debug x64 first:
    echo %MFC_EXE%
    pause
    exit /b 1
)

echo Starting InfinityServer with MySQL %INFINITY_MYSQL_HOST%:%INFINITY_MYSQL_PORT%
start "InfinityServer" "%SERVER_EXE%"

timeout /t 1 /nobreak >nul

echo Starting MFC test client
start "MFC Test Client" "%MFC_EXE%"

endlocal
