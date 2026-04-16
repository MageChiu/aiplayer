@echo off
setlocal

set SCRIPT_DIR=%~dp0
set PS_SCRIPT=%SCRIPT_DIR%build_windows.ps1

if not exist "%PS_SCRIPT%" (
  echo [Windows] 未找到 PowerShell 构建脚本: "%PS_SCRIPT%"
  exit /b 1
)

set CONFIG=%1
if "%CONFIG%"=="" set CONFIG=--release

where pwsh >nul 2>nul
if %ERRORLEVEL%==0 (
  pwsh -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %CONFIG%
  exit /b %ERRORLEVEL%
)

where powershell >nul 2>nul
if %ERRORLEVEL%==0 (
  powershell -ExecutionPolicy Bypass -File "%PS_SCRIPT%" %CONFIG%
  exit /b %ERRORLEVEL%
)

echo [Windows] 未找到 PowerShell，请先安装 PowerShell 7 或启用系统 PowerShell。
exit /b 1
