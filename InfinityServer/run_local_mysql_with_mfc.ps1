$ErrorActionPreference = "Stop"

$env:INFINITY_MYSQL_HOST = "127.0.0.1"
$env:INFINITY_MYSQL_PORT = "3306"
$env:INFINITY_MYSQL_DATABASE = "infinity"
$env:INFINITY_MYSQL_USER = "app"
$env:INFINITY_MYSQL_PASSWORD = "dev-password"

$connectorRoot = "C:\MySQL\MySQL Connector C++ 8.4"
$env:MYSQL_CONNECTOR_CPP_DIR = $connectorRoot
$env:PATH = "$connectorRoot\lib64;$connectorRoot\bin;$env:PATH"

$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$serverExe = Join-Path $root "InfinityServer\x64\Debug\InfinityServer.exe"
$mfcExe = Join-Path $root "x64\Debug\MfcTestClientSkeleton.exe"

if (-not (Test-Path $serverExe)) {
    throw "InfinityServer.exe not found. Build InfinityServer Debug|x64 first: $serverExe"
}

if (-not (Test-Path $mfcExe)) {
    throw "MfcTestClientSkeleton.exe not found. Build MfcTestClientSkeleton Debug|x64 first: $mfcExe"
}

Write-Host "Starting InfinityServer with MySQL $env:INFINITY_MYSQL_HOST`:$env:INFINITY_MYSQL_PORT"
$serverProcess = Start-Process -FilePath $serverExe -PassThru

Start-Sleep -Seconds 1

Write-Host "Starting MFC test client"
Start-Process -FilePath $mfcExe | Out-Null

Write-Host "InfinityServer PID: $($serverProcess.Id)"
