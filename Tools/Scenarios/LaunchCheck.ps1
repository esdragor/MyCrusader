# Checks of the USER'S launch of a trial (preflight.ps1): stub, implemented after the red test launch_check_test.ps1.
function Get-LaunchProblems([string]$Root, [string]$Name, [string]$PlayerName = "") { return @() }
function Get-LogLaunchProblems([string[]]$Lines) { return @() }
function Get-UncommittedDependencies([string[]]$Porcelain, [string]$Name) { return @() }
