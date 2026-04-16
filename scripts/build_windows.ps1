#!/usr/bin/env pwsh
param(
    [ValidateSet('--release', '--debug', '')]
    [string]$Config = '--release'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ProjectName = 'aiplayer'
$Configuration = if ($Config -eq '--debug') { 'Debug' } else { 'Release' }

function Test-Command {
    param([string]$Name)
    $null -ne (Get-Command $Name -ErrorAction SilentlyContinue)
}

Write-Host '[Windows] 检查依赖...' -ForegroundColor Cyan
$missing = @()

if (-not (Test-Command 'cmake')) { $missing += 'cmake' }
if (-not (Test-Command 'cl')) { $missing += 'MSVC (cl.exe)' }
if (-not (Test-Command 'pkg-config')) { $missing += 'pkg-config(可选)' }

$vcpkgRoot = $env:VCPKG_ROOT

if ($missing.Count -gt 0) {
    Write-Warning ("[Windows] 检测到缺少以下依赖: {0}" -f ($missing -join ', '))
    Write-Host '请确保已安装 Visual Studio 2022 (含 C++ 桌面开发)、CMake、Qt。' -ForegroundColor Yellow
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$SrcDir = Resolve-Path (Join-Path $ScriptDir '..')
$BuildDir = Join-Path $SrcDir 'build/windows'
$DistDir = Join-Path $SrcDir 'dist/windows'
$mpvDevRoot = Join-Path $SrcDir 'mpv-dev'

New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

$cmakeArgs = @(
    '-S', $SrcDir,
    '-B', $BuildDir,
    '-G', 'Visual Studio 17 2022',
    '-A', 'x64',
    "-DCMAKE_BUILD_TYPE=$Configuration"
)

if ($vcpkgRoot -and (Test-Path $vcpkgRoot)) {
    $toolchain = Join-Path $vcpkgRoot 'scripts/buildsystems/vcpkg.cmake'
    if (Test-Path $toolchain) {
        $cmakeArgs += "-DCMAKE_TOOLCHAIN_FILE=$toolchain"
    }
}

if (Test-Path $mpvDevRoot) {
    Write-Host "[Windows] 检测到本地 mpv-dev: $mpvDevRoot" -ForegroundColor Green
    $mpvInclude = Get-ChildItem -Path $mpvDevRoot -Recurse -Directory -Filter mpv -ErrorAction SilentlyContinue |
        Where-Object { Test-Path (Join-Path $_.FullName 'client.h') } |
        Select-Object -First 1
    $mpvLib = Get-ChildItem -Path $mpvDevRoot -Recurse -File -Include *.lib -ErrorAction SilentlyContinue |
        Where-Object { $_.Name -match '^(mpv|libmpv|mpv-2)\.lib$' } |
        Select-Object -First 1

    if ($mpvInclude) {
        $cmakeArgs += "-DMPV_INCLUDE_DIR=$($mpvInclude.Parent.FullName)"
    }
    if ($mpvLib) {
        $cmakeArgs += "-DMPV_LIBRARY=$($mpvLib.FullName)"
    }
}

Write-Host "[Windows] 运行 CMake 配置 (配置: $Configuration)..." -ForegroundColor Cyan
cmake @cmakeArgs

Write-Host '[Windows] 开始构建...' -ForegroundColor Cyan
cmake --build $BuildDir --config $Configuration

$exePath = Join-Path $BuildDir ("$Configuration/$ProjectName.exe")
if (-not (Test-Path $exePath)) {
    # 兼容 target 名可能为 AIPlayer
    $exePath = Join-Path $BuildDir ("$Configuration/AIPlayer.exe")
}

if (Test-Path $exePath) {
    $destExe = Join-Path $DistDir 'aiplayer.exe'
    Copy-Item $exePath $destExe -Force
    Write-Host "[Windows] 构建完成，可执行文件已复制到: $destExe" -ForegroundColor Green
    Write-Host "[Windows] dist 目录: $DistDir" -ForegroundColor Green
} else {
    Write-Warning "[Windows] 未找到预期的 exe 产物，请检查构建输出目录: $BuildDir"
}
