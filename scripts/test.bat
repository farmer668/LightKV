@echo off
setlocal

ctest --test-dir build --output-on-failure
exit /b %errorlevel%

