#include "HUD/HOLGameNames.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Simulation/HOLUnitTypes.h"

#define LOCTEXT_NAMESPACE "HOLGameNames"

namespace HOLGameNames
{
	FText GetUnitName(const FName TypeName, const FHOLUnitTypeRow* Row)
	{
		static const FName Peasant(TEXT("Peasant"));
		if (TypeName == Peasant)
		{
			return LOCTEXT("Peasant", "Paysan");
		}
		if (Row && !Row->DisplayName.IsEmpty())
		{
			return FText::AsCultureInvariant(Row->DisplayName);
		}
		return FText::FromName(TypeName);
	}

	FText GetBuildingName(const FName TypeName, const FHOLBuildingTypeRow* Row)
	{
		if (TypeName == HOLBuildingTypeNames::MainCentre)
		{
			return LOCTEXT("MainCentre", "Centre principal");
		}
		if (Row && !Row->DisplayName.IsEmpty())
		{
			return Row->DisplayName;
		}
		return FText::FromName(TypeName);
	}
}

#undef LOCTEXT_NAMESPACE
