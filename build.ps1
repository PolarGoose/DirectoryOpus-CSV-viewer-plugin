Function Info($msg) {
  Write-Host -ForegroundColor DarkGreen "`nINFO: $msg`n"
}

Function Error($msg) {
  Write-Host `n`n
  Write-Error $msg
  exit 1
}

Function CheckReturnCodeOfPreviousCommand($msg) {
  if(-Not $?) {
    Error "${msg}. Error code: $LastExitCode"
  }
}

Function GetVersion() {
  $gitCommand = Get-Command -Name git

  try { $tag = & $gitCommand describe --exact-match --tags HEAD 2> $null } catch { }
  if(-Not $?) {
    Info "The commit is not tagged. Use 'v0.0' as a tag instead"
    $tag = "v0.0"
  }

  return [version]($tag.TrimStart('v'))
}

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

$root = Resolve-Path $PSScriptRoot
$buildDir = "$root/out/release"
$version = GetVersion

Info "Version: '$version'"

Info "Find Visual Studio installation path"
$vswhereCommand = Get-Command -Name "${Env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$installationPath = & $vswhereCommand -prerelease -latest -property installationPath

Info "Open Visual Studio 2022 Developer PowerShell"
& "$installationPath\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64

Info "Cmake generate cache"
cmake `
  -S $root `
  -B $buildDir `
  -G Ninja `
  -D CMAKE_BUILD_TYPE=Release `
  -D CMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -D VCPKG_TARGET_TRIPLET=x64-windows-static-release `
  -D VERSION_MAJOR:STRING=$($version.Major) `
  -D VERSION_MINOR:STRING=$($version.Minor) 
CheckReturnCodeOfPreviousCommand "cmake cache failed"

Info "Cmake build"
cmake --build $buildDir
CheckReturnCodeOfPreviousCommand "cmake build failed"

Info "Create a zip archive from DLL"
Compress-Archive -Force -Path $buildDir/DirectoryOpus-CSV-viewer-plugin.dll -DestinationPath $buildDir/DirectoryOpus-CSV-viewer-plugin.zip
