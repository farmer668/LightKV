@echo off
setlocal

cmake -S . -B build
if errorlevel 1 exit /b %errorlevel%

cmake --build build
if errorlevel 1 exit /b %errorlevel%

