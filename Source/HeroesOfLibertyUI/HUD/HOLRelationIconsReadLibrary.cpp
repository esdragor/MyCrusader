// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLRelationIconsReadLibrary.h"

#include "HUD/HOLRelationIcons.h"
#include "Match/HOLForumRules.h"

FString UHOLRelationIconsReadLibrary::GetIconPath(const int32 Relation)
{
	const TCHAR* Path = HOLRelationIcons::GetIconPath(static_cast<EHOLPlayerRelation>(Relation));
	return Path ? FString(Path) : FString();
}

FString UHOLRelationIconsReadLibrary::GetAllyPath() { return FString(HOLRelationIcons::AllyPath); }
FString UHOLRelationIconsReadLibrary::GetEnemyPath() { return FString(HOLRelationIcons::EnemyPath); }
int32 UHOLRelationIconsReadLibrary::GetRelationNone() { return static_cast<int32>(EHOLPlayerRelation::None); }
int32 UHOLRelationIconsReadLibrary::GetRelationSelf() { return static_cast<int32>(EHOLPlayerRelation::Self); }
int32 UHOLRelationIconsReadLibrary::GetRelationAlly() { return static_cast<int32>(EHOLPlayerRelation::Ally); }
int32 UHOLRelationIconsReadLibrary::GetRelationEnemy() { return static_cast<int32>(EHOLPlayerRelation::Enemy); }
int32 UHOLRelationIconsReadLibrary::GetRelationNeutral() { return static_cast<int32>(EHOLPlayerRelation::Neutral); }
