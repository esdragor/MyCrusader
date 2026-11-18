// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLCampBoard.h"

#include "Engine/World.h"

namespace
{
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
		// No script board: no C++ rule to fall back on (D836 point 3). The camp does not move: the state is kept, nobody is sent back.
		UE_LOG(LogTemp, Error, TEXT("Camp: no script board, the camp keeps its state"));
		return HOLCampRules::FStep();
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
		// No script board: no reward.
		UE_LOG(LogTemp, Error, TEXT("Camp: no script board, no reward"));
		return TArray<HOLCampRules::FShare>();
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
	// Inert and safe: only the script board knows the rule. The camp does not move (state kept, nothing changed, nobody sent back).
	UE_LOG(LogTemp, Error, TEXT("Camp: no script override of CampStep, the camp keeps its state"));
	FHOLCampBoardStep Answer;
	Answer.State = State;
	return Answer;
}

TArray<FHOLCampBoardShare> UHOLCampBoard::CampRewards_Implementation(const FHOLCampBoardSettings& Settings, const TArray<FHOLCampBoardHit>& Hits)
{
	UE_LOG(LogTemp, Error, TEXT("Camp: no script override of CampRewards, no reward"));
	return TArray<FHOLCampBoardShare>();
}
