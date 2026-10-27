// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLHeroReturnSubsystem.h"

#include "Engine/World.h"

UHOLHeroReturnSubsystem* UHOLHeroReturnSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLHeroReturnSubsystem*> Decisions = World->GetSubsystemArrayCopy<UHOLHeroReturnSubsystem>();
	return Decisions.Num() > 0 ? Decisions[0] : nullptr;
}

bool UHOLHeroReturnSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UScriptWorldSubsystem, UHOLAIBrainSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLHeroReturnSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UHOLHeroReturnSubsystem::DecideNow(const int32 Step)
{
	const UWorld* World = GetWorld();
	// The server (or a solo match) decides alone: a client only reads the copies.
	if (!World || World->GetNetMode() == NM_Client)
	{
		return false;
	}
	const bool bDecided = Decide(Step);
	if (bDecided)
	{
		++NumDecisions;
	}
	return bDecided;
}

bool UHOLHeroReturnSubsystem::BuyNow(const int32 Slot, const int32 Step)
{
	const UWorld* World = GetWorld();
	// The server (or a solo match) alone: a client's order reaches it through ServerReturnHero.
	if (!World || World->GetNetMode() == NM_Client)
	{
		return false;
	}
	return Buy(Slot, Step);
}
