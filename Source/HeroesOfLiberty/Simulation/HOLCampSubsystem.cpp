// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCampSubsystem.h"

#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLHeroXPRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

namespace
{
	/** A camp thinks every fourth step (5 times a second at 20 Hz), as the combat's search. */
	int32 CampThinkSteps = 4;
	/** The attack-move of a fighting camp is given again this often (steps). */
	int32 CampOrderSteps = 20;
	/** The units of the Neutral owner this near a camp's post at its first thought are its members (cm). */
	float CampAdoptRadius = 1200.f;
	int32 CampAggro = 0;
	int32 CampCredit = 2;
	float CampAggroRadius = 1500.f;
	float CampLeashRadius = 3000.f;
	float CampArriveRadius = 400.f;
	int32 CampRewardXP = 100;

	FAutoConsoleVariableRef CVarCampAggro(TEXT("hol.Camp.Aggro"), CampAggro, TEXT("NQ1, neutral camps: 0 = charge what comes within hol.Camp.AggroRadius of the post, 1 = only when attacked"));
	FAutoConsoleVariableRef CVarCampCredit(TEXT("hol.Camp.Credit"), CampCredit, TEXT("NQ2, XP of a cleared camp: 0 = the last blow, 1 = the most damage, 2 = shared by the damage"));
	FAutoConsoleVariableRef CVarCampAggroRadius(TEXT("hol.Camp.AggroRadius"), CampAggroRadius, TEXT("Distance from its post at which an adversary draws a neutral camp (cm)"));
	FAutoConsoleVariableRef CVarCampLeashRadius(TEXT("hol.Camp.LeashRadius"), CampLeashRadius, TEXT("A neutral camp gives up the chase beyond this from its post (cm)"));
	FAutoConsoleVariableRef CVarCampArriveRadius(TEXT("hol.Camp.ArriveRadius"), CampArriveRadius, TEXT("A neutral camp is back at its post when all its members are within this of it (cm)"));
	FAutoConsoleVariableRef CVarCampRewardXP(TEXT("hol.Camp.RewardXP"), CampRewardXP, TEXT("XP of a cleared neutral camp, shared out by the credit rule"));

	/** Statics: the combat processor asks and tells without a world lookup. Only the game thread of the simulation writes them. */
	TSet<uint32> PassiveUnits;
	struct FHitNote
	{
		uint32 VictimId = 0;
		int32 AttackerOwner = 0;
		int32 Damage = 0;
	};
	TArray<FHitNote> HitNotes;
}

bool HOLCamps::IsPassive(const uint32 UnitId)
{
	return PassiveUnits.Contains(UnitId);
}

void HOLCamps::NoteHit(const uint32 VictimId, const int32 AttackerOwner, const float Damage)
{
	if (AttackerOwner != FHOLMatchConfig::NeutralOwner)
	{
		HitNotes.Add(FHitNote{ VictimId, AttackerOwner, FMath::Max(1, FMath::RoundToInt(Damage)) });
	}
}

HOLCampRules::FSettings UHOLCampSubsystem::MakeSettings()
{
	HOLCampRules::FSettings Settings;
	Settings.Aggro = CampAggro == 1 ? HOLCampRules::EAggro::OnAttack : HOLCampRules::EAggro::OnSight;
	Settings.Credit = CampCredit == 0 ? HOLCampRules::ECredit::LastBlow : CampCredit == 1 ? HOLCampRules::ECredit::MostDamage : HOLCampRules::ECredit::Shared;
	Settings.AggroRadius = CampAggroRadius;
	Settings.LeashRadius = CampLeashRadius;
	Settings.ArriveRadius = CampArriveRadius;
	Settings.RewardXP = CampRewardXP;
	return Settings;
}

bool UHOLCampSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLCampSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	PassiveUnits.Reset();
	HitNotes.Reset();
	// The server (or a solo match) decides alone; a client's camps stay asleep.
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	if (UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepEndedHandle = Simulation->GetOnStepEnded().AddUObject(this, &UHOLCampSubsystem::OnStepEnded);
	}
}

void UHOLCampSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepEnded().Remove(StepEndedHandle);
		}
	}
	StepEndedHandle.Reset();
	PassiveUnits.Reset();
	HitNotes.Reset();
	Super::Deinitialize();
}

int32 UHOLCampSubsystem::AddCamp(const FVector& Post, const FName TypeName, const int32 Count)
{
	UHOLUnitSubsystem* Units = GetWorld() ? GetWorld()->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Units || Count <= 0)
	{
		return INDEX_NONE;
	}
	FHOLUnitSpawnRequest Request;
	Request.TypeName = TypeName;
	Request.Count = Count;
	Request.Center = Post;
	Request.Radius = 300.f;
	Request.OwnerPlayer = static_cast<uint8>(FHOLMatchConfig::NeutralOwner);
	Units->RequestUnits(Request);
	FCamp& Camp = Camps.AddDefaulted_GetRef();
	Camp.Post = Post;
	Camp.Size = Count;
	const int32 Id = Camps.Num() - 1;
	UE_LOG(LogHOLMatch, Display, TEXT("Camp %d: %d %s at (%.0f, %.0f), guarding"), Id, Count, *TypeName.ToString(), Post.X, Post.Y);
	return Id;
}

void UHOLCampSubsystem::OnStepEnded(const int64 StepIndex)
{
	if (Camps.IsEmpty())
	{
		HitNotes.Reset();
		return;
	}
	// The hits struck between two thoughts are kept for the next one (they were dropped: a camp cleared "with 0 hits taken" gave no reward, seen on `neutrals`).
	if (StepIndex % FMath::Max(1, CampThinkSteps) != 0)
	{
		return;
	}
	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	if (!Units || !Commands)
	{
		return;
	}
	TArray<FHOLUnitView> Views;
	Units->GetUnitViews(Views);
	TMap<uint32, int32> ViewOfUnit;
	for (int32 Index = 0; Index < Views.Num(); ++Index)
	{
		ViewOfUnit.Add(Views[Index].UnitId, Index);
	}

	// What hit the members since the last thought.
	for (const FHitNote& Note : HitNotes)
	{
		for (FCamp& Camp : Camps)
		{
			if (Camp.MemberIds.Contains(Note.VictimId))
			{
				Camp.Hits.Add(HOLCampRules::FHit{ Note.AttackerOwner, Note.Damage });
				Camp.bAttacked = true;
			}
		}
	}
	HitNotes.Reset();

	const HOLCampRules::FSettings Settings = MakeSettings();
	for (int32 CampId = 0; CampId < Camps.Num(); ++CampId)
	{
		FCamp& Camp = Camps[CampId];
		if (Camp.State == HOLCampRules::EState::Cleared)
		{
			continue;
		}
		const FVector2D Post(Camp.Post);

		// Born at the step after the request: the neutral units near the post, not in a camp yet, are its members.
		if (!Camp.bAdopted)
		{
			for (const FHOLUnitView& View : Views)
			{
				bool bTaken = false;
				for (const FCamp& Other : Camps)
				{
					bTaken = bTaken || Other.MemberIds.Contains(View.UnitId);
				}
				if (View.OwnerPlayer == FHOLMatchConfig::NeutralOwner && View.Health > 0 && !bTaken
					&& FVector2D::Distance(FVector2D(View.Location), Post) <= CampAdoptRadius && Camp.MemberIds.Num() < Camp.Size)
				{
					Camp.MemberIds.Add(View.UnitId);
				}
			}
			Camp.bAdopted = Camp.MemberIds.Num() >= Camp.Size;
			if (Camp.MemberIds.IsEmpty())
			{
				continue;
			}
		}

		TArray<HOLCampRules::FMember> Members;
		for (const uint32 Id : Camp.MemberIds)
		{
			const int32* Index = ViewOfUnit.Find(Id);
			if (Index && Views[*Index].Health > 0)
			{
				Members.Add(HOLCampRules::FMember{ Id, FVector2D(Views[*Index].Location) });
			}
		}
		TArray<HOLCampRules::FIntruder> Intruders;
		const HOLCampRules::FIntruder* Nearest = nullptr;
		for (const FHOLUnitView& View : Views)
		{
			if (View.OwnerPlayer != FHOLMatchConfig::NeutralOwner && View.Health > 0
				&& FVector2D::Distance(FVector2D(View.Location), Post) <= Settings.LeashRadius)
			{
				Intruders.Add(HOLCampRules::FIntruder{ View.UnitId, View.OwnerPlayer, FVector2D(View.Location) });
			}
		}
		for (const HOLCampRules::FIntruder& Intruder : Intruders)
		{
			if (!Nearest || FVector2D::Distance(Intruder.Location, Post) < FVector2D::Distance(Nearest->Location, Post))
			{
				Nearest = &Intruder;
			}
		}

		const HOLCampRules::EState Before = Camp.State;
		const HOLCampRules::FStep Step = HOLCampRules::Step(Camp.State, Settings, Post, Members, Intruders, Camp.bAttacked);
		Camp.bAttacked = false;
		Camp.Members = Members.Num();
		if (Step.bChanged)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Camp %d: %s (members %d, adversaries near %d)"), CampId, HOLCampRules::GetStateName(Camp.State), Members.Num(), Intruders.Num());
		}

		// What a member does: passive at the post and on the way back, free to fight in a fight (but sent back when it chases past the leash).
		TArray<uint32> Fighting;
		TArray<uint32> Going;
		for (const HOLCampRules::FMember& Member : Members)
		{
			const bool bGoesBack = Step.ReturnIds.Contains(Member.Id);
			if (Camp.State == HOLCampRules::EState::Fighting && !bGoesBack)
			{
				PassiveUnits.Remove(Member.Id);
				Fighting.Add(Member.Id);
			}
			else
			{
				PassiveUnits.Add(Member.Id);
				if (bGoesBack || (Camp.State == HOLCampRules::EState::Returning && Before != HOLCampRules::EState::Returning))
				{
					Going.Add(Member.Id);
				}
			}
		}
		if (!Going.IsEmpty())
		{
			Commands->IssueMoveOrder(Going, Camp.Post);
		}
		if (!Fighting.IsEmpty() && Nearest && (Before != HOLCampRules::EState::Fighting || StepIndex % FMath::Max(1, CampOrderSteps) == 0))
		{
			Commands->IssueAttackMoveOrder(Fighting, FVector(Nearest->Location, Camp.Post.Z), INDEX_NONE, false);
		}

		if (Step.bJustCleared)
		{
			UHOLHeroSubsystem* Heroes = World->GetSubsystem<UHOLHeroSubsystem>();
			UE_LOG(LogHOLMatch, Display, TEXT("Camp %d: cleared for good (%d hits taken)"), CampId, Camp.Hits.Num());
			for (const HOLCampRules::FShare& Share : HOLCampRules::Rewards(Settings, Camp.Hits))
			{
				if (Share.Owner >= 0 && Share.Owner < FHOLMatchConfig::MaxSlots && Share.XP > 0)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Camp %d reward: slot %d +%d XP"), CampId, Share.Owner, Share.XP);
					if (Heroes)
					{
						Heroes->AddXP(Share.Owner, Share.XP, EHOLXPSource::Exploration);
					}
				}
			}
		}
	}
	WriteViews();
}

void UHOLCampSubsystem::WriteViews()
{
	UWorld* World = GetWorld();
	const AHOLGameState* GameState = World && World->GetNetMode() != NM_Client ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		return;
	}
	const UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	const bool bFog = Vision && Vision->IsStarted();
	Known.SetNum(FHOLMatchConfig::MaxSlots);
	for (APlayerState* Each : GameState->PlayerArray)
	{
		AHOLPlayerState* Player = Cast<AHOLPlayerState>(Each);
		const int32 Slot = Player ? Player->GetPlayerIndex() : INDEX_NONE;
		if (Slot < 0 || Slot >= FHOLMatchConfig::MaxSlots)
		{
			continue;
		}
		TArray<FKnownCamp>& Mine = Known[Slot];
		Mine.SetNum(Camps.Num());
		TArray<FHOLCampView> Views;
		for (int32 CampId = 0; CampId < Camps.Num(); ++CampId)
		{
			const FCamp& Camp = Camps[CampId];
			// Q1 C: in sight of his camp (or without a fog), known and read live; out of sight, as last seen; never seen, absent.
			if (!bFog || Vision->GetVision(Slot, FVector(Camp.Post.X, Camp.Post.Y, 0.)) == EHOLCellVision::Visible)
			{
				FHOLCampView& View = Mine[CampId].View;
				View.Index = static_cast<uint8>(FMath::Clamp(CampId, 0, 255));
				View.X = static_cast<float>(Camp.Post.X);
				View.Y = static_cast<float>(Camp.Post.Y);
				View.bCleared = Camp.State == HOLCampRules::EState::Cleared;
				View.Living = static_cast<uint8>(FMath::Clamp(Camp.Members, 0, 255));
				Mine[CampId].bKnown = true;
			}
			if (Mine[CampId].bKnown)
			{
				Views.Add(Mine[CampId].View);
			}
		}
		Player->SetCampViews(Views);
	}
}

FString UHOLCampSubsystem::MakeReport() const
{
	FString Text = FString::Printf(TEXT("Camp report: %d camps"), Camps.Num());
	for (int32 CampId = 0; CampId < Camps.Num(); ++CampId)
	{
		const FCamp& Camp = Camps[CampId];
		Text += FString::Printf(TEXT("\nCamp %d at (%.0f, %.0f): %s, members %d of %d, hits taken %d"), CampId, Camp.Post.X, Camp.Post.Y,
			HOLCampRules::GetStateName(Camp.State), Camp.Members, Camp.Size, Camp.Hits.Num());
	}
	return Text;
}

namespace
{
	FAutoConsoleCommandWithWorldAndArgs CommandCampAdd(TEXT("hol.Camp.Add"),
		TEXT("hol.Camp.Add X Y [Count=4] [Type=Spearman]: a neutral camp of Count units around the ground point X Y (cm), server or solo; a test command"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UHOLCampSubsystem* Camps = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLCampSubsystem>() : nullptr;
			if (!Camps || Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("hol.Camp.Add X Y [Count=4] [Type=Spearman], on the server or in solo"));
				return;
			}
			Camps->AddCamp(FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.), FName(Args.Num() >= 4 ? *Args[3] : TEXT("Spearman")),
				Args.Num() >= 3 ? FCString::Atoi(*Args[2]) : 4);
		}));

	FAutoConsoleCommandWithWorldAndArgs CommandCampReport(TEXT("hol.Camp.Report"),
		TEXT("hol.Camp.Report: logs each neutral camp: its post, state, living members and hits taken (server or solo)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>&, UWorld* World)
		{
			const UHOLCampSubsystem* Camps = World ? World->GetSubsystem<UHOLCampSubsystem>() : nullptr;
			UE_LOG(LogHOLMatch, Display, TEXT("%s"), Camps ? *Camps->MakeReport() : TEXT("Camp report: no camps here"));
		}));
}
