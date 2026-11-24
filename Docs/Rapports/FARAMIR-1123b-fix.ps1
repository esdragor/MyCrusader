$R = 'C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Source\HeroesOfLiberty'
function Rep($rel, $pattern, $new) {
    $p = Join-Path $R $rel
    $t = [IO.File]::ReadAllText($p)
    $crlf = $t.Contains("`r`n")
    if ($crlf) { $new = $new.Replace("`r`n", "`n").Replace("`n", "`r`n") }
    $m = [regex]::Matches($t, $pattern)
    if ($m.Count -ne 1) { Write-Output "REFUS $rel $pattern ($($m.Count))"; return }
    $out = [regex]::Replace($t, $pattern, { param($x) $new.Replace('$ORIG', $x.Value) })
    $bom = ([IO.File]::ReadAllBytes($p)[0] -eq 0xEF)
    [IO.File]::WriteAllText($p, $out, (New-Object Text.UTF8Encoding $bom))
    Write-Output "ok $rel"
}
Rep 'Simulation\HOLHealProcessor.h' '[ \t]*static TArray<double> HonorPendingHp;' @'
$ORIG
	/** S1-e: adds the hit points a slot just healed to its pending Honor (no effect while the Honor is off). */
	static void AddHonorHp(int32 Slot, double Hp);
	/** S1-e: turns each slot's pending hit points into whole Honor points on its faction gauge (the Dawn's only), up to the cap. */
	static void CreditHonor(const UWorld* World);
'@
Rep 'Simulation\HOLHealProcessor.cpp' '[ \t]*TotalHealed \+= static_cast<double>\(After - Before\);\r?\n\t\tNewKept\.Add\(Healer' @'
		AddHonorHp(Healer.State->OwnerPlayer, static_cast<double>(After - Before));
$ORIG
'@
Rep 'Simulation\HOLHealProcessor.cpp' '[ \t]*Unit\.State->CurrentHealth = After;\r?\n[ \t]*TotalHealed \+= static_cast<double>\(After - Before\);' @'
$ORIG
			AddHonorHp(Zone.OwnerSlot, static_cast<double>(After - Before));
'@
Rep 'Simulation\HOLHealProcessor.cpp' '[ \t]*Kept = MoveTemp\(NewKept\);' @'
	CreditHonor(World);
$ORIG
'@
Rep 'Simulation\HOLHealProcessor.cpp' 'void UHOLHealProcessor::ConfigureHonor\(const int32 HpPerPoint, const int32 Cap\)' @'
void UHOLHealProcessor::AddHonorHp(const int32 Slot, const double Hp)
{
	if (HonorHpPerPoint > 0 && Hp > 0. && HonorPendingHp.IsValidIndex(Slot))
	{
		HonorPendingHp[Slot] += Hp;
	}
}

void UHOLHealProcessor::CreditHonor(const UWorld* World)
{
	AHOLGameState* GameState = HonorHpPerPoint > 0 && World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		return;
	}
	for (int32 Slot = 0; Slot < HonorPendingHp.Num(); ++Slot)
	{
		const int32 Points = FMath::FloorToInt32(HonorPendingHp[Slot] / HonorHpPerPoint);
		if (Points <= 0)
		{
			continue;
		}
		HonorPendingHp[Slot] -= static_cast<double>(Points) * HonorHpPerPoint;
		// Only the Dawn has the Honor gauge (faction 0, as AHOLGameState::GetSlotFaction); the others' healing earns nothing.
		if (GameState->GetSlotFaction(Slot) == 0)
		{
			GameState->SetFactionResource(Slot, FMath::Min(GameState->GetFactionResource(Slot) + Points, HonorCap));
		}
	}
}

$ORIG
'@
Rep 'Simulation\HOLHealProcessor.cpp' '#include "Match/HOLDiplomacyRules.h"' @'
$ORIG
#include "Match/HOLGameState.h"
'@
