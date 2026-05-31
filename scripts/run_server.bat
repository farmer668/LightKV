@echo off
setlocal

if exist build\Debug\lightkv_server.exe (
    build\Debug\lightkv_server.exe
    exit /b %errorlevel%
)

if exist build\Release\lightkv_server.exe (
    build\Release\lightkv_server.exe
    exit /b %errorlevel%
)

if exist build\lightkv_server.exe (
    build\lightkv_server.exe
    exit /b %errorlevel%
)

echo lightkv_server.exe not found. Please run scripts\build.bat first.
exit /b 1

