@echo off
REM Build Ezcel (C++/Qt6 MinGW). Toolchain cai qua aqtinstall o C:\Qt.
setlocal
set QT_ROOT=C:\Qt\6.8.3\mingw_64
set MINGW=C:\Qt\Tools\mingw1310_64\bin
set CMAKE=C:\Qt\Tools\CMake_64\bin
set NINJA=C:\Qt\Tools\Ninja
set PATH=%MINGW%;%CMAKE%;%NINJA%;%QT_ROOT%\bin;%PATH%

cmake -G Ninja -S "%~dp0." -B "%~dp0build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=%QT_ROOT% -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++
if errorlevel 1 exit /b 1

cmake --build "%~dp0build" --parallel
if errorlevel 1 exit /b 1

echo BUILD_OK -^> "%~dp0build\Ezcel.exe"
endlocal
