// The AI that holds the position of a player who left (DisconnectInProgress.md, D717): the base economy (Peasants, houses, camps, mill,
// farms) and the defence of the base and of the units, no far attack (no wave, no relay, no altar squad), no expansion (no building
// beyond BaseRadius of the centre), no duel sought or accepted. The brain (HOLAIBrain.as) asks these rules for the slots it holds
// (UHOLAIBrain::SetHeld); the rules are pure and tested alone (HOLAIHold_Test.as). BaseRadius is a provisional figure (Q-hold-scope).
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

	// Whether a part of the brain runs for a slot: all of them when it is not held; held, only the economy, the technologies, the
	// defence and the production of the defence units and of the Light Cavalry (Q-hold-scope = C: the answer of the user).
	bool Allows(bool bHeld, EHOLAIHoldPart Part)
	{
		if (!bHeld)
		{
			return true;
		}
		// Q-hold-scope = C (2026-11-05): and the Light Cavalry (the Stable, the cavalry queued: the mobile defence); the AI makes no other cavalry.
		return Part == EHOLAIHoldPart::Economy || Part == EHOLAIHoldPart::Technology || Part == EHOLAIHoldPart::Defence || Part == EHOLAIHoldPart::Military
			|| Part == EHOLAIHoldPart::Cavalry;
	}

	// The building the opening chose, unless it stands beyond the base while the slot holds (an expansion): then nothing.
	FHOLAIBuildChoice FilterBuild(bool bHeld, const FHOLAIBuildChoice& Choice, const FHOLAISummary& Summary, const FHOLAIHoldRules& Rules)
	{
		if (!bHeld || !Choice.bBuild || !Summary.bHasCentre)
		{
			return Choice;
		}
		FVector Away = (Choice.bAtSpot ? Choice.Spot : Choice.Anchor) - Summary.CentreLocation;
		Away.Z = 0;
		if (Away.Size() <= Rules.BaseRadius)
		{
			return Choice;
		}
		FHOLAIBuildChoice Refused;
		Refused.Nothing = "holding: beyond the base";
		return Refused;
	}

	// One step of the wave: held, none is launched and a wave out is called back; not held, the wave rules as they are.
	FHOLAIWaveStep WaveStep(bool bHeld, FHOLAIWave& Wave, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, bool bDefending, bool bRecall, FVector Assembly, float Now)
	{
		if (!bHeld)
		{
			return HOLAIArmy::UpdateWave(Wave, Summary, Data, bDefending, bRecall, Assembly, Now);
		}
		if (Wave.Phase == EHOLAIWavePhase::Home)
		{
			return FHOLAIWaveStep();
		}
		return HOLAIArmy::UpdateWave(Wave, Summary, Data, bDefending, true, Assembly, Now);
	}

	// A duel challenge received: held, always refused.
	FHOLAIDuelDecision Answer(bool bHeld, const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, int Noise)
	{
		FHOLAIDuelDecision D = HOLAIDuel::Answer(S, View, Noise);
		if (bHeld)
		{
			D.bYes = false;
			D.Reason = "holding the position: no duel is accepted";
		}
		return D;
	}

	// A duel to seek: held, never.
	FHOLAIDuelDecision Challenge(bool bHeld, const FHOLAIDuelSettings& S, const FHOLAIDuelView& View, bool bAllowed, int Noise)
	{
		FHOLAIDuelDecision D = HOLAIDuel::Challenge(S, View, bAllowed, Noise);
		if (bHeld)
		{
			D.bYes = false;
			D.Reason = "holding the position: no duel is sought";
		}
		return D;
	}
}
