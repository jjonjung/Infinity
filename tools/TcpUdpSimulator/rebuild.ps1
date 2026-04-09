$cmakeExe = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$msbuildExe = "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe"
$sourceDir = "C:\Users\EJ\Desktop\Fork\Infinity\tools\TcpUdpSimulator"
$buildDir = "C:\Users\EJ\Desktop\Fork\Infinity\tools\TcpUdpSimulator\build"
$solution = Join-Path $buildDir "TcpUdpSimulator.sln"

if (-not (Test-Path $cmakeExe)) {
    Write-Error "cmake.exe not found: $cmakeExe"
    exit 1
}

if (-not (Test-Path $msbuildExe)) {
    Write-Error "MSBuild.exe not found: $msbuildExe"
    exit 1
}

Write-Host "[1/2] Regenerating CMake build files..."
& $cmakeExe -S $sourceDir -B $buildDir
if ($LASTEXITCODE -ne 0) {
    Write-Error "CMake generation failed."
    exit $LASTEXITCODE
}

Write-Host "[2/2] Building TcpUdpSimulator.sln..."
& $msbuildExe $solution /p:Configuration=Debug /p:Platform=x64
if ($LASTEXITCODE -ne 0) {
    Write-Error "MSBuild failed."
    exit $LASTEXITCODE
}

Write-Host "TcpUdpSimulator rebuild completed."
