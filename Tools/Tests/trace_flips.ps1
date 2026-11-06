# Heading flips (> 25 degrees between two steps, both moving) of the units dumped by hol.Sanity.DumpUnit -3 in a scenario log, with what changed at the same time
# (target, place, slot) and whether a rear-slide way is set. Used for the neutrals_trace spin chain (NeutralsInProgress.md, 2026-11-05 Celeborn-6).
#   powershell -NoProfile -File <worktree>\Tools\Tests\trace_flips.ps1 -Log <Saved_Tests\Logs\neutrals_trace_204.log> -Unit 10
# One line per flip: step, position, turn, velocity before -> after, phase, no-progress seconds, slide/<changes>; then the totals by category.
param([Parameter(Mandatory=$true)][string]$Log,[Parameter(Mandatory=$true)][int[]]$Unit)
$re='SanityDump3 #(\d+) step (\d+) at \((-?\d+), (-?\d+)\) vel \((-?\d+), (-?\d+)\) ph (-?\d+) slot \((-?\d+), (-?\d+)\) ap(\d) at(\d) tgt #(-?\d+) \((-?\d+), (-?\d+)\) slide \((-?[\d\.]+), (-?[\d\.]+)\) flank (\d) place (\d)/(-?[\d\.]+) np ([\d\.]+) room (\d) loop ([\d\.]+)'
$tot=@{}
foreach($uu in $Unit){ $c=@(0,$uu)
  $f=$Log; $u=$uu
  $rows=@()
  Select-String -Path $f -Pattern "SanityDump3 #$u step" -Encoding UTF8 | ForEach-Object {
    $m=[regex]::Match($_.Line,$re); if($m.Success){ $g=$m.Groups
      $rows += [pscustomobject]@{S=[int]$g[2].Value;X=[int]$g[3].Value;Y=[int]$g[4].Value;VX=[double]$g[5].Value;VY=[double]$g[6].Value;Ph=[int]$g[7].Value;SX=[int]$g[8].Value;SY=[int]$g[9].Value;Ap=[int]$g[10].Value;At=[int]$g[11].Value;Tgt=[int]$g[12].Value;SlX=[double]$g[15].Value;SlY=[double]$g[16].Value;Pl=[int]$g[18].Value;Pa=[double]$g[19].Value;Np=[double]$g[20].Value} } }
  $n=0
  for($i=1;$i -lt $rows.Count;$i++){
    $a=$rows[$i-1];$b=$rows[$i]
    if($b.S -ne $a.S+1 -and $b.S -ne $a.S){ }
    $la=[math]::Sqrt($a.VX*$a.VX+$a.VY*$a.VY);$lb=[math]::Sqrt($b.VX*$b.VX+$b.VY*$b.VY)
    if($la -lt 100 -or $lb -lt 100){continue}
    $cos=($a.VX*$b.VX+$a.VY*$b.VY)/($la*$lb); if($cos -gt 1){$cos=1}; if($cos -lt -1){$cos=-1}
    $deg=[math]::Acos($cos)*180/[math]::PI
    if($deg -gt 25){
      $w=@(); foreach($k in -2..0){ $j=$i+$k; if($j -ge 1){ $p=$rows[$j-1];$q=$rows[$j]; if($p.Tgt -ne $q.Tgt){$w+='tgtchg'}; if($p.Pl -ne $q.Pl -or [math]::Abs($p.Pa-$q.Pa) -gt 0.01){$w+='placechg'}; if(($p.SX -ne $q.SX) -or ($p.SY -ne $q.SY)){$w+='slotchg'} } }
      $sl= if([math]::Abs($b.SlX)+[math]::Abs($b.SlY) -gt 0.01){'slide'}else{'noslide'}
      $key=(($w | Sort-Object -Unique) -join '+'); if(-not $key){$key='none'}; $key="$sl/$key"
      $tot[$key]++; $n++
      "{0} #{1} step {2} at ({3},{4}) {5:N0} deg v ({6:N0},{7:N0})->({8:N0},{9:N0}) ph {10} np {11} {12}" -f $c[0],$u,$b.S,$b.X,$b.Y,$deg,$a.VX,$a.VY,$b.VX,$b.VY,$b.Ph,$b.Np,$key
    }
  }
  "== $($c[0]) #$u rows=$($rows.Count) flips=$n"
}
"TOTAL by category:"; $tot.GetEnumerator() | Sort-Object Value -Descending | ForEach-Object { "{0} {1}" -f $_.Value,$_.Key }
