// The AI's garrison exit (S4-b, D696 Q2 A): which of the AI's towers let their occupants out in this think. Pure function on the
// summary, built on HOLAIGarrison::OccupantsLeave (HOLAIGarrisonRules.as). The order itself (UngarrisonAll of the tower, owner = the AI's slot)
// is NOT here: it needs a C++ binding HOLAIGarrisonOrder::IssueAIUngarrisonOrder (TYPES_A_EXPOSER of FARAMIR-1124se).

namespace HOLAIGarrisonExit
{
	// STUB (red first): no tower empties.
	TArray<int> TowersToEmpty(int PlayerIndex, const FHOLAISummary& Summary, const TArray<float>& SinceLastHits, float CalmSeconds)
	{
		TArray<int> Empty;
		return Empty;
	}
}
