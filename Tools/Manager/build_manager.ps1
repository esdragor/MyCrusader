# Builds the lobby manager (Manager/HOLManager, C#, M6 R2) into one self-contained .exe for Windows x64:
# <copy>\Saved\Manager\HOLManager.exe (no .NET install needed on the host PC).
# Run from Windows PowerShell:
#   powershell -ExecutionPolicy Bypass -File Tools\Manager\build_manager.ps1 [-Root <copy>] [-Test]
# -Test runs the C# tests (dotnet test) first and stops on a failure.
# Full log: <copy>\Saved\Logs\build_manager.log. Prints one line PASS / FAIL; exit code 0 / 1.
param(
    [string] $Root = "",
    [switch] $Test
)

$Project = if ($Root) { (Resolve-Path $Root).Path } else { (Resolve-Path "$PSScriptRoot\..\..").Path }
$Solution = Join-Path $Project "Manager\HOLManager.sln"
$App = Join-Path $Project "Manager\HOLManager\HOLManager.csproj"
$Output = Join-Path $Project "Saved\Manager"
$Exe = Join-Path $Output "HOLManager.exe"
$Log = Join-Path $Project "Saved\Logs\build_manager.log"
New-Item -ItemType Directory -Force (Split-Path $Log) | Out-Null
if (-not (Test-Path $App)) { Write-Host "build_manager : FAIL (no $App)"; exit 1 }
if (-not (Get-Command dotnet -ErrorAction SilentlyContinue)) { Write-Host "build_manager : FAIL (dotnet not found)"; exit 1 }

# A running manager locks its .exe: name it instead of failing in the middle of the publish.
$Running = Get-Process HOLManager -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $Exe }
if ($Running) { Write-Host "build_manager : FAIL (HOLManager.exe is running from $Output, pid $($Running.Id -join ', '))"; exit 1 }

"build_manager $(Get-Date -Format s)" | Set-Content -Encoding utf8 $Log
if ($Test) {
    & dotnet test $Solution --nologo 2>&1 | Out-File -Append -Encoding utf8 $Log
    if ($LASTEXITCODE -ne 0) { Write-Host "build_manager : FAIL (dotnet test, see $Log)"; exit 1 }
}

& dotnet publish $App --nologo -c Release -r win-x64 --self-contained -p:PublishSingleFile=true `
    -p:IncludeNativeLibrariesForSelfExtract=true -p:DebugType=none -o $Output 2>&1 | Out-File -Append -Encoding utf8 $Log
if ($LASTEXITCODE -ne 0 -or -not (Test-Path $Exe)) {
    $Errors = Select-String -Path $Log -Pattern ": error " | Select-Object -First 3 | ForEach-Object { $_.Line.Trim() }
    Write-Host "build_manager : FAIL (dotnet publish, see $Log)"
    $Errors | ForEach-Object { Write-Host "  $_" }
    exit 1
}
$SizeMb = [math]::Round((Get-Item $Exe).Length / 1MB, 1)
Write-Host "build_manager : PASS ($Exe, $SizeMb Mo)"
exit 0
