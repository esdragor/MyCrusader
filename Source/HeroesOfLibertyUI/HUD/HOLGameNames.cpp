#include "HUD/HOLGameNames.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLUnitTypes.h"

#define LOCTEXT_NAMESPACE "HOLGameNames"

namespace HOLGameNames
{
	FText GetUnitName(const FName TypeName, const FHOLUnitTypeRow* Row)
	{
		static const FName Peasant(TEXT("Peasant"));
		static const FName Spearman(TEXT("Spearman"));
		static const FName ManAtArms(TEXT("ManAtArms"));
		static const FName Archer(TEXT("Archer"));
		static const FName King(TEXT("King"));
		if (TypeName == Peasant)
		{
			return LOCTEXT("Peasant", "Paysan");
		}
		if (TypeName == Spearman)
		{
			return LOCTEXT("Spearman", "Lancier");
		}
		if (TypeName == ManAtArms)
		{
			return LOCTEXT("ManAtArms", "Homme d'armes");
		}
		if (TypeName == Archer)
		{
			return LOCTEXT("Archer", "Archer");
		}
		if (TypeName == King)
		{
			// V5.1, translated in V5.4 (Tools/Localization/fill_po.py).
			return LOCTEXT("King", "Roi");
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
		if (TypeName == HOLBuildingTypeNames::House)
		{
			return LOCTEXT("House", "Maison");
		}
		if (TypeName == HOLBuildingTypeNames::LumberCamp)
		{
			return LOCTEXT("LumberCamp", "Camp de bûcherons");
		}
		if (TypeName == HOLBuildingTypeNames::MiningCamp)
		{
			return LOCTEXT("MiningCamp", "Camp de mineurs");
		}
		if (TypeName == HOLBuildingTypeNames::Mill)
		{
			return LOCTEXT("Mill", "Moulin");
		}
		if (TypeName == HOLBuildingTypeNames::Barracks)
		{
			return LOCTEXT("Barracks", "Caserne");
		}
		if (TypeName == HOLBuildingTypeNames::Farm)
		{
			// IA1.2 F8, translated by Tools/Localization/fill_po.py.
			return LOCTEXT("Farm", "Ferme");
		}
		if (Row && !Row->DisplayName.IsEmpty())
		{
			return Row->DisplayName;
		}
		return FText::FromName(TypeName);
	}

	FText GetSourceName(const FName TypeName, const FHOLSourceTypeRow* Row)
	{
		if (TypeName == HOLSourceTypeNames::Tree)
		{
			return LOCTEXT("Tree", "Arbre");
		}
		if (TypeName == HOLSourceTypeNames::BerryBush)
		{
			return LOCTEXT("BerryBush", "Buisson de baies");
		}
		if (TypeName == HOLSourceTypeNames::StoneVein)
		{
			return LOCTEXT("StoneVein", "Filon de pierre");
		}
		if (TypeName == HOLSourceTypeNames::GoldVein)
		{
			return LOCTEXT("GoldVein", "Filon d'or");
		}
		if (TypeName == HOLSourceTypeNames::FarmField)
		{
			// A farm's field (IA1.2 F8).
			return LOCTEXT("FarmField", "Champ");
		}
		if (Row && !Row->DisplayName.IsEmpty())
		{
			return Row->DisplayName;
		}
		return FText::FromName(TypeName);
	}
}

#undef LOCTEXT_NAMESPACE
