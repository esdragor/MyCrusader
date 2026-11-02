// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * The neutral camps' table (NeutralsInProgress.md, NQ3 = C, D613): Tools/Data/Camps.csv, one camp per line, its members a list of
 * unit types of Tools/Data/UnitTypes.csv. The prototype's "brigands" use existing types (Spearman, ManAtArms, Archer, Crossbowman),
 * owned by the neutral owner and drawn grey; real creatures will be new lines and new types after the prototype. The camps'
 * placement (which camp where) comes with the maps (N2); this file is only the content of a camp.
 *
 * Columns: Name, DisplayName, Members (type names joined by ';', a type repeated = that many units).
 */
namespace HOLCampTable
{
	struct FCampDef
	{
		FName Name;
		FString DisplayName;
		TArray<FName> Members;
	};

	/** Reads the text of Camps.csv. Problems (bad header, wrong cell count, duplicate name, empty members) are appended to OutProblems. */
	HEROESOFLIBERTY_API void Parse(const FString& Text, TArray<FCampDef>& OutCamps, TArray<FString>& OutProblems);

	/** The first column (row names) of the text of UnitTypes.csv. */
	HEROESOFLIBERTY_API TSet<FName> ReadTypeNames(const FString& UnitTypesText);

	/** The members of a camp that are not in the known types, in order, without repeats. */
	HEROESOFLIBERTY_API TArray<FName> UnknownMembers(const FCampDef& Camp, const TSet<FName>& KnownTypes);
}
