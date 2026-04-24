@echo off
setlocal

set NDK_ROOT=C:\Users\EJ\AppData\Local\Android\Sdk\ndk\27.2.12479018
set JNILIBS=C:\Users\EJ\Desktop\Fork\Infinity\Intermediate\Android\arm64\gradle\app\src\main\jniLibs\arm64-v8a
set ADB=C:\Users\EJ\AppData\Local\Android\Sdk\platform-tools\adb.exe
set APK=C:\Users\EJ\Desktop\Fork\Infinity\Intermediate\Android\arm64\gradle\app\build\outputs\apk\debug\app-debug.apk
set LIBC=%NDK_ROOT%\toolchains\llvm\prebuilt\windows-x86_64\sysroot\usr\lib\aarch64-linux-android\libc++_shared.so

echo [1/3] Copying libc++_shared.so...
if not exist "%JNILIBS%" mkdir "%JNILIBS%"
copy /Y "%LIBC%" "%JNILIBS%\libc++_shared.so" >nul
if errorlevel 1 ( echo FAILED: copy libc++_shared.so & exit /b 1 )
echo Done.

echo [2/3] Gradle build...
call C:\Users\EJ\Desktop\Fork\Infinity\Intermediate\Android\arm64\gradle\gradlew.bat -p C:\Users\EJ\Desktop\Fork\Infinity\Intermediate\Android\arm64\gradle :app:assembleDebug -x ueAFSProjectAssembleDebug -x ueAFSProjectAssembleRelease
if errorlevel 1 ( echo FAILED: gradle build & exit /b 1 )
echo Done.

echo [3/3] Installing APK...
"%ADB%" uninstall com.cej.fighter 2>nul
"%ADB%" install "%APK%"
if errorlevel 1 ( echo FAILED: adb install & exit /b 1 )
echo Done. Launch the app.

endlocal
