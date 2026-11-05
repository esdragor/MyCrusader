// The AI that holds the position of a player who left (Disconnect T2/T3, D717): STUB, every part allowed (the test is red first).
enum EHOLAIHoldPart
{
	Economy,
	Technology,
	Defence,
	Military,
	Market,
	Siege,
	Cavalry,
	Relays,
	Altar,
	AttackWave,
	DuelChallenge,
	DuelAccept
}

struct FHOLAIHoldRules
{
	float BaseRadius = 3000.f;
}

namespace HOLAIHold
{
	FHOLAIHoldRules DefaultRules()
	{
		return FHOLAIHoldRules();
	}

	bool Allows(bool bHeld, EHOLAIHoldPart Part)
	{
		return true;
	}

	FHOLAIBuildChoice FilterBuild(bool bHeld, const FHOLAIBuildChoice& Choice, const FHOLAISummary& Summary, const FHOLAIHoldRules& Rules)
	{
		return Choice;
	}

	FHOLAIWaveStep WaveStep(bool bHeld, FHOLAIWave& Wave, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, bool bDefending, bool bRecall, FVector Assembly, float Now)
	{
		return HOLAIArmy::UpdateWave(Wave, Summary, Data, bDefending, bRecall, Assembly, Now);
	}

	FHOLAIDuelDecision Answer(bool bHeld, const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, int Noise)
	{
		return HOLAIDuel::Answer(S, View, Noise);
	}

	FHOLAIDuelDecision Challenge(bool bHeld, const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, bool bAllowed, int Noise)
	{
		return HOLAIDuel::Challenge(S, View, bAllowed, Noise);
	}
}
