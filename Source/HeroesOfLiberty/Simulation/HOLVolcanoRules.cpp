// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLVolcanoRules.h"

namespace
{
	/** The length of the segment between two points on the ground plane (Z ignored). */
	float FlatSegment(const FVector& A, const FVector& B)
	{
		return static_cast<float>(FVector::Dist2D(A, B));
	}
}

float HOLVolcano::FFlow::Length() const
{
	float Total = 0.f;
	for (int32 Index = 1; Index < Points.Num(); ++Index)
	{
		Total += FlatSegment(Points[Index - 1], Points[Index]);
	}
	return Total;
}

float HOLVolcano::EruptionDuration(const FFlow& Flow, const float Speed)
{
	return Speed > 0.f ? Flow.Length() / Speed : 0.f;
}

float HOLVolcano::FrontDistance(const FFlow& Flow, const float Now, const float EruptAt, const float Speed)
{
	if (Speed <= 0.f || Now <= EruptAt)
	{
		return 0.f;
	}
	return FMath::Min((Now - EruptAt) * Speed, Flow.Length());
}

bool HOLVolcano::IsInLava(const FFlow& Flow, const float FrontDistance, const float HalfWidth, const FVector& Point, const float Radius)
{
	if (FrontDistance <= 0.f || Flow.Points.Num() < 2)
	{
		return false;
	}
	// The distance on the ground to the part of the polyline already run: segment by segment, the last one cut at the front.
	const double Reach = static_cast<double>(FMath::Max(HalfWidth, 0.f)) + static_cast<double>(FMath::Max(Radius, 0.f));
	const FVector2D P(Point.X, Point.Y);
	float Left = FrontDistance;
	for (int32 Index = 1; Index < Flow.Points.Num() && Left > 0.f; ++Index)
	{
		const FVector2D A(Flow.Points[Index - 1].X, Flow.Points[Index - 1].Y);
		const FVector2D B(Flow.Points[Index].X, Flow.Points[Index].Y);
		const float Segment = FlatSegment(Flow.Points[Index - 1], Flow.Points[Index]);
		if (Segment <= 0.f)
		{
			continue;
		}
		const FVector2D End = Left >= Segment ? B : A + (B - A) * (Left / Segment);
		const double Distance = FMath::PointDistToSegment(FVector(P, 0.0), FVector(A, 0.0), FVector(End, 0.0));
		if (Distance <= Reach)
		{
			return true;
		}
		Left -= Segment;
	}
	return false;
}
bool HOLVolcano::IsInCorridor(const FFlow& Flow, const float HalfWidth, const FVector& Point, const float Radius)
{
	return IsInLava(Flow, Flow.Length(), HalfWidth, Point, Radius);
}

FVector HOLVolcano::EvacuationPoint(const FFlow& Flow, const float HalfWidth, const FVector& Point, const float Radius, const float Beyond)
{
	if (!IsInCorridor(Flow, HalfWidth, Point, Radius))
	{
		return Point;
	}
	// The nearest point of the line (ground plane) and the segment it lies on.
	const FVector2D P(Point.X, Point.Y);
	double BestDistance = TNumericLimits<double>::Max();
	FVector2D Nearest = P;
	FVector2D SegmentDirection(1.0, 0.0);
	for (int32 Index = 1; Index < Flow.Points.Num(); ++Index)
	{
		const FVector2D A(Flow.Points[Index - 1].X, Flow.Points[Index - 1].Y);
		const FVector2D B(Flow.Points[Index].X, Flow.Points[Index].Y);
		const FVector2D AB = B - A;
		const double Length2 = AB.SizeSquared();
		if (Length2 <= 0.0)
		{
			continue;
		}
		const double T = FMath::Clamp(FVector2D::DotProduct(P - A, AB) / Length2, 0.0, 1.0);
		const FVector2D On = A + AB * T;
		const double Distance = FVector2D::Distance(P, On);
		if (Distance < BestDistance)
		{
			BestDistance = Distance;
			Nearest = On;
			SegmentDirection = AB.GetSafeNormal();
		}
	}
	FVector2D Away = P - Nearest;
	if (Away.SizeSquared() < 1.0)
	{
		// On the line itself: the left of the flow's direction.
		Away = FVector2D(-SegmentDirection.Y, SegmentDirection.X);
	}
	Away = Away.GetSafeNormal();
	const double Clear = static_cast<double>(EvacuationReach(HalfWidth, Radius)) + static_cast<double>(FMath::Max(Beyond, 0.f));
	FVector Target(Nearest.X + Away.X * Clear, Nearest.Y + Away.Y * Clear, Point.Z);
	if (IsInCorridor(Flow, HalfWidth, Target, Radius))
	{
		Target = FVector(Nearest.X - Away.X * Clear, Nearest.Y - Away.Y * Clear, Point.Z);
	}
	return Target;
}

TArray<HOLVolcano::FObstacleBox> HOLVolcano::GetObstacleBoxes(const FFlow& Flow, const float HalfWidth)
{
	TArray<FObstacleBox> Boxes;
	for (int32 Index = 1; Index < Flow.Points.Num(); ++Index)
	{
		const FVector2D A(Flow.Points[Index - 1].X, Flow.Points[Index - 1].Y);
		const FVector2D B(Flow.Points[Index].X, Flow.Points[Index].Y);
		const float Segment = FlatSegment(Flow.Points[Index - 1], Flow.Points[Index]);
		if (Segment <= 0.f)
		{
			continue;
		}
		FObstacleBox Box;
		Box.Centre = (A + B) * 0.5;
		Box.HalfLength = 0.5f * Segment + HalfWidth;
		Box.HalfWidth = HalfWidth;
		Box.YawDegrees = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(B.Y - A.Y, B.X - A.X)));
		Boxes.Add(Box);
	}
	return Boxes;
}

FVector HOLVolcano::PointAt(const FFlow& Flow, const float Distance)
{
	if (Flow.Points.IsEmpty())
	{
		return FVector::ZeroVector;
	}
	float Left = FMath::Max(Distance, 0.f);
	for (int32 Index = 1; Index < Flow.Points.Num(); ++Index)
	{
		const float Segment = FlatSegment(Flow.Points[Index - 1], Flow.Points[Index]);
		if (Left <= Segment && Segment > 0.f)
		{
			return FMath::Lerp(Flow.Points[Index - 1], Flow.Points[Index], static_cast<double>(Left / Segment));
		}
		Left -= Segment;
	}
	return Flow.Points.Last();
}

HOLVolcano::FCycle HOLVolcano::MakeCycle(const int32 Seed, const int32 Index, const float PreviousReArmAt, const FHOLVolcanoSettings& Settings, const float EruptionLength)
{
	// One stream per cycle: the same dates on every machine, and a cycle never depends on how often it was asked for.
	FRandomStream Stream(Seed * 7919 + Index * 104729 + 1);
	FCycle Cycle;
	const float WindowOpensAt = PreviousReArmAt + Settings.WindowOpensAfter;
	Cycle.WarningAt = WindowOpensAt + Stream.FRand() * Settings.WindowLength;
	Cycle.EruptAt = Cycle.WarningAt + Stream.FRandRange(Settings.WarningMin, Settings.WarningMax);
	Cycle.EruptEndAt = Cycle.EruptAt + (EruptionLength > 0.f ? EruptionLength : Settings.EruptionLength);
	Cycle.CooledAt = Cycle.EruptEndAt + Stream.FRandRange(Settings.CoolingMin, Settings.CoolingMax);
	Cycle.ReArmAt = Cycle.CooledAt + Stream.FRandRange(Settings.RestMin, Settings.RestMax);
	return Cycle;
}

HOLVolcano::FCycle HOLVolcano::AdvanceForAltar(const FCycle& Cycle, const float HeldAt, const FHOLVolcanoSettings& Settings)
{
	if (HeldAt >= Cycle.WarningAt || Settings.AltarAdvance <= 0.f)
	{
		return Cycle;
	}
	// The alert comes sooner by the setting but not before the altar was taken; the rest of the cycle follows it, the alert's length unchanged.
	const float Sooner = FMath::Min(Settings.AltarAdvance, Cycle.WarningAt - HeldAt);
	FCycle Moved = Cycle;
	Moved.WarningAt -= Sooner;
	Moved.EruptAt -= Sooner;
	Moved.EruptEndAt -= Sooner;
	Moved.CooledAt -= Sooner;
	Moved.ReArmAt -= Sooner;
	return Moved;
}

int32 HOLVolcano::DrawFlow(const int32 Seed, const int32 Index, const int32 FlowCount)
{
	if (FlowCount <= 0)
	{
		return INDEX_NONE;
	}
	// A stream of its own (another salt than the cycle's dates): drawing a flow never moves a date. No memory of the previous eruption.
	// The seed is hashed first: the stream's generator is linear, so neighbouring seeds (cycle 0, cycle 1) would draw alike otherwise.
	FRandomStream Stream(static_cast<int32>(MurmurFinalize32(HashCombineFast(MurmurFinalize32(static_cast<uint32>(Seed)), MurmurFinalize32(static_cast<uint32>(Index) + 31337u)))));
	return Stream.RandRange(0, FlowCount - 1);
}

int32 HOLVolcano::SiteSeed(const int32 MatchSeed, const int32 SiteIndex)
{
	// Hashed, not added: the cycles' streams are linear in the seed, so neighbouring seeds would erupt alike.
	return static_cast<int32>(HashCombineFast(MurmurFinalize32(static_cast<uint32>(MatchSeed)), MurmurFinalize32(static_cast<uint32>(SiteIndex) + 977u)));
}

HOLVolcano::FSiteState HOLVolcano::SiteStateAt(const bool bCatastrophe, const int32 MatchSeed, const int32 SiteIndex, const FSite& Site, const float Now,
	const FHOLVolcanoSettings& Settings, const float ManAtArmsSpeed, const FWakes* Wakes)
{
	FSiteState State;
	if (!bCatastrophe)
	{
		return State;
	}
	const int32 Seed = SiteSeed(MatchSeed, SiteIndex);
	const float Speed = Settings.LavaSpeed(ManAtArmsSpeed);
	float ReArmAt = 0.f;
	// Cycles follow one another; a match lasts well under an hour, so a few turns of the loop at most.
	for (int32 Index = 0; Index < 64; ++Index)
	{
		const int32 FlowIndex = DrawFlow(Seed, Index, Site.Flows.Num());
		const FFlow* Flow = FlowIndex != INDEX_NONE ? &Site.Flows[FlowIndex] : nullptr;
		FCycle Cycle = MakeCycle(Seed, Index, ReArmAt, Settings, Flow ? EruptionDuration(*Flow, Speed) : 0.f);
		// V8: the altar held in this cycle begins its alert then; the rest of the cycle, and the re-arming, follow it (D400).
		const float* HeldAt = Wakes ? Wakes->Find(Index) : nullptr;
		if (HeldAt)
		{
			Cycle = AdvanceForAltar(Cycle, *HeldAt, Settings);
			State.bWoken = true;
		}
		if (Now >= Cycle.ReArmAt)
		{
			ReArmAt = Cycle.ReArmAt;
			State.bWoken = false;
			continue;
		}
		State.CycleIndex = Index;
		State.FlowIndex = FlowIndex;
		State.WarningAt = Cycle.WarningAt;
		State.EruptAt = Cycle.EruptAt;
		if (Now < Cycle.WarningAt)
		{
			State.Phase = EHOLVolcanoPhase::Armed;
			State.bAltarLit = Site.Altar.bPresent && !State.bWoken;
		}
		else if (Now < Cycle.EruptAt)
		{
			State.Phase = EHOLVolcanoPhase::Warning;
			State.SecondsToErupt = Cycle.EruptAt - Now;
		}
		else if (Now < Cycle.EruptEndAt)
		{
			State.Phase = EHOLVolcanoPhase::Erupting;
		}
		else
		{
			State.Phase = Now < Cycle.CooledAt ? EHOLVolcanoPhase::Cooling : EHOLVolcanoPhase::Rest;
		}
		if (Flow)
		{
			State.FrontDistance = FrontDistance(*Flow, Now, Cycle.EruptAt, Speed);
			State.FrontPoint = PointAt(*Flow, State.FrontDistance);
		}
		return State;
	}
	State.Phase = EHOLVolcanoPhase::Rest;
	return State;
}

EHOLVolcanoPhase HOLVolcano::GetPhase(const bool bCatastrophe, const int32 Seed, const float Now, const FHOLVolcanoSettings& Settings, const float EruptionLength)
{
	if (!bCatastrophe)
	{
		return EHOLVolcanoPhase::Dormant;
	}
	float ReArmAt = 0.f;
	// Cycles follow one another; a match lasts well under an hour, so a few turns of the loop at most.
	for (int32 Index = 0; Index < 64; ++Index)
	{
		const FCycle Cycle = MakeCycle(Seed, Index, ReArmAt, Settings, EruptionLength);
		if (Now < Cycle.WarningAt)
		{
			return EHOLVolcanoPhase::Armed;
		}
		if (Now < Cycle.EruptAt)
		{
			return EHOLVolcanoPhase::Warning;
		}
		if (Now < Cycle.EruptEndAt)
		{
			return EHOLVolcanoPhase::Erupting;
		}
		if (Now < Cycle.CooledAt)
		{
			return EHOLVolcanoPhase::Cooling;
		}
		if (Now < Cycle.ReArmAt)
		{
			return EHOLVolcanoPhase::Rest;
		}
		ReArmAt = Cycle.ReArmAt;
	}
	return EHOLVolcanoPhase::Rest;
}

HOLVolcano::FSiteState HOLVolcano::SiteStateOfMatch(const FHOLMatchConfig& Config, const int32 SiteIndex, const FSite& Site, const float Now, const FHOLVolcanoSettings& Settings, const float ManAtArmsSpeed, const FWakes* Wakes)
{
	return SiteStateAt(Config.bCatastrophe, Config.Seed, SiteIndex, Site, Now, Settings, ManAtArmsSpeed, Wakes);
}

HOLVolcano::EAltarHold HOLVolcano::StepAltar(float& HeldSeconds, const int32 CampsInZone, const float DeltaSeconds, const FHOLVolcanoSettings& Settings)
{
	// D348's rule, as the ruin's: one camp in the zone holds, none lets the count run down, two camps or more freeze it.
	if (CampsInZone <= 0)
	{
		HeldSeconds = FMath::Max(0.f, HeldSeconds - Settings.AltarDecayRate * DeltaSeconds);
		return EAltarHold::Idle;
	}
	if (CampsInZone == 1)
	{
		HeldSeconds += DeltaSeconds;
		return EAltarHold::Holding;
	}
	return EAltarHold::Contested;
}

HOLVolcano::EAltarHold HOLVolcano::StepAltarOwned(float& HeldSeconds, int32& Holder, const int32 CampsInZone, const int32 SoleCamp, const float DeltaSeconds, const FHOLVolcanoSettings& Settings)
{
	// The count belongs to the camp that raised it: another camp alone in the zone starts from 0, it never takes over the loser's percentage.
	if (CampsInZone == 1 && Holder != SoleCamp)
	{
		HeldSeconds = 0.f;
		Holder = SoleCamp;
	}
	return StepAltar(HeldSeconds, CampsInZone, DeltaSeconds, Settings);
}

HOLVolcano::FAltarSend HOLVolcano::AltarSend(const FAltarSendInput& Input, const FHOLVolcanoSettings& Settings)
{
	const int32 Free = FMath::Max(0, Input.FreeSoldiers);
	const int32 Reserve = FMath::CeilToInt(Settings.AiAltarReserveShare * Free);
	const int32 Available = FMath::Max(0, Free - Reserve);
	const int32 Need = FMath::Max(Settings.AiAltarMinSend, FMath::CeilToInt(Settings.AiAltarMargin * Input.AdversaryStrength));
	FAltarSend Result;
	if (Need > Available)
	{
		// An army the AI cannot beat with its margin: the adversary's camp is bare. Hard strikes elsewhere; the others spare what they can.
		Result.bDiversion = Input.Level == EAltarLevel::Hard;
		Result.Squad = Result.bDiversion ? 0 : Available;
	}
	else
	{
		Result.Squad = Need;
	}
	// The soldiers already sent stay (no recall: the movement audit sees the to-and-fro as spins).
	Result.Squad = FMath::Max(Result.Squad, Input.AlreadySent);
	return Result;
}

int32 HOLVolcano::AltarHoldSquad(const FAltarHoldInput& Input, const FHOLVolcanoSettings& Settings)
{
	const int32 Kept = FMath::Max(0, Input.AlreadySent);
	if (Input.bAdversaryInZone || Input.NearestTraceDistance <= Settings.AiAltarSafeDistance)
	{
		return Kept;
	}
	const float Share = Input.Level == EAltarLevel::Easy ? Settings.AiAltarHoldShareEasy : Settings.AiAltarHoldShare;
	return FMath::Max(Kept, FMath::CeilToInt(Share * FMath::Max(0, Input.FreeSoldiers)));
}

float HOLVolcano::DistanceToFlow(const FFlow& Flow, const FVector& Point)
{
	double Best = TNumericLimits<double>::Max();
	if (Flow.Points.Num() == 1)
	{
		Best = FVector2D::Distance(FVector2D(Point), FVector2D(Flow.Points[0]));
	}
	for (int32 Index = 1; Index < Flow.Points.Num(); ++Index)
	{
		const FVector A(Flow.Points[Index - 1].X, Flow.Points[Index - 1].Y, 0.0);
		const FVector B(Flow.Points[Index].X, Flow.Points[Index].Y, 0.0);
		Best = FMath::Min(Best, static_cast<double>(FMath::PointDistToSegment(FVector(Point.X, Point.Y, 0.0), A, B)));
	}
	return static_cast<float>(Best);
}
