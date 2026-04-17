#!/usr/bin/env pwsh
param(
    [string]$Triplet = 'x64-windows'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RootDir = Resolve-Path (Join-Path $ScriptDir '..')
$DepsDir = Join-Path $RootDir '.deps'
$VcpkgRoot = if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { Join-Path $DepsDir 'vcpkg' }
$MpvRoot = Join-Path $DepsDir 'mpv'

New-Item -ItemType Directory -Force -Path $DepsDir | Out-Null

if (-not (Test-Path (Join-Path $VcpkgRoot '.git'))) {
    Write-Host "[deps] 初始化本地 vcpkg 到 $VcpkgRoot" -ForegroundColor Cyan
    git clone https://github.com/microsoft/vcpkg.git $VcpkgRoot
}

$vcpkgExe = Join-Path $VcpkgRoot 'vcpkg.exe'
if (-not (Test-Path $vcpkgExe)) {
    Write-Host '[deps] 启动 vcpkg bootstrap' -ForegroundColor Cyan
    & (Join-Path $VcpkgRoot 'bootstrap-vcpkg.bat') -disableMetrics
}

Write-Host "[deps] 使用 vcpkg triplet: $Triplet" -ForegroundColor Cyan
& $vcpkgExe install --triplet $Triplet --x-manifest-root=$RootDir

if (-not (Test-Path $MpvRoot)) {
    New-Item -ItemType Directory -Force -Path $MpvRoot | Out-Null
}

$clientHeader = Get-ChildItem -Path $MpvRoot -Recurse -File -Filter "client.h" -ErrorAction SilentlyContinue |
    Where-Object { $_.FullName -match '[\\/]mpv[\\/]client\.h$' } |
    Select-Object -First 1

if (-not $clientHeader) {
    Write-Host '[deps] 下载本地 mpv-dev SDK' -ForegroundColor Cyan
    $release = Invoke-RestMethod -Uri "https://api.github.com/repos/shinchiro/mpv-winbuild-cmake/releases/latest"
    $asset = $release.assets | Where-Object { $_.name -match '^mpv-dev-x86_64-.*\.7z$' } | Select-Object -First 1
    if (-not $asset) {
        throw "Cannot find mpv-dev-x86_64 asset in latest release."
    }

    $archivePath = Join-Path $DepsDir 'mpv-dev.7z'
    Invoke-WebRequest -Uri $asset.browser_download_url -OutFile $archivePath
    7z x $archivePath "-o$MpvRoot" -y
}

Write-Host "[deps] 本地依赖已准备完成" -ForegroundColor Green
Write-Host "  VCPKG_ROOT=$VcpkgRoot"
Write-Host "  VCPKG_TARGET_TRIPLET=$Triplet"
Write-Host "  CMAKE_TOOLCHAIN_FILE=$(Join-Path $VcpkgRoot 'scripts/buildsystems/vcpkg.cmake')"
Write-Host "  MPV_ROOT=$MpvRoot"
