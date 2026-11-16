// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCampBoard.h"

#include "Engine/World.h"

namespace
{
	HOLCampRules::FSettings ToRules(const FHOLCampBoardSettings& In)
	{
		HOLCampRules::FSettings Out;
		Out.Aggro = static_cast<HOLCampRules::EAggro>(In.Aggro);
		Out.Credit = static_cast<HOLCampRules::ECredit>(In.Credit);
		Out.AggroRadius = In.AggroRadius;
		Out.LeashRadius = In.LeashRadius;
		Out.ArriveRadius = In.ArriveRadius;
		Out.RewardXP = In.RewardXP;
		return Out;
	}

	FHOLCampBoardSettings ToBoard(const HOLCampRules::FSettings& In)
	{
		FHOLCampBoardSettings Out;
		Out.Aggro = static_cast<EHOLCampBoardAggro>(In.Aggro);
		Out.Credit = static_cast<EHOLCampBoardCredit>(In.Credit);
		Out.AggroRadius = In.AggroRadius;
		Out.LeashRadius = In.LeashRadius;
		Out.ArriveRadius = In.ArriveRadius;
		Out.RewardXP = In.RewardXP;
		return Out;
	}
}

UHOLCampBoard* UHOLCampBoard::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLCampBoard*> Boards = World->GetSubsystemArrayCopy<UHOLCampBoard>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

bool UHOLCampBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLGarrisonBoard).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLCampBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

HOLCampRules::FStep UHOLCampBoard::StepForCamp(const UWorld* World, HOLCampRules::EState& InOutState, const HOLCampRules::FSettings& Settings,
	const FVector2D& Post, const TArray<HOLCampRules::FMember>& Members, const TArray<HOLCampRules::FIntruder>& Intruders, const bool bAttacked)
{
	UHOLCampBoard* Board = Find(World);
	if (!Board)
	{
		return HOLCampRules::Step(InOutState, Settings, Post, Members, Intruders, bAttacked);
	}
	TArray<FHOLCampBoardMember> BoardMembers;
	BoardMembers.Reserve(Members.Num());
	for (const HOLCampRules::FMember& Member : Members)
	{
		FHOLCampBoardMember& Out = BoardMembers.AddDefaulted_GetRef();
		Out.Id = static_cast<int32>(Member.Id);
		Out.Location = Member.Location;
	}
	TArray<FHOLCampBoardIntruder> BoardIntruders;
	BoardIntruders.Reserve(Intruders.Num());
	for (const HOLCampRules::FIntruder& Intruder : Intruders)
	{
		FHOLCampBoardIntruder& Out = BoardIntruders.AddDefaulted_GetRef();
		Out.Id = static_cast<int32>(Intruder.Id);
		Out.Owner = Intruder.Owner;
		Out.Location = Intruder.Location;
	}
	const FHOLCampBoardStep Answer = Board->StepNow(static_cast<EHOLCampBoardState>(InOutState), ToBoard(Settings), Post, BoardMembers, BoardIntruders, bAttacked);
	InOutState = static_cast<HOLCampRules::EState>(Answer.State);
	HOLCampRules::FStep Result;
	Result.bChanged = Answer.bChanged;
	Result.bJustCleared = Answer.bJustCleared;
	for (const int32 Id : Answer.ReturnIds)
	{
		Result.ReturnIds.Add(static_cast<uint32>(Id));
	}
	return Result;
}

TArray<HOLCampRules::FShare> UHOLCampBoard::RewardsForCamp(const UWorld* World, const HOLCampRules::FSettings& Settings, const TArray<HOLCampRules::FHit>& Hits)
{
	UHOLCampBoard* Board = Find(World);
	if (!Board)
	{
		return HOLCampRules::Rewards(Settings, Hits);
	}
	TArray<FHOLCampBoardHit> BoardHits;
	BoardHits.Reserve(Hits.Num());
	for (const HOLCampRules::FHit& Hit : Hits)
	{
		BoardHits.Add(FHOLCampBoardHit{ Hit.Owner, Hit.Damage });
	}
	TArray<HOLCampRules::FShare> Shares;
	for (const FHOLCampBoardShare& Share : Board->RewardsNow(ToBoard(Settings), BoardHits))
	{
		Shares.Add(HOLCampRules::FShare{ Share.Owner, Share.XP });
	}
	return Shares;
}

FHOLCampBoardStep UHOLCampBoard::StepNow(const EHOLCampBoardState State, const FHOLCampBoardSettings& Settings, const FVector2D Post,
	const TArray<FHOLCampBoardMember>& Members, const TArray<FHOLCampBoardIntruder>& Intruders, const bool bAttacked)
{
	return CampStep(State, Settings, Post, Members, Intruders, bAttacked);
}

TArray<FHOLCampBoardShare> UHOLCampBoard::RewardsNow(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits)
{
	return CampRewards(Settings, Hits);
}

FHOLCampBoardStep UHOLCampBoard::CampStep_Implementation(const EHOLCampBoardState State, const FHOLCampBoardSettings& Settings, const FVector2D Post,
	const TArray<FHOLCampBoardMember>& Members, const TArray<FHOLCampBoardIntruder>& Intruders, const bool bAttacked)
{
	// The C++ rule, the same thought as the script's.
	TArray<HOLCampRules::FMember> RuleMembers;
	for (const FHOLCampBoardMember& Member : Members)
	{
		RuleMembers.Add(HOLCampRules::FMember{ static_cast<uint32>(Member.Id), Member.Location });
	}
	TArray<HOLCampRules::FIntruder> RuleIntruders;
	for (const FHOLCampBoardIntruder& Intruder : Intruders)
	{
		RuleIntruders.Add(HOLCampRules::FIntruder{ static_cast<uint32>(Intruder.Id), Intruder.Owner, Intruder.Location });
	}
	HOLCampRules::EState RuleState = static_cast<HOLCampRules::EState>(State);
	const HOLCampRules::FStep Step = HOLCampRules::Step(RuleState, ToRules(Settings), Post, RuleMembers, RuleIntruders, bAttacked);
	FHOLCampBoardStep Answer;
	Answer.State = static_cast<EHOLCampBoardState>(RuleState);
	Answer.bChanged = Step.bChanged;
	Answer.bJustCleared = Step.bJustCleared;
	for (const uint32 Id : Step.ReturnIds)
	{
		Answer.ReturnIds.Add(static_cast<int32>(Id));
	}
	return Answer;
}

TArray<FHOLCampBoardShare> UHOLCampBoard::CampRewards_Implementation(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits)
{
	TArray<HOLCampRules::FHit> RuleHits;
	for (const FHOLCampBoardHit& Hit : Hits)
	{
		RuleHits.Add(HOLCampRules::FHit{ Hit.Owner, Hit.Damage });
	}
	TArray<FHOLCampBoardShare> Shares;
	for (const HOLCampRules::FShare& Share : HOLCampRules::Rewards(ToRules(Settings), RuleHits))
	{
		Shares.Add(FHOLCampBoardShare{ Share.Owner, Share.XP });
	}
	return Shares;
}
