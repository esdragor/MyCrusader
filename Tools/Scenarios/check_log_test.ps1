# Test of CheckLog.ps1 (the lines check.ps1 prints), on a real scenario log when one is there, else on a fixture.
#   powershell -ExecutionPolicy Bypass -File Tools\Scenarios\check_log_test.ps1 [-Log <scenario log>]
# The engine's smoke tests (UnifiedErrorTests.cpp, run at frame 0 of every -game start) write 20 "LogAutomationTest: Error:
# Condition failed" under -culture=fr (their messages are compared to English text; 0 under -culture=en, SAURON-1116d): they
# are not the scenario's, so check.ps1 must print ONE line naming them, not the 20 lines, and still print the game's errors.
param([string] $Log = "")

. (Join-Path $PSScriptRoot "CheckLog.ps1")
$Fixture = @("[2026.11.16-14.50.36:679][  0]LogTemp: Error test: UE::UnifiedErrorTest::Empty: [Erreur vide]") +
    @(1..20 | ForEach-Object { "[2026.11.16-14.50.36:746][  0]LogAutomationTest: Error: Condition failed" }) +
    @("[2026.11.16-14.51.00:001][500]LogHOLTest: Display: Test sequence: step 1 waitlog Starting hero",
      "[2026.11.16-14.51.02:001][600]LogHOLTest: Error: Test sequence: something broke",
      "[2026.11.16-14.51.03:001][700]LogHOLTest: Display: Scenario zz: ready")
$Cases = @(@{ Name = "fixture"; Lines = $Fixture; Expected = 20 })
if ($Log -and (Test-Path $Log)) {
    $Real = @(Get-Content $Log)
    $Cases += @{ Name = "log $Log"; Lines = $Real; Expected = @($Real | Where-Object { $_ -match "LogAutomationTest: Error: Condition failed" }).Count }
}
$Failed = 0
foreach ($Case in $Cases) {
    $Out = @(Select-CheckLogLines $Case.Lines)
    $Raw = @($Out | Where-Object { $_ -match "LogAutomationTest: Error: Condition failed" }).Count
    $Summary = @($Out | Where-Object { $_ -match "^engine smoke tests: $($Case.Expected) 'Condition failed'" }).Count
    $Game = @($Out | Where-Object { $_ -match "LogHOLTest: Error|Scenario .*: ready" }).Count
    $NeedGame = if ($Case.Name -eq "fixture") { 2 } else { 0 }
    $Ok = ($Raw -eq 0) -and ($Summary -eq [int]($Case.Expected -gt 0)) -and ($Game -ge $NeedGame)
    Write-Host ("{0} : {1} (raw 'Condition failed' lines printed {2}, summary line {3}, game lines {4})" -f $Case.Name, $(if ($Ok) { "PASS" } else { "FAIL" }), $Raw, $Summary, $Game)
    if (-not $Ok) { $Failed++ }
}
if ($Failed -eq 0) { Write-Host "check_log_test : PASS"; exit 0 }
Write-Host "check_log_test : FAIL ($Failed case(s))"
exit 1
