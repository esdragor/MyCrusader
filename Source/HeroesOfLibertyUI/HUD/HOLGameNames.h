#pragma once

#include "CoreMinimal.h"

struct FHOLBuildingTypeRow;
struct FHOLSourceTypeRow;
struct FHOLUnitTypeRow;

/**
 * Names of the unit, building and resource source types as the player reads them (D245): the type tables are data the localization
 * gather (source code only) does not see, and a unit's DisplayName is not even a text. The names shown are written
 * here, by type name, as localized texts; a type missing here shows its table's name.
 */
namespace HOLGameNames
{
	HEROESOFLIBERTYUI_API FText GetUnitName(FName TypeName, const FHOLUnitTypeRow* Row);
	HEROESOFLIBERTYUI_API FText GetBuildingName(FName TypeName, const FHOLBuildingTypeRow* Row);
	HEROESOFLIBERTYUI_API FText GetSourceName(FName TypeName, const FHOLSourceTypeRow* Row);
}
