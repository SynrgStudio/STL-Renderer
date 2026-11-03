<#
.SYNOPSIS
  Builds the release artifacts into dist/: MSI installer, portable zip and SHA-256 checksums.
.DESCRIPTION
  Run from a "Developer PowerShell for VS 2022" (cmake, ninja and cl on PATH) with the .NET SDK installed.
  WiX v5 is pinned in .config/dotnet-tools.json and restored automatically.
#>
[CmdletBinding()]
param(
    [string]$BuildDir = "build-release"
)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
Set-Location $root

function Invoke-Checked {
    param([string]$Exe, [string[]]$Arguments)
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe $($Arguments -join ' ') failed with exit code $LASTEXITCODE" }
}

$version = (Select-String -Path CMakeLists.txt -Pattern 'project\(STLRenderer VERSION ([0-9.]+)').Matches[0].Groups[1].Value
Write-Host "STL Renderer $version"

Invoke-Checked cmake @("-S", ".", "-B", $BuildDir, "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release")
Invoke-Checked cmake @("--build", $BuildDir)
Invoke-Checked ctest @("--test-dir", $BuildDir, "--output-on-failure")

$exe = Join-Path $root "$BuildDir\STLRenderer.exe"
$reported = (& $exe --version | Out-String).Trim()
if ($reported -ne "STL Renderer $version") { throw "Unexpected version output: $reported" }

$dist = Join-Path $root "dist"
Remove-Item -Recurse -Force $dist -ErrorAction SilentlyContinue
New-Item -ItemType Directory $dist | Out-Null

# License as RTF for the installer dialog.
$text = (Get-Content LICENSE -Raw) -replace '\\', '\\' -replace '\{', '\{' -replace '\}', '\}'
$text = $text -replace "`r?`n", "\par`r`n"
$rtf = Join-Path $root "$BuildDir\License.rtf"
Set-Content -Path $rtf -Encoding ascii -Value "{\rtf1\ansi\deff0{\fonttbl{\f0 Consolas;}}\f0\fs16`r`n$text}"

Invoke-Checked dotnet @("tool", "restore")
Invoke-Checked dotnet @("tool", "run", "wix", "--", "extension", "add", "-g", "WixToolset.UI.wixext/5.0.2")
$msi = Join-Path $dist "STLRenderer-Setup-$version.msi"
Invoke-Checked dotnet @("tool", "run", "wix", "--", "build", "installer\STLRenderer.wxs",
    "-arch", "x64", "-ext", "WixToolset.UI.wixext/5.0.2",
    "-d", "Version=$version", "-d", "SourceDir=$root", "-d", "ExePath=$exe", "-d", "LicenseRtf=$rtf",
    "-o", $msi)
Remove-Item (Join-Path $dist "*.wixpdb") -ErrorAction SilentlyContinue

$staging = Join-Path $root "$BuildDir\portable\STLRenderer-$version"
Remove-Item -Recurse -Force (Split-Path $staging) -ErrorAction SilentlyContinue
New-Item -ItemType Directory $staging | Out-Null
Copy-Item $exe, README.md $staging
Copy-Item LICENSE (Join-Path $staging "LICENSE.txt")
# An empty config.ini next to the exe enables portable mode (settings stay in this folder).
Set-Content -Path (Join-Path $staging "config.ini") -Encoding ascii -Value "# STL Renderer portable configuration"
Compress-Archive -Path $staging -DestinationPath (Join-Path $dist "STLRenderer-$version-portable-x64.zip")

Get-ChildItem $dist -File | ForEach-Object {
    "{0}  {1}" -f (Get-FileHash $_.FullName -Algorithm SHA256).Hash.ToLower(), $_.Name
} | Set-Content -Path (Join-Path $dist "SHA256SUMS.txt") -Encoding ascii

Get-ChildItem $dist | Format-Table Name, Length
