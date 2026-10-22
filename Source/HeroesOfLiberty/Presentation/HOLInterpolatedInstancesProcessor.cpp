// Copyright Epic Games, Inc. All Rights Reserved.

#include "Presentation/HOLInterpolatedInstancesProcessor.h"

#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "Buildings/HOLBuilding.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassLODFragments.h"
#include "MassRepresentationFragments.h"
#include "MassRepresentationSubsystem.h"
#include "MassUpdateISMProcessor.h"
#include "MassUpdateSkinnedMeshProcessor.h"
#include "MassVisualizationComponent.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Presentation/HOLCorpse.h"
#include "Presentation/HOLPresentationFragments.h"
#include "Presentation/HOLUnitAnimation.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLBuildFragments.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Presentation
{
	constexpr float AnimationBlendSeconds = 0.2f;

	/**
	 * Player colours (D233), chosen by the client: the units only carry their owner's index. The palette is shared
	 * with the menu (HOLPlayerColors). Written as linear base colour into the instance's custom data 0-2
	 * (M_UnitPlayerColor), followed by the prop shown (J5c.2, custom data 3, M_UnitPlayerColorProps; 0 for a type
	 * without props). Every type writes the four: the float count is one per component, and Mass ensures it.
	 * Plain floats: Mass copies the struct into its float array at any float offset (FVector4f is 16-byte aligned).
	 */
	struct FUnitCustomData
	{
		FVector3f Color;
		float Prop;
	};
	static_assert(sizeof(FUnitCustomData) == 4 * sizeof(float));

	/**
	 * The colour each slot chose (D244), as this machine knows it, gathered once per frame: OwnerPlayer is a slot, not a
	 * colour. A slot's buildings carry its colour (replicated, AI slots included), its player state too (humans); the
	 * slot's own index until either is known.
	 */
	/** Seconds between two gatherings of the slots' colours (FSlotColors). */
	constexpr double SlotColorsRefreshSeconds = 0.5;

	struct FSlotColors
	{
		TStaticArray<uint8, FHOLMatchConfig::MaxSlots> ColorIndices;

		FSlotColors() = default;

		explicit FSlotColors(const UWorld* World)
		{
			for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
			{
				ColorIndices[Slot] = static_cast<uint8>(Slot);
			}
			if (!World)
			{
				return;
			}
			for (TActorIterator<AHOLBuilding> It(World); It; ++It)
			{
				if (It->GetOwnerPlayer() < FHOLMatchConfig::MaxSlots)
				{
					ColorIndices[It->GetOwnerPlayer()] = It->GetColorIndex();
				}
			}
			if (const AGameStateBase* GameState = World->GetGameState())
			{
				for (const APlayerState* PlayerState : GameState->PlayerArray)
				{
					const AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
					if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() >= 0 && HOLPlayerState->GetPlayerIndex() < FHOLMatchConfig::MaxSlots)
					{
						ColorIndices[HOLPlayerState->GetPlayerIndex()] = static_cast<uint8>(HOLPlayerState->GetColorIndex());
					}
				}
			}
		}

		int32 Get(const uint8 OwnerPlayer) const
		{
			return OwnerPlayer < FHOLMatchConfig::MaxSlots ? ColorIndices[OwnerPlayer] : OwnerPlayer;
		}
	};

	FUnitCustomData GetCustomData(const int32 ColorIndex, const uint8 Prop)
	{
		return { HOLPlayerColors::GetLinearColor(ColorIndex), static_cast<float>(Prop) };
	}

	/** Start offset (s) of a unit in a sequence: stable per unit and sequence, so 2 000 units do not step in time. */
	float GetStartOffset(const uint32 UnitId, const int32 SequenceIndex, const float SequenceLength)
	{
		const uint32 Hash = HashCombineFast(GetTypeHash(UnitId), GetTypeHash(SequenceIndex));
		return SequenceLength * static_cast<float>(Hash & 0xFFFF) / 65536.f;
	}

	/**
	 * What a unit does as the clients have it (J7b.4, UHOLUnitReplicationSubsystem): Dying at 0 health, Attacking while it
	 * strikes (its fight, solo and server), else its activity. A client's display entity holds the received activity.
	 */
	EHOLUnitActivity GetShownActivity(const FHOLUnitStateFragment& State, const FHOLCombatFragment* Combat, const bool bDisplayEntity)
	{
		if (bDisplayEntity)
		{
			return State.Activity;
		}
		if (State.CurrentHealth <= 0 || State.Activity == EHOLUnitActivity::Dying)
		{
			return EHOLUnitActivity::Dying;
		}
		return Combat && Combat->bAttacking ? EHOLUnitActivity::Attacking : State.Activity;
	}

	/**
	 * Where a gatherer's source is (J5c.1), to draw it facing it: in solo its source in the simulation; on a client, which
	 * does not receive it, the standing source of the gathered resource nearest the unit (a gatherer stands against its
	 * source). False if none is known.
	 */
	bool FindGatheredSource(const UHOLSourceSubsystem* Sources, const FHOLGatherFragment* Gather, const FVector& Location,
		const EHOLResource Resource, FVector& OutLocation)
	{
		if (!Sources)
		{
			return false;
		}
		const int32 SourceIndex = Gather ? Gather->SourceIndex : Sources->FindNearestStanding(Location, Resource);
		if (!Sources->GetSources().IsValidIndex(SourceIndex))
		{
			return false;
		}
		OutLocation = Sources->GetSources()[SourceIndex].Location;
		return true;
	}

	/** How far (cm) beyond its body a client looks for the site of a builder: it works at contact (D312). */
	constexpr double MaxSiteGap = 200.;

	/**
	 * The footprints of the buildings not finished that this machine knows (J6a.4: sites, and its own bare foundations),
	 * gathered at the first need of the frame: a client knows only that a unit builds, not where.
	 */
	struct FSiteFootprints
	{
		UWorld* World = nullptr;
		bool bGathered = false;
		TArray<FBox2D, TInlineAllocator<16>> Footprints;

		TConstArrayView<FBox2D> Get()
		{
			if (!bGathered && World)
			{
				bGathered = true;
				for (TActorIterator<AHOLBuilding> It(World); It; ++It)
				{
					if (!It->IsFinished() && It->GetType())
					{
						Footprints.Add(It->GetFootprintBounds());
					}
				}
			}
			return Footprints;
		}
	};

	/**
	 * Where a builder's hammering is turned to (J6a.4): the point of its site's footprint nearest it (it stands beside one
	 * side). In solo its site in the simulation (FHOLBuildFragment); on a client, which does not receive it, the building
	 * not finished whose footprint is nearest the unit, within MaxSiteGap of its body. False if none is known.
	 */
	bool FindBuiltSite(FSiteFootprints& Sites, const FHOLBuildFragment* Build, const FVector& Location, const float BodyRadius,
		FVector& OutLocation)
	{
		const FVector2D Location2D(Location);
		FBox2D Footprint(ForceInit);
		if (Build)
		{
			if (Build->Phase == EHOLBuildPhase::Building)
			{
				Footprint = Build->SiteFootprint;
			}
		}
		else
		{
			double BestDistanceSquared = FMath::Square(BodyRadius + MaxSiteGap);
			for (const FBox2D& Candidate : Sites.Get())
			{
				const double DistanceSquared = Candidate.ComputeSquaredDistanceToPoint(Location2D);
				if (DistanceSquared <= BestDistanceSquared)
				{
					BestDistanceSquared = DistanceSquared;
					Footprint = Candidate;
				}
			}
		}
		if (!Footprint.bIsValid)
		{
			return false;
		}
		const FVector2D Nearest = Footprint.GetClosestPointTo(Location2D);
		OutLocation = FVector(Nearest.X, Nearest.Y, Location.Z);
		return true;
	}
}

UHOLInterpolatedInstancesProcessor::UHOLInterpolatedInstancesProcessor()
	: EntityQuery(*this)
	, CorpseQuery(*this)
	, CorpseExpiryQuery(*this)
{
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Client | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Representation);
	bRequiresGameThreadExecution = true;
}

void UHOLInterpolatedInstancesProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLPreviousTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassRepresentationAnimationFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLUnitAnimationStateFragment>(EMassFragmentAccess::ReadWrite);
	// A client's display entities (D271) carry the received velocity: there is no fixed step to measure one on.
	EntityQuery.AddRequirement<FHOLNetUnitDisplayFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	// In solo, the load and the source of a gatherer (J5c.1), the site of a builder (J6a.4); a client reads the received
	// state instead.
	EntityQuery.AddRequirement<FHOLGatherFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLBuildFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	// In solo, a fighter's strike (J7b.5): whether it strikes, its cool-down (the phase of its gesture) and its target.
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>();
	EntityQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	EntityQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddSubsystemRequirement<UMassRepresentationSubsystem>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FMassStaticRepresentationTag>(EMassFragmentPresence::None);

	// The bodies (J7c, bodies kept 30 s): drawn from FHOLCorpseFragment alone, as the units are (same representation).
	CorpseQuery.AddTagRequirement<FHOLCorpseTag>(EMassFragmentPresence::All);
	CorpseQuery.AddRequirement<FHOLCorpseFragment>(EMassFragmentAccess::ReadOnly);
	CorpseQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	CorpseQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
	CorpseQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadOnly);
	CorpseQuery.AddRequirement<FMassRepresentationAnimationFragment>(EMassFragmentAccess::ReadWrite);
	CorpseQuery.AddChunkRequirement<FMassVisualizationChunkFragment>(EMassFragmentAccess::ReadWrite);
	CorpseQuery.SetChunkFilter(&FMassVisualizationChunkFragment::AreAnyEntitiesVisibleInChunk);
	CorpseQuery.AddSharedRequirement<FMassRepresentationSubsystemSharedFragment>(EMassFragmentAccess::ReadWrite);
	CorpseQuery.AddSubsystemRequirement<UMassRepresentationSubsystem>(EMassFragmentAccess::ReadWrite);
	CorpseQuery.AddTagRequirement<FMassStaticRepresentationTag>(EMassFragmentPresence::None);

	// Their expiry, seen or not.
	CorpseExpiryQuery.AddTagRequirement<FHOLCorpseTag>(EMassFragmentPresence::All);
	CorpseExpiryQuery.AddRequirement<FHOLCorpseFragment>(EMassFragmentAccess::ReadOnly);
}

void UHOLInterpolatedInstancesProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Presentation;

	UWorld* World = EntityManager.GetWorld();
	const UHOLSimulationSubsystem* Simulation = World ? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
	const float Alpha = Simulation ? Simulation->GetInterpolationAlpha() : 1.f;
	const float StepSeconds = Simulation ? Simulation->GetStepSeconds() : 0.f;
	const UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	FSiteFootprints Sites;
	Sites.World = World;
	// The slots' colours, gathered from the actors at most twice a second (they change only when a player picks one in the
	// lobby or a building of a new slot appears), not at every frame.
	const double Now = FPlatformTime::Seconds();
	if (CachedColorIndices.Num() != FHOLMatchConfig::MaxSlots || Now - ColorsRefreshedAt >= UE::HeroesOfLiberty::Presentation::SlotColorsRefreshSeconds)
	{
		const FSlotColors Fresh(World);
		CachedColorIndices.SetNumUninitialized(FHOLMatchConfig::MaxSlots);
		for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
		{
			CachedColorIndices[Slot] = Fresh.ColorIndices[Slot];
		}
		ColorsRefreshedAt = Now;
	}
	FSlotColors SlotColors;
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		SlotColors.ColorIndices[Slot] = CachedColorIndices[Slot];
	}

	// The units' speed of the cheats (hol.Units.SpeedScale, this machine's: a client's is not the server's), for the walks'
	// play rate (J7a.4).
	static IConsoleVariable* const SpeedScaleVariable = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Units.SpeedScale"));
	const float SpeedScale = SpeedScaleVariable ? FMath::Clamp(SpeedScaleVariable->GetFloat(), 0.1f, 10.f) : 1.f;

	// The world's clock (J7c, bodies kept 30 s): pauses and slows with the game, as the instances' animations do
	// (AnimSequenceTrackTime::GetTime reads World->GetTimeSeconds()).
	const double WorldSeconds = World ? World->GetTimeSeconds() : 0.;

	EntityQuery.ForEachEntityChunk(Context, [Alpha, StepSeconds, Sources, &Sites, &SlotColors, SpeedScale, WorldSeconds](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfos = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		FMassInstancedSkinnedMeshInfoArrayView SkinnedMeshInfos = RepresentationSubsystem->GetMutableInstancedSkinnedMeshInfos();

		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();

		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLPreviousTransformFragment> PreviousTransforms = Context.GetFragmentView<FHOLPreviousTransformFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TArrayView<FMassRepresentationFragment> Representations = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODs = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FMassRepresentationAnimationFragment> Animations = Context.GetMutableFragmentView<FMassRepresentationAnimationFragment>();
		const TArrayView<FHOLUnitAnimationStateFragment> AnimationStates = Context.GetMutableFragmentView<FHOLUnitAnimationStateFragment>();
		const TConstArrayView<FHOLNetUnitDisplayFragment> NetDisplays = Context.GetFragmentView<FHOLNetUnitDisplayFragment>();
		const TConstArrayView<FHOLGatherFragment> Gathers = Context.GetFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FHOLBuildFragment> Builds = Context.GetFragmentView<FHOLBuildFragment>();
		const TConstArrayView<FHOLCombatFragment> Combats = Context.GetFragmentView<FHOLCombatFragment>();
		const FMassEntityManager& EntityManager = Context.GetEntityManagerChecked();
		const int32 StepMs = FMath::RoundToInt(StepSeconds * 1000.f);

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassRepresentationFragment& Representation = Representations[EntityIt];
			const bool bStatic = Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance;
			const bool bSkinned = Representation.CurrentRepresentation == EMassRepresentationType::SkinnedMeshInstance;
			if (!bStatic && !bSkinned)
			{
				continue;
			}

			const FTransform& From = PreviousTransforms[EntityIt].Transform;
			const FTransform& To = Transforms[EntityIt].GetTransform();
			FTransform Drawn(
				FQuat::Slerp(From.GetRotation(), To.GetRotation(), Alpha),
				FMath::Lerp(From.GetLocation(), To.GetLocation(), Alpha),
				To.GetScale3D() * Constants.MeshScale);

			const FMassEntityHandle Entity = Context.GetEntity(EntityIt);
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODs[EntityIt];
			// PrevTransform is last frame's drawn transform (motion vectors), not the previous fixed step.
			if (bStatic)
			{
				const int32 ISMInfoIndex = Representation.StaticMeshDescHandle.ToIndex();
				if (ensureMsgf(ISMInfos.IsValidIndex(ISMInfoIndex), TEXT("Invalid handle index %u for ISMInfosView"), ISMInfoIndex))
				{
					UMassUpdateISMProcessor::UpdateISMTransform(Entity, ISMInfos[ISMInfoIndex], Drawn, Representation.PrevTransform,
						RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
				}
			}
			else
			{
				const int32 SkinnedMeshInfoIndex = Representation.SkinnedMeshDescHandle.ToIndex();
				if (ensureMsgf(SkinnedMeshInfos.IsValidIndex(SkinnedMeshInfoIndex), TEXT("Invalid handle index %u for SkinnedMeshInfosView"), SkinnedMeshInfoIndex))
				{
					FMassInstancedSkinnedMeshInfo& MeshInfo = SkinnedMeshInfos[SkinnedMeshInfoIndex];

					// What it does as this machine knows it (D272): the simulation in solo, the last state received on a client.
					// The speed the client draws (D234): the step's displacement over its duration, or the received velocity
					// for a client's display entity (D271).
					const FHOLUnitStateFragment& State = States[EntityIt];
					const FHOLNetUnitDisplayFragment* NetDisplay = !NetDisplays.IsEmpty() ? &NetDisplays[EntityIt] : nullptr;
					const FHOLGatherFragment* Gather = !NetDisplay && !Gathers.IsEmpty() ? &Gathers[EntityIt] : nullptr;
					const FHOLBuildFragment* Build = !NetDisplay && !Builds.IsEmpty() ? &Builds[EntityIt] : nullptr;
					const FHOLCombatFragment* Combat = !NetDisplay && !Combats.IsEmpty() ? &Combats[EntityIt] : nullptr;
					FHOLUnitAnimationStateFragment& AnimationState = AnimationStates[EntityIt];
					FAnimSequenceTrackAutoPlayData& AnimData = Animations[EntityIt].AnimData;
					FHOLUnitAnimationInput Input;
					Input.Activity = GetShownActivity(State, Combat, NetDisplay != nullptr);
					Input.GatherResource = State.GatherResource;
					Input.bCarrying = NetDisplay ? NetDisplay->State.bCarrying : Gather && Gather->Load.Amount > 0;
					Input.LoadResource = NetDisplay ? NetDisplay->State.LoadResource : Gather ? Gather->Load.Resource : EHOLResource::Food;
					Input.Speed = NetDisplay ? static_cast<float>(NetDisplay->Velocity.Size2D())
						: StepSeconds > 0.f ? FVector::Dist2D(From.GetLocation(), To.GetLocation()) / StepSeconds : 0.f;
					Input.PlayingSequence = AnimationState.bStarted ? AnimData.SequenceIndex : INDEX_NONE;
					// Its feet keep pace with a walk slower than its own (J7a.4: a group in formation at its slowest unit's speed).
					Input.WalkSpeed = Constants.Speed * SpeedScale;
					const FHOLUnitAnimationChoice Choice = HOLUnitAnimation::Choose(Constants, Input);
					if (!AnimationState.bStarted || AnimData.SequenceIndex != Choice.Sequence)
					{
						// Mass hands the data to the instance only when the sequence changes, so the play rate is set here.
						AnimData.SequenceIndex = Choice.Sequence;
						AnimData.PlayRate = Choice.PlayRate;
						const float Length = Constants.SequenceLengths.IsValidIndex(Choice.Sequence) ? Constants.SequenceLengths[Choice.Sequence] : 0.f;
						if (Choice.bStrike)
						{
							// Its hit on the server's (J7b.5): in solo from its cool-down, plus the drawing's lag behind the last step
							// (it draws between the last two steps); on a client, which does not receive the cool-down, a shooter
							// from its replayed cadence (UHOLNetUnitDisplayProcessor, its release on its drawn arrow, a cool-down left
							// from a hit and run included), else as a strike that just started (the type's wind-up,
							// HOLCombat::StartAttack, less the step that brought the state). Its hit at the type's share of the loop.
							const float SecondsToHit = Combat
								? HOLUnitAnimation::GetSecondsToHit(Combat->CooldownMs, StepMs) + (1.f - Alpha) * StepSeconds
								: NetDisplay && NetDisplay->ShotStep >= 0 ? NetDisplay->ShotSecondsToHit
								: HOLUnitAnimation::GetSecondsToHit(HOLCombat::GetWindUpMilliseconds(HOLCombat::ToMilliseconds(Constants.AttackInterval),
									Constants.AttackWindUpPercent) - StepMs, StepMs);
							AnimData.Position = HOLUnitAnimation::GetStrikeStartPosition(Length, Choice.PlayRate, SecondsToHit,
								HOLUnitAnimation::GetStrikeHitShare(Constants));
						}
						else
						{
							// The fall from its start (J7b.5); the loops from a random point (D234).
							AnimData.Position = Choice.bOnce ? 0.f : GetStartOffset(State.UnitId, Choice.Sequence, Length);
						}
						AnimData.BlendTime = AnimationState.bStarted ? AnimationBlendSeconds : 0.f;
						AnimData.LoopMode = Choice.bOnce ? EAnimSequenceTrackLoopMode::Clamp : EAnimSequenceTrackLoopMode::Loop;
						AnimationState.bStarted = true;

						// A gatherer turns towards its source once, when its gesture starts (it stands there), at once (D304); a
						// builder towards its site (J6a.4), the same way.
						FVector SourceLocation;
						AnimationState.bFacingSource = ((Choice.bFacesSource
								&& FindGatheredSource(Sources, Gather, To.GetLocation(), State.GatherResource, SourceLocation))
							|| (Choice.bFacesSite && FindBuiltSite(Sites, Build, To.GetLocation(), Constants.Radius, SourceLocation)))
							&& FVector::DistSquared2D(SourceLocation, To.GetLocation()) > 1.;
						if (AnimationState.bFacingSource)
						{
							const FVector ToSource = SourceLocation - To.GetLocation();
							AnimationState.FacingYaw = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(ToSource.Y, ToSource.X)));
						}
					}
					// Striking in solo (J7b.5): turned towards its target, as a client draws it (UHOLNetUnitDisplayProcessor).
					if (Choice.bStrike && Combat && EntityManager.IsEntityValid(Combat->Target))
					{
						if (const FTransformFragment* TargetTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Combat->Target))
						{
							const FVector ToTarget = TargetTransform->GetTransform().GetLocation() - To.GetLocation();
							if (ToTarget.SizeSquared2D() > 1.)
							{
								AnimationState.bFacingSource = true;
								AnimationState.FacingYaw = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(ToTarget.Y, ToTarget.X)));
							}
						}
					}
					// D345.2b: striking a building, turned to the nearest point of its body.
					else if (Choice.bStrike && Combat && !Combat->TargetBuilding.IsExplicitlyNull())
					{
						if (const AHOLBuilding* Struck = Combat->TargetBuilding.Get())
						{
							const FBox2D Box = Struck->GetBodyBounds();
							const FVector Here = To.GetLocation();
							const FVector2D Wall(FMath::Clamp(Here.X, Box.Min.X, Box.Max.X), FMath::Clamp(Here.Y, Box.Min.Y, Box.Max.Y));
							if (FVector2D::DistSquared(Wall, FVector2D(Here)) > 1.)
							{
								AnimationState.bFacingSource = true;
								AnimationState.FacingYaw = static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(Wall.Y - Here.Y, Wall.X - Here.X)));
							}
						}
					}
					// Falling (J7b code review, point 4): it keeps, for its whole fall, the yaw it was last drawn at alive, turned
					// towards its target if it was striking, rather than going back to the simulation's (its place's front), as a
					// client keeps its Display.Yaw (UHOLNetUnitDisplayProcessor): no half-turn, and the views agree.
					if (Input.Activity == EHOLUnitActivity::Dying)
					{
						if (!AnimationState.bFalling)
						{
							AnimationState.bFalling = true;
							AnimationState.FallStartSeconds = WorldSeconds;
							AnimationState.FallYaw = static_cast<float>((AnimationState.DrawnFrame != 0 ? Representation.PrevTransform : Drawn).Rotator().Yaw);
						}
						AnimationState.bFacingSource = true;
						AnimationState.FacingYaw = AnimationState.FallYaw;
					}
					if (AnimationState.bFacingSource)
					{
						Drawn.SetRotation(FRotator(0.f, AnimationState.FacingYaw, 0.f).Quaternion());
					}
					if (AnimationState.bFalling)
					{
						AnimationState.FallTurnDegrees = FMath::Max(AnimationState.FallTurnDegrees,
							FMath::Abs(FRotator::NormalizeAxis(static_cast<float>(Drawn.Rotator().Yaw) - AnimationState.FallYaw)));
					}
					AnimationState.Prop = Choice.Prop;
					AnimationState.DrawnFrame = GFrameCounter;

					// Transform, animation and colour (and prop) in this order for each entity: Mass pairs them by position.
					UMassUpdateInstancedSkinnedMeshProcessor::UpdateMeshTransform(Entity, MeshInfo, Drawn, Representation.PrevTransform,
						RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					MeshInfo.AddBatchedAnimationData(Entity, AnimData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					MeshInfo.AddBatchedCustomData(GetCustomData(SlotColors.Get(State.OwnerPlayer), Choice.Prop), RepresentationLOD.LODSignificance,
						Representation.PrevLODSignificance);
				}
			}

			Representation.PrevTransform = Drawn;
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});

	// The bodies (J7c, bodies kept 30 s): each ends its unit's fall, holds its last pose, then sinks (HOLCorpse).
	CorpseQuery.ForEachEntityChunk(Context, [&SlotColors, WorldSeconds](FMassExecutionContext& Context)
	{
		UMassRepresentationSubsystem* RepresentationSubsystem = Context.GetSharedFragment<FMassRepresentationSubsystemSharedFragment>().RepresentationSubsystem;
		check(RepresentationSubsystem);
		FMassInstancedStaticMeshInfoArrayView ISMInfos = RepresentationSubsystem->GetMutableInstancedStaticMeshInfos();
		FMassInstancedSkinnedMeshInfoArrayView SkinnedMeshInfos = RepresentationSubsystem->GetMutableInstancedSkinnedMeshInfos();

		const TConstArrayView<FHOLCorpseFragment> Corpses = Context.GetFragmentView<FHOLCorpseFragment>();
		const TArrayView<FMassRepresentationFragment> Representations = Context.GetMutableFragmentView<FMassRepresentationFragment>();
		const TConstArrayView<FMassRepresentationLODFragment> RepresentationLODs = Context.GetFragmentView<FMassRepresentationLODFragment>();
		const TArrayView<FMassRepresentationAnimationFragment> Animations = Context.GetMutableFragmentView<FMassRepresentationAnimationFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FMassRepresentationFragment& Representation = Representations[EntityIt];
			const bool bStatic = Representation.CurrentRepresentation == EMassRepresentationType::StaticMeshInstance;
			const bool bSkinned = Representation.CurrentRepresentation == EMassRepresentationType::SkinnedMeshInstance;
			if (!bStatic && !bSkinned)
			{
				continue;
			}

			const FHOLCorpseFragment& Corpse = Corpses[EntityIt];
			const double Age = WorldSeconds - Corpse.LaidAt;
			FTransform Drawn = Corpse.Base;
			Drawn.AddToTranslation(FVector(0., 0., -HOLCorpse::GetSinkOffset(Age)));

			const FMassEntityHandle Entity = Context.GetEntity(EntityIt);
			const FMassRepresentationLODFragment& RepresentationLOD = RepresentationLODs[EntityIt];
			if (bStatic)
			{
				const int32 ISMInfoIndex = Representation.StaticMeshDescHandle.ToIndex();
				if (ensureMsgf(ISMInfos.IsValidIndex(ISMInfoIndex), TEXT("Invalid handle index %u for ISMInfosView"), ISMInfoIndex))
				{
					UMassUpdateISMProcessor::UpdateISMTransform(Entity, ISMInfos[ISMInfoIndex], Drawn, Representation.PrevTransform,
						RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
				}
			}
			else
			{
				const int32 SkinnedMeshInfoIndex = Representation.SkinnedMeshDescHandle.ToIndex();
				if (ensureMsgf(SkinnedMeshInfos.IsValidIndex(SkinnedMeshInfoIndex), TEXT("Invalid handle index %u for SkinnedMeshInfosView"), SkinnedMeshInfoIndex))
				{
					FMassInstancedSkinnedMeshInfo& MeshInfo = SkinnedMeshInfos[SkinnedMeshInfoIndex];

					// Its fall where it would be now, at every frame: Mass hands the data to an instance only when its track is
					// allocated (the body appears, or comes back into view) or its sequence changes, and the track's reference
					// time is then set from this position and the world's clock (AnimSequenceTransformProviderData.h,
					// ReferenceTimestamp); the rest of the time it is ignored. So a body that (re)appears resumes its fall where
					// it should be, or holds its last frame (Clamp).
					FAnimSequenceTrackAutoPlayData& AnimData = Animations[EntityIt].AnimData;
					AnimData.SequenceIndex = Corpse.Sequence;
					AnimData.PlayRate = Corpse.PlayRate;
					AnimData.Position = HOLCorpse::GetResumePosition(Corpse.StartPosition, Age, Corpse.PlayRate, Corpse.Length);
					AnimData.BlendTime = 0.f;
					AnimData.LoopMode = EAnimSequenceTrackLoopMode::Clamp;

					// Transform, animation and colour (and prop) in this order for each entity: Mass pairs them by position.
					UMassUpdateInstancedSkinnedMeshProcessor::UpdateMeshTransform(Entity, MeshInfo, Drawn, Representation.PrevTransform,
						RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					MeshInfo.AddBatchedAnimationData(Entity, AnimData, RepresentationLOD.LODSignificance, Representation.PrevLODSignificance);
					MeshInfo.AddBatchedCustomData(GetCustomData(SlotColors.Get(Corpse.OwnerPlayer), Corpse.Prop), RepresentationLOD.LODSignificance,
						Representation.PrevLODSignificance);
				}
			}

			Representation.PrevTransform = Drawn;
			Representation.PrevLODSignificance = RepresentationLOD.LODSignificance;
		}
	});

	// Expired bodies go (J7c, bodies kept 30 s), seen or not: destroyed once the processors have run (Mass removes their
	// instances, as for a unit).
	TArray<FMassEntityHandle> ExpiredCorpses;
	CorpseExpiryQuery.ForEachEntityChunk(Context, [World, WorldSeconds, &ExpiredCorpses](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHOLCorpseFragment> Corpses = Context.GetFragmentView<FHOLCorpseFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const double Age = WorldSeconds - Corpses[EntityIt].LaidAt;
			if (HOLCorpse::IsExpired(Age))
			{
				ExpiredCorpses.Add(Context.GetEntity(EntityIt));
				// Counted for hol.Units.CorpseReport: how many went, and how old.
				if (World)
				{
					HOLCorpse::NoteRemoved(*World, Age);
				}
			}
		}
	});
	if (!ExpiredCorpses.IsEmpty())
	{
		Context.Defer().DestroyEntities(MoveTemp(ExpiredCorpses));
	}
}

namespace
{
	/**
	 * Scripted check of J5c.4 (quick_check's "anims" part): what this frame's drawing gave the units of a slot, the sequence
	 * playing and the prop shown, counted per sequence, per prop, and per state the choice was made from (activity, what it
	 * gathers, what it carries). Solo only: a client's units are display entities with no unit identifier to look them up.
	 * Reads what the presentation wrote (FHOLUnitAnimationStateFragment, Mass' animation fragment), nothing recomputed.
	 */
	FAutoConsoleCommand AnimReportCommand(
		TEXT("hol.Units.AnimReport"),
		TEXT("hol.Units.AnimReport [Slot]: logs, for the units of a slot (the local player's by default) drawn now, the sequences played and the props shown, per state (solo)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				const UWorld* World = WorldContext.World();
				const UHOLUnitSubsystem* Units = World && World->IsGameWorld() && World->GetNetMode() == NM_Standalone
					? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
				if (!Units || !EntitySubsystem)
				{
					continue;
				}
				int32 Slot = INDEX_NONE;
				if (!Args.IsEmpty())
				{
					Slot = FCString::Atoi(*Args[0]);
				}
				else if (const APlayerController* Controller = World->GetFirstPlayerController())
				{
					const AHOLPlayerState* PlayerState = Controller->GetPlayerState<AHOLPlayerState>();
					Slot = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
				}

				// The sequence's asset name in the type's animation provider (its index if the provider is not loaded).
				auto GetSequenceName = [](const FHOLUnitTypeConstants& Type, const int32 Sequence)
				{
					const UAnimSequenceTransformProviderData* Provider = Cast<UAnimSequenceTransformProviderData>(Type.AnimationProvider.ResolveObject());
					if (Provider && Provider->GetSequences().IsValidIndex(Sequence) && Provider->GetSequences()[Sequence].Sequence)
					{
						return Provider->GetSequences()[Sequence].Sequence->GetName();
					}
					return FString::Printf(TEXT("sequence %d"), Sequence);
				};
				auto GetEnumName = [](const UEnum* Enum, const uint8 Value) { return Enum->GetNameStringByValue(static_cast<int64>(Value)); };

				const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
				TArray<uint32> UnitIds;
				Units->GetAllUnitIds(UnitIds);
				UnitIds.Sort();
				TMap<FString, int32> Sequences;
				TMap<int32, int32> Props;
				TMap<FString, TMap<FString, int32>> ByState;
				int32 NumDrawn = 0;
				int32 NumNotDrawn = 0;
				int32 NumFalling = 0;
				float LargestFallTurn = 0.f;
				for (const uint32 UnitId : UnitIds)
				{
					const FMassEntityHandle Entity = Units->FindEntity(UnitId);
					const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
					const FHOLUnitAnimationStateFragment* AnimationState = EntityManager.GetFragmentDataPtr<FHOLUnitAnimationStateFragment>(Entity);
					const FMassRepresentationAnimationFragment* Animation = EntityManager.GetFragmentDataPtr<FMassRepresentationAnimationFragment>(Entity);
					const FHOLUnitTypeConstants* Type = EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity);
					if (!State || !AnimationState || !Animation || !Type || (Slot != INDEX_NONE && State->OwnerPlayer != Slot))
					{
						continue;
					}
					// Drawn by this frame's or the last one's presentation (the command may run before or after it).
					if (!AnimationState->bStarted || AnimationState->DrawnFrame + 2 < GFrameCounter)
					{
						++NumNotDrawn;
						continue;
					}
					++NumDrawn;
					if (AnimationState->bFalling)
					{
						++NumFalling;
						LargestFallTurn = FMath::Max(LargestFallTurn, AnimationState->FallTurnDegrees);
					}
					const FString Sequence = GetSequenceName(*Type, Animation->AnimData.SequenceIndex);
					++Sequences.FindOrAdd(Sequence);
					++Props.FindOrAdd(AnimationState->Prop);

					// The state as the presentation read it: the simulation's (solo), Attacking and Dying as the clients have them.
					const EHOLUnitActivity Activity = UE::HeroesOfLiberty::Presentation::GetShownActivity(*State, EntityManager.GetFragmentDataPtr<FHOLCombatFragment>(Entity), false);
					FString StateName = Type->TypeName.ToString() + TEXT(" ") + GetEnumName(StaticEnum<EHOLUnitActivity>(), static_cast<uint8>(Activity));
					if (State->Activity == EHOLUnitActivity::Gathering)
					{
						StateName += TEXT(" ") + GetEnumName(StaticEnum<EHOLResource>(), static_cast<uint8>(State->GatherResource));
					}
					const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
					if (Gather && Gather->Load.Amount > 0)
					{
						StateName += TEXT(" carrying ") + GetEnumName(StaticEnum<EHOLResource>(), static_cast<uint8>(Gather->Load.Resource));
					}
					++ByState.FindOrAdd(StateName).FindOrAdd(FString::Printf(TEXT("%s prop %d"), *Sequence, AnimationState->Prop));
				}

				Sequences.KeySort(TLess<FString>());
				Props.KeySort(TLess<int32>());
				ByState.KeySort(TLess<FString>());
				TArray<FString> SequenceParts;
				for (const TPair<FString, int32>& Pair : Sequences)
				{
					SequenceParts.Add(FString::Printf(TEXT("%s x%d"), *Pair.Key, Pair.Value));
				}
				TArray<FString> PropParts;
				for (const TPair<int32, int32>& Pair : Props)
				{
					PropParts.Add(FString::Printf(TEXT("%d x%d"), Pair.Key, Pair.Value));
				}
				TArray<FString> StateParts;
				for (TPair<FString, TMap<FString, int32>>& Pair : ByState)
				{
					Pair.Value.KeySort(TLess<FString>());
					TArray<FString> Shown;
					for (const TPair<FString, int32>& Outcome : Pair.Value)
					{
						Shown.Add(FString::Printf(TEXT("%s x%d"), *Outcome.Key, Outcome.Value));
					}
					StateParts.Add(Pair.Key + TEXT(": ") + FString::Join(Shown, TEXT(", ")));
				}
				UE_LOG(LogHOLPresentation, Display, TEXT("Anim report: slot %d, %d drawn, %d not drawn; sequences %s; props %s"), Slot, NumDrawn, NumNotDrawn,
					*FString::Join(SequenceParts, TEXT(", ")), *FString::Join(PropParts, TEXT(", ")));
				UE_LOG(LogHOLPresentation, Display, TEXT("Anim report by state: %s"), *FString::Join(StateParts, TEXT("; ")));
				UE_LOG(LogHOLPresentation, Display, TEXT("Anim report falls: slot %d, %d falling, largest turn %.1f deg from the yaw drawn when it died"),
					Slot, NumFalling, LargestFallTurn);
			}
		}));

	/**
	 * Scripted check of the bodies (J7c, bodies kept 30 s; quick_check's "combat" and "net" parts): the bodies this machine
	 * keeps now, per type, those sinking, those whose fall has ended, the oldest; and those that went since the world began
	 * (HOLCorpse::GetRemovedStats). Every game world, a dedicated server's included (it lays none: 0 bodies).
	 */
	FAutoConsoleCommand CorpseReportCommand(
		TEXT("hol.Units.CorpseReport"),
		TEXT("hol.Units.CorpseReport: logs the bodies kept now (per type, sinking, fall ended, the oldest) and those that went (how many, the longest kept)"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				UMassEntitySubsystem* EntitySubsystem = World && World->IsGameWorld() ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
				if (!EntitySubsystem)
				{
					continue;
				}
				const double WorldSeconds = World->GetTimeSeconds();
				FMassEntityManager& EntityManager = EntitySubsystem->GetMutableEntityManager();
				FMassEntityQuery Query(EntityManager.AsShared());
				Query.AddTagRequirement<FHOLCorpseTag>(EMassFragmentPresence::All);
				Query.AddRequirement<FHOLCorpseFragment>(EMassFragmentAccess::ReadOnly);
				TMap<FName, int32> ByType;
				int32 NumCorpses = 0;
				int32 NumSinking = 0;
				int32 NumFallEnded = 0;
				double Oldest = 0.;
				FMassExecutionContext ExecutionContext(EntityManager);
				Query.ForEachEntityChunk(ExecutionContext, [&](FMassExecutionContext& Context)
				{
					const TConstArrayView<FHOLCorpseFragment> Corpses = Context.GetFragmentView<FHOLCorpseFragment>();
					for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
					{
						const FHOLCorpseFragment& Corpse = Corpses[EntityIt];
						const double Age = WorldSeconds - Corpse.LaidAt;
						++NumCorpses;
						++ByType.FindOrAdd(Corpse.TypeName);
						NumSinking += HOLCorpse::GetSinkOffset(Age) > 0.f ? 1 : 0;
						NumFallEnded += HOLCorpse::GetResumePosition(Corpse.StartPosition, Age, Corpse.PlayRate, Corpse.Length) >= Corpse.Length ? 1 : 0;
						Oldest = FMath::Max(Oldest, Age);
					}
				});

				ByType.KeySort(FNameLexicalLess());
				TArray<FString> TypeParts;
				for (const TPair<FName, int32>& Pair : ByType)
				{
					TypeParts.Add(FString::Printf(TEXT("%s x%d"), *Pair.Key.ToString(), Pair.Value));
				}
				const HOLCorpse::FRemovedStats Removed = HOLCorpse::GetRemovedStats(*World);
				UE_LOG(LogHOLPresentation, Display, TEXT("Corpse report: %d corpses (%s), %d sinking, %d fall ended, oldest %.1f s; %d removed, longest kept %.1f s"),
					NumCorpses, *FString::Join(TypeParts, TEXT(", ")), NumSinking, NumFallEnded, Oldest, Removed.NumRemoved, Removed.LongestKept);
			}
		}));
}
