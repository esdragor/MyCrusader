// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLRelationIconsReadLibrary.generated.h"

/**
 * The players' table's relation icons (HOLRelationIcons::GetIconPath in HUD/HOLRelationIcons.h, the C++ rule) read by Angelscript
 * (namespace HOLRelationIconsRead there), so that its test (HOLRelationIconsTests.cpp) can be ported to script (E3-10, D830, D839).
 * Wrappers only: no rule here. EHOLPlayerRelation is not a UENUM, so the relation comes as its number (GetRelationNone... below).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLRelationIconsReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLRelationIcons::GetIconPath of the EHOLPlayerRelation number Relation: the icon's object path, empty when it has none (Self, None). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static FString GetIconPath(int32 Relation);

	/** HOLRelationIcons::AllyPath. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static FString GetAllyPath();

	/** HOLRelationIcons::EnemyPath. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static FString GetEnemyPath();

	/** EHOLPlayerRelation::None (a spectator's view). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static int32 GetRelationNone();

	/** EHOLPlayerRelation::Self (the viewer's own row). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static int32 GetRelationSelf();

	/** EHOLPlayerRelation::Ally. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static int32 GetRelationAlly();

	/** EHOLPlayerRelation::Enemy. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static int32 GetRelationEnemy();

	/** EHOLPlayerRelation::Neutral (Diplomacy mode). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|RelationIcons")
	static int32 GetRelationNeutral();
};
