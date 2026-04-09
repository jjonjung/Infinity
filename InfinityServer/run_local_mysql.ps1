$env:INFINITY_MYSQL_HOST = "127.0.0.1"
$env:INFINITY_MYSQL_PORT = "3306"
$env:INFINITY_MYSQL_DATABASE = "infinity"
$env:INFINITY_MYSQL_USER = "app"
$env:INFINITY_MYSQL_PASSWORD = "dev-password"

$serverExe = "C:\Users\EJ\Desktop\Fork\Infinity\InfinityServer\InfinityServer\x64\Debug\InfinityServer.exe"

if (-not (Test-Path $serverExe)) {
    Write-Error "InfinityServer.exe not found: $serverExe"
    exit 1
}

Write-Host "Starting InfinityServer with MySQL $env:INFINITY_MYSQL_HOST`:$env:INFINITY_MYSQL_PORT"
& $serverExe
