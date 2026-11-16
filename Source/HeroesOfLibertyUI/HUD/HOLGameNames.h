#pragma once

#include "CoreMinimal.h"

struct FHOLBuildingTypeRow;
struct FHOLGridCommand;
struct FHOLSourceTypeRow;
struct FHOLTechnologyRow;
struct FHOLUnitTypeRow;

/**
 * Names of the unit, building and resource source types as the player reads them (D245): the type tables are data the localization
 * gather (source code only) does not see, and a unit's DisplayName is not even a text. The names shown are written
 * here, by type name, as localized texts; a type missing here shows its table's name.
 */
namespace HOLGameNames
{
	HEROESOFLIBERTYUI_API FText GetUnitName(FName TypeName, const FHOLUnitTypeRow* Row);
	/** L2: the name of a unit type for a faction (Legions: their own names, `DisplayName` rows of FactionOverrides.csv, provisional until L13); None or a faction without its own name: GetUnitName(TypeName, Row). */
	HEROESOFLIBERTYUI_API FText GetUnitName(FName TypeName, const FHOLUnitTypeRow* Row, FName FactionKey);
	HEROESOFLIBERTYUI_API FText GetBuildingName(FName TypeName, const FHOLBuildingTypeRow* Row);
	HEROESOFLIBERTYUI_API FText GetSourceName(FName TypeName, const FHOLSourceTypeRow* Row);
	/** T8: a technology's name (a table's DisplayName is not a localized text), and its effect, one line by statistic ("Récolte du bois +10 %"). */
	HEROESOFLIBERTYUI_API FText GetTechnologyName(FName TypeName, const FHOLTechnologyRow* Row);
	HEROESOFLIBERTYUI_API FText GetTechnologyEffect(const FHOLTechnologyRow& Row);
	/** SP3: the name of Option (0 or 1) of the specialization Tier (provisional Â« Voie A / B du palier N Â», Placeholders.txt; GDD 13 per faction to come). */
	HEROESOFLIBERTYUI_API FText GetSpecializationName(int32 Tier, int32 Option);
	/**
	 * SP3: what a specialization cell of the Forum's grid writes (Specialize / CancelSpecialization): the option's name; running, two
	 * short lines (the seconds left, then Â« Annuler Â»), the option's name in its tooltip (ELROND-1115i: four lines overflowed the cell).
	 */
	HEROESOFLIBERTYUI_API FText GetSpecializationCellText(const FHOLGridCommand& Command);
}
