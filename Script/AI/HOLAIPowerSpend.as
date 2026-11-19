// L12 / A12 (priority 7 of the faction base): the AI's rule for spending its faction resource in a power (D688: "a simple rule of
// spending per power, tuned by level, no cheating"). Pure rule, no world, no brain wiring (nothing calls it yet), tested in
// Script/Tests/HOLAIPowerSpend_Test.as.
// Among the powers of the book it picks the one to cast now, or none (-1). A power can be picked only when (1) the book's own rule says
// Ready (HOLPowerBook::StateOf: coded effect, tier reached, cool-down over, stock enough: the refusals are not copied here), (2) the
// caller says its trigger holds (Wanted: "a group in difficulty", "a fight lost"... is the situation reading of the brain, not decided
// here: without it the rule would be the rejected "spend as soon as possible"), (3) the stock left after paying stays at or above the
// RESERVE (a parameter, one value per AI level, chosen by the caller: no number of design lives here). The first such power in the
// order of the book wins (no other order is written in the GDD). The AI only spends what it has: no cheating.
namespace HOLAIPowerSpend
{
	// The index in Rows of the power to cast now, or -1. The three arrays are parallel (one cool-down left in ms and one trigger flag per
	// row); arrays of different sizes are a caller bug and read as "nothing to cast".
	int Choose(const TArray<FHOLPowerRow>& Rows, const TArray<int>& CooldownLeftMs, const TArray<bool>& Wanted, int Stock, int Tier, int Reserve)
	{
		if (CooldownLeftMs.Num() != Rows.Num() || Wanted.Num() != Rows.Num())
		{
			return -1;
		}
		for (int Index = 0; Index < Rows.Num(); Index++)
		{
			if (!Wanted[Index])
			{
				continue;
			}
			if (HOLPowerBook::StateOf(Rows[Index], Stock, CooldownLeftMs[Index], Tier) != EHOLPowerState::Ready)
			{
				continue;
			}
			if (Stock - Rows[Index].Cost < Reserve)
			{
				continue;
			}
			return Index;
		}
		return -1;
	}
}
