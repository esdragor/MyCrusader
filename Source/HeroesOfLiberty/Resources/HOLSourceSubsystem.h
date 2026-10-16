// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLResources.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLSourceSubsystem.generated.h"

class AHOLSourceChunk;
class AHOLSourceField;
class UDataTable;
struct FHOLSourceTypeRow;

/** A resource source of the match (D297). */
struct FHOLSource
{
	/** Index in UHOLSourceSubsystem::GetTypeNames; NoType: an index not used (or not received yet, on a client). */
	uint8 TypeIndex = NoType;
	EHOLResource Resource = EHOLResource::Wood;
	/** Ground point (cm). */
	FVector Location = FVector::ZeroVector;
	/** Body radius (cm), from its type. */
	float Radius = 0.f;
	/** What it still holds; 0: used up, it left the map. */
	int32 Remaining = 0;

	static constexpr uint8 NoType = 0xFF;

	bool IsStanding() const { return TypeIndex != NoType && Remaining > 0; }
};

/**
 * The registry of the resource sources (D297). On the server, or in a solo game, it holds them (read from the map's
 * AHOLSourceLayout when the match starts, or added by code: the stress test of J5a.3), takes from them for the
 * gathering, forbids them on the navmesh and, in a networked match, sends them in chunks of 63 (AHOLSourceChunk). On a
 * client, it holds what the chunks bring. On every machine that draws, an AHOLSourceField shows them. Sources keep their
 * index for the whole match: a used-up source stays in the list with nothing left.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSourceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** The server or the solo game: the registry decides; a client only mirrors. */
	bool HasAuthority() const { return bAuthority; }

	/**
	 * Server: adds a source of the type TypeName on Location (its ground point), holding Amount (the type's amount when
	 * INDEX_NONE). Its index, INDEX_NONE if the type is unknown.
	 */
	int32 AddSource(FName TypeName, const FVector& Location, int32 Amount = INDEX_NONE);

	/**
	 * Server: takes up to Amount from a standing source; returns what was taken. A source left with nothing is used up:
	 * it leaves the map (display, navmesh, every client).
	 */
	int32 Take(int32 SourceIndex, int32 Amount);

	const TArray<FHOLSource>& GetSources() const { return Sources; }
	int32 GetNumStanding() const { return NumStanding; }

	/** The source type table's rows, in its order: their indices travel instead of names. */
	const TArray<FName>& GetTypeNames() const { return TypeNames; }
	const FHOLSourceTypeRow* GetType(uint8 TypeIndex) const;

	/** Server: what a player's gatherers unloaded since the start (reports, stress test S7). */
	void RecordDeposit(uint8 OwnerPlayer, EHOLResource Resource, int32 Amount);
	FHOLResources GetDeposited(uint8 OwnerPlayer) const { return Deposited.FindRef(OwnerPlayer); }

	/** The standing source nearest Location (of Resource only, when set), INDEX_NONE if there is none. */
	int32 FindNearestStanding(const FVector& Location, TOptional<EHOLResource> Resource = {}) const;

	/** Client: a chunk arrived (left); its sources are read when all its slots are in. */
	void AddChunk(AHOLSourceChunk* Chunk);
	void RemoveChunk(AHOLSourceChunk* Chunk);
	void ReadChunk(const AHOLSourceChunk& Chunk);

	/** "Sources: 320 standing of 320 (Tree 320: 32000 wood left); shown 320, navmesh 320": for the scripted checks. */
	FString MakeReport() const;

private:
	void LoadTypes();
	/** The source enters this machine's map: display, navmesh (server), chunk (networked server). */
	void OnSourceAdded(int32 SourceIndex);
	void OnSourceUsedUp(int32 SourceIndex);
	AHOLSourceChunk* GetOrAddChunk(int32 ChunkIndex);

	/** Kept loaded: the cached rows point into it. */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> TypesTable;

	TArray<FName> TypeNames;
	TArray<const FHOLSourceTypeRow*> TypeRows;

	TArray<FHOLSource> Sources;
	int32 NumStanding = 0;
	TMap<uint8, FHOLResources> Deposited;

	UPROPERTY(Transient)
	TObjectPtr<AHOLSourceField> Field;

	/** Server of a networked match: the chunks it spawned. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<AHOLSourceChunk>> Chunks;

	bool bStarted = false;
	bool bAuthority = false;
	bool bReplicate = false;
};
