# Ports of this copy's lobby tests (M6). Each copy (the main copy, a worktree) has its own range of 10 ports, so two
# copies testing at the same moment never share a test manager (TCP: both probed 7790 free, then one manager could not
# bind and the other copy's games went into its lobby, seen 2026-10-29, Elrond and Arwen) nor a match server (UDP: a
# « Carol » of another copy entered a server of Arwen's, every copy's servers started from 7900). Same table of copies
# as quick_check.ps1 (its net part, Get-AITestPort). Dot-source this file, then:
#   $CopyIndex = Get-CopyIndex $Project          0 the main copy, 1-6 the worktrees, 7-9 any other folder (hashed)
#   Get-FreeTcpPort (7790 + 10 * $CopyIndex)     the test manager's port (--urls): 7790-7889
#   7900 + 10 * $CopyIndex                       the first UDP port of its match servers (--FirstServerPort): 7900-7999
#   Get-FreeTcpPort (7590 + 10 * $CopyIndex)     a trial's manager (Tools\Scenariosi_lobby.ps1): 7590-7689
#   8000 + 10 * $CopyIndex                       a trial's match servers: 8000-8099
# Clear of quick_check's net part (UDP 7800-7899) and of the real manager of the shortcut (TCP 7700, servers from 7777).
function Get-CopyIndex([string] $ProjectPath) {
    $Name = Split-Path -Leaf $ProjectPath
    $Index = @{ "MyCrusader" = 0; "HeroesOfLiberty-Elrond" = 1; "HeroesOfLiberty-Arwen" = 2; "HeroesOfLiberty-Glorfindel" = 3;
        "HeroesOfLiberty-Haldir" = 4; "HeroesOfLiberty-Galadriel" = 5; "HeroesOfLiberty-Celeborn" = 6 }[$Name]
    if ($null -eq $Index) { $Index = 7; foreach ($Char in $Name.ToCharArray()) { $Index = 7 + ($Index + [int]$Char) % 3 } }
    return $Index
}
# The first TCP port of 127.0.0.1 free from $First (the range of 10 of this copy first; beyond, up to $First + 99, if
# the range is taken: a test of this copy still running).
function Get-FreeTcpPort([int] $First) {
    $Port = $First
    while ($Port -lt $First + 100) {
        try { $Probe = New-Object Net.Sockets.TcpListener([Net.IPAddress]::Loopback, $Port); $Probe.Start(); $Probe.Stop(); return $Port } catch { $Port++ }
    }
    throw "no free TCP port from $First"
}
