#include "HUD/HOLGameNames.h"

#include "Buildings/HOLBuildingTypes.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLUnitTypes.h"
#include "Technologies/HOLTechnologyTypes.h"

#define LOCTEXT_NAMESPACE "HOLGameNames"

namespace HOLGameNames
{
	FText GetUnitName(const FName TypeName, const FHOLUnitTypeRow* Row)
	{
		static const FName Peasant(TEXT("Peasant"));
		static const FName Spearman(TEXT("Spearman"));
		static const FName ManAtArms(TEXT("ManAtArms"));
		static const FName Archer(TEXT("Archer"));
		static const FName Crossbowman(TEXT("Crossbowman"));
		static const FName King(TEXT("King"));
		static const FName Hero(TEXT("Hero"));
		static const FName Ram(TEXT("Ram"));
		static const FName Mangonel(TEXT("Mangonel"));
		if (TypeName == Ram)
		{
			// S2 (GDD 6.1), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Ram", "Bélier");
		}
		if (TypeName == FName(TEXT("Monk")))
		{
			// Q-chapelle-moine = A (GDD 7.1, D134), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Monk", "Moine");
		}
		if (TypeName == Mangonel)
		{
			// S2 (GDD 6.1), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Mangonel", "Mangonneau");
		}
		if (TypeName == FName(TEXT("DamnedWarrior")))
		{
			// L4 (GDD 11.1 "Guerrier Damné", the Legions' Barracks), translated in Tools/Localization/fill_po.py; a key of its own so that the
			// Barracks tooltip (no table row there) and the grid show a name, not the raw type id.
			return LOCTEXT("DamnedWarrior", "Guerrier damné");
		}
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
		if (TypeName == Crossbowman)
		{
			// AR8, translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Crossbowman", "Arbalétrier");
		}
		if (TypeName == FName(TEXT("Arquebusier")))
		{
			// G3 (Palier 3, D713), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Arquebusier", "Arquebusier");
		}
		if (TypeName == King)
		{
			// V5.1, translated in V5.4 (Tools/Localization/fill_po.py).
			return LOCTEXT("King", "Roi");
		}
		if (TypeName == Hero)
		{
			// J8a.2: the hero (J8a-Q1), translated by Tools/Localization/fill_po.py.
			return LOCTEXT("Hero", "Paladin-Commandant");
		}
		if (Row && !Row->DisplayName.IsEmpty())
		{
			return FText::AsCultureInvariant(Row->DisplayName);
		}
		return FText::FromName(TypeName);
	}

	FText GetUnitName(const FName TypeName, const FHOLUnitTypeRow* Row, const FName FactionKey)
	{
		// The Legions' own names (FactionOverrides.csv, rows `Legions_<Unit>_Name`, field DisplayName; names of 2026-11-07: French short, EN and JA in the .po by fill_po.py): one key per
		// unit and faction, the texts kept equal to the table by the test UI.GameNames.FactionNames.
		if (FactionKey == FName(TEXT("Legions")))
		{
			static const TMap<FName, FText> LegionsNames = {
				{ FName(TEXT("Spearman")), LOCTEXT("Legions_Spearman", "Lancier squelette") },
				{ FName(TEXT("ManAtArms")), LOCTEXT("Legions_ManAtArms", "Garde noire") },
				{ FName(TEXT("Archer")), LOCTEXT("Legions_Archer", "Archer squelette") },
				{ FName(TEXT("Crossbowman")), LOCTEXT("Legions_Crossbowman", "Arbalétrier squelette") },
				{ FName(TEXT("LightCavalry")), LOCTEXT("Legions_LightCavalry", "Cavalier funeste") },
				{ FName(TEXT("Ram")), LOCTEXT("Legions_Ram", "Bélier d'os") },
				{ FName(TEXT("Mangonel")), LOCTEXT("Legions_Mangonel", "Mangonneau d'os") },
			};
			if (const FText* Own = LegionsNames.Find(TypeName))
			{
				return *Own;
			}
		}
		return GetUnitName(TypeName, Row);
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
		if (TypeName == HOLBuildingTypeNames::ArcheryRange)
		{
			// AR7b, translated in Tools/Localization/fill_po.py.
			return LOCTEXT("ArcheryRange", "Champ de tir");
		}
		if (TypeName == HOLBuildingTypeNames::SiegeWorkshop)
		{
			// S2 (GDD 6.1, 11.1), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("SiegeWorkshop", "Atelier de siège");
		}
		if (TypeName == HOLBuildingTypeNames::Market)
		{
			// M3, translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Market", "Marché");
		}
		if (TypeName == FName(TEXT("Forge")))
		{
			// F1 (FortificationsInProgress.md), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Forge", "Forge");
		}
		if (TypeName == FName(TEXT("Chapel")))
		{
			// Q-chapelle-moine = A (GDD 11.1, D211), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Chapel", "Chapelle");
		}
		if (TypeName == FName(TEXT("Sanctuary")))
		{
			// SC-Q1 = B (D602), translated in Tools/Localization/fill_po.py.
			return LOCTEXT("Sanctuary", "Sanctuaire de l'Aube");
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
		// M-Q9: the veins in other sizes (the standard ones, 2 500, keep the plain names above).
		if (TypeName == HOLSourceTypeNames::StoneVeinSmall)
		{
			return LOCTEXT("StoneVeinSmall", "Petit filon de pierre");
		}
		if (TypeName == HOLSourceTypeNames::StoneVeinMedium)
		{
			return LOCTEXT("StoneVeinMedium", "Filon de pierre moyen");
		}
		if (TypeName == HOLSourceTypeNames::StoneVeinBig)
		{
			return LOCTEXT("StoneVeinBig", "Gros filon de pierre");
		}
		if (TypeName == HOLSourceTypeNames::GoldVeinSmall)
		{
			return LOCTEXT("GoldVeinSmall", "Petit filon d'or");
		}
		if (TypeName == HOLSourceTypeNames::GoldVeinMedium)
		{
			return LOCTEXT("GoldVeinMedium", "Filon d'or moyen");
		}
		if (TypeName == HOLSourceTypeNames::GoldVeinBig)
		{
			return LOCTEXT("GoldVeinBig", "Gros filon d'or");
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

	FText GetTechnologyName(const FName TypeName, const FHOLTechnologyRow* Row)
	{
		if (TypeName == FName(TEXT("MeleeAttackI")))
		{
			return LOCTEXT("TechMeleeAttackI", "Lames affûtées");
		}
		if (TypeName == FName(TEXT("MeleeAttackII")))
		{
			return LOCTEXT("TechMeleeAttackII", "Lames affûtées II");
		}
		if (TypeName == FName(TEXT("RangedAttackI")))
		{
			return LOCTEXT("TechRangedAttackI", "Empennage");
		}
		if (TypeName == FName(TEXT("RangedAttackII")))
		{
			return LOCTEXT("TechRangedAttackII", "Empennage II");
		}
		if (TypeName == FName(TEXT("InfantryArmorI")))
		{
			return LOCTEXT("TechInfantryArmorI", "Maille");
		}
		if (TypeName == FName(TEXT("InfantryArmorII")))
		{
			return LOCTEXT("TechInfantryArmorII", "Maille II");
		}
		if (TypeName == FName(TEXT("CavalryArmorI")))
		{
			return LOCTEXT("TechCavalryArmorI", "Barde");
		}
		if (TypeName == FName(TEXT("CavalryArmorII")))
		{
			return LOCTEXT("TechCavalryArmorII", "Barde II");
		}
		if (TypeName == FName(TEXT("DoubleAxesI")))
		{
			return LOCTEXT("TechDoubleAxesI", "Haches doubles");
		}
		if (TypeName == FName(TEXT("DoubleAxesII")))
		{
			return LOCTEXT("TechDoubleAxesII", "Haches doubles II");
		}
		if (TypeName == FName(TEXT("MinerPicksI")))
		{
			return LOCTEXT("TechMinerPicksI", "Pics de mineur");
		}
		if (TypeName == FName(TEXT("MinerPicksII")))
		{
			return LOCTEXT("TechMinerPicksII", "Pics de mineur II");
		}
		if (TypeName == FName(TEXT("PloughI")))
		{
			return LOCTEXT("TechPloughI", "Charrue");
		}
		if (TypeName == FName(TEXT("PloughII")))
		{
			return LOCTEXT("TechPloughII", "Charrue II");
		}
		if (TypeName == FName(TEXT("WheelbarrowI")))
		{
			return LOCTEXT("TechWheelbarrowI", "Brouette");
		}
		if (TypeName == FName(TEXT("WheelbarrowII")))
		{
			return LOCTEXT("TechWheelbarrowII", "Brouette II");
		}
		if (TypeName == FName(TEXT("ImprovedAlarmBell")))
		{
			return LOCTEXT("TechImprovedAlarmBell", "Cloche d'alarme améliorée");
		}
		if (TypeName == FName(TEXT("Masonry")))
		{
			return LOCTEXT("TechMasonry", "Maçonnerie");
		}
		if (TypeName == FName(TEXT("Veterans")))
		{
			return LOCTEXT("TechVeterans", "Vétérans");
		}
		if (TypeName == FName(TEXT("Logistics")))
		{
			return LOCTEXT("TechLogistics", "Logistique");
		}
		if (TypeName == FName(TEXT("ReinforcedRam")))
		{
			return LOCTEXT("TechReinforcedRam", "Bélier renforcé");
		}
		if (TypeName == FName(TEXT("HeavyMunitions")))
		{
			return LOCTEXT("TechHeavyMunitions", "Munitions lourdes");
		}
		if (TypeName == FName(TEXT("Liturgy")))
		{
			return LOCTEXT("TechLiturgy", "Liturgie");
		}
		if (TypeName == FName(TEXT("TemperedHalberds")))
		{
			return LOCTEXT("TechTemperedHalberds", "Hallebardes trempées");
		}
		if (TypeName == FName(TEXT("BlessingOfArms")))
		{
			return LOCTEXT("TechBlessingOfArms", "Bénédiction des armes");
		}
		if (TypeName == FName(TEXT("VowOfPoverty")))
		{
			return LOCTEXT("TechVowOfPoverty", "Vœu de pauvreté");
		}
		if (Row && !Row->DisplayName.IsEmpty())
		{
			return Row->DisplayName;
		}
		return FText::FromName(TypeName);
	}

	/** The name of a statistic a technology changes, as a title (T8). */
	static FText GetTechnologyStatName(const FName Stat)
	{
		if (Stat == FName(TEXT("MeleeAttack")))
		{
			return LOCTEXT("TechStatMeleeAttack", "Dégâts de mêlée");
		}
		if (Stat == FName(TEXT("RangedAttack")))
		{
			return LOCTEXT("TechStatRangedAttack", "Dégâts à distance");
		}
		if (Stat == FName(TEXT("InfantryArmor")))
		{
			return LOCTEXT("TechStatInfantryArmor", "Armure de l'infanterie");
		}
		if (Stat == FName(TEXT("CavalryArmor")))
		{
			return LOCTEXT("TechStatCavalryArmor", "Armure de la cavalerie");
		}
		if (Stat == FName(TEXT("GatherWood")))
		{
			return LOCTEXT("TechStatGatherWood", "Récolte du bois");
		}
		if (Stat == FName(TEXT("GatherStone")))
		{
			return LOCTEXT("TechStatGatherStone", "Récolte de la pierre");
		}
		if (Stat == FName(TEXT("GatherGold")))
		{
			return LOCTEXT("TechStatGatherGold", "Récolte de l'or");
		}
		if (Stat == FName(TEXT("GatherFood")))
		{
			return LOCTEXT("TechStatGatherFood", "Récolte de nourriture");
		}
		if (Stat == FName(TEXT("WorkerSpeed")))
		{
			return LOCTEXT("TechStatWorkerSpeed", "Vitesse des travailleurs");
		}
		if (Stat == FName(TEXT("WorkerCarry")))
		{
			return LOCTEXT("TechStatWorkerCarry", "Charge des travailleurs");
		}
		if (Stat == FName(TEXT("GarrisonSpeed")))
		{
			return LOCTEXT("TechStatGarrisonSpeed", "Vitesse de garnison");
		}
		if (Stat == FName(TEXT("WallTowerHealth")))
		{
			return LOCTEXT("TechStatWallTowerHealth", "Points de vie des murs et des tours");
		}
		if (Stat == FName(TEXT("CoreUnitHealth")))
		{
			return LOCTEXT("TechStatCoreUnitHealth", "Points de vie des unités de base");
		}
		if (Stat == FName(TEXT("ArmyMarchSpeed")))
		{
			return LOCTEXT("TechStatArmyMarchSpeed", "Vitesse de marche de l'armée");
		}
		if (Stat == FName(TEXT("RamHealth")))
		{
			return LOCTEXT("TechStatRamHealth", "Points de vie du bélier");
		}
		if (Stat == FName(TEXT("MangonelDamage")))
		{
			return LOCTEXT("TechStatMangonelDamage", "Dégâts du mangonneau");
		}
		if (Stat == FName(TEXT("HonourGain")))
		{
			return LOCTEXT("TechStatHonourGain", "Gain d'Honneur");
		}
		if (Stat == FName(TEXT("HalberdierBonusVsHeavyInfantry")))
		{
			return LOCTEXT("TechStatHalberdierBonusVsHeavyInfantry", "Bonus du hallebardier contre l'infanterie lourde");
		}
		if (Stat == FName(TEXT("KnightAndHalberdierBonusVsProfane")))
		{
			return LOCTEXT("TechStatKnightAndHalberdierBonusVsProfane", "Bonus du chevalier et du hallebardier contre les profanes");
		}
		if (Stat == FName(TEXT("MonkCostDiscount")))
		{
			return LOCTEXT("TechStatMonkCostDiscount", "Réduction du coût des moines");
		}
		return FText::FromName(Stat);
	}

	FText GetTechnologyEffect(const FHOLTechnologyRow& Row)
	{
		TArray<FText> Lines;
		for (const FName Stat : Row.Stats)
		{
			Lines.Add(FText::Format(LOCTEXT("TechEffectLine", "{0} +{1} %"), GetTechnologyStatName(Stat), FText::AsNumber(Row.Percent)));
		}
		return FText::Join(FText::FromString(TEXT("\n")), Lines);
	}
}

#undef LOCTEXT_NAMESPACE
