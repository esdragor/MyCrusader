// The Diplomacy panel's rows (D7 of DiplomacyInProgress.md, GDD D387, pure): for each player, what the line shows and what it lets the viewer do,
// from the facts the game gives (the stances of the book, the Market, the mode). The three presentations of the panel (hol.UI.DiplomacyLayout
// 0|1|2) read these same rows: they differ by where the controls sit, never by what they do. Whole numbers and flags only; nothing here reads
// the world. Tested in HOLDiplomacyPanel_Test.as on the real rules (HOLDiplomacyRules.as).
struct FHOLDiplomacyRowView
{
	int Slot = -1;
	// The viewer's own line: no control (one does not set a stance toward oneself), the lives only.
	bool bSelf = false;
	bool bAI = false;
	bool bDefeated = false;
	// The stance the viewer has set toward this player, the one this player has set toward the viewer, and the relation (the most hostile, D375).
	int MyStance = 0;
	int TheirStance = 0;
	int Relation = 0;
	// The three posture controls may be used. False: shown but greyed (the standard modes' fixed teams, an AI that is always Enemy, D367, a
	// defeated player, a spectator).
	bool bStanceEditable = false;
	// « Share my vision » (D371): ticked now, and whether it may be changed (allies only; automatic between teammates of the standard modes).
	bool bVisionChecked = false;
	bool bVisionEnabled = false;
	// The tribute (D372): usable only with a finished Market of the viewer's own and toward a player still in the match; the tax is shown on the button.
	bool bTributeEnabled = false;
	bool bNoMarket = false;
	int TaxPercent = 0;
	// The private message (D385): the button opens the discussion, which is another chantier; usable toward any other human still in the match.
	bool bMessageEnabled = false;
	// "slot 2 me ally them enemy stance on | vision off off | tribute on tax 30 | message on" (hol.HUD.Report).
	FString Describe;
}

namespace HOLDiplomacyPanel
{
	// The stance cycled by a click on the colour chip (layout 1): Ally -> Neutral -> Enemy -> Ally, the order the GDD names them.
	int NextStance(int Stance)
	{
		if (Stance == HOLDiplomacy::Ally)
			return HOLDiplomacy::Neutral;
		if (Stance == HOLDiplomacy::Neutral)
			return HOLDiplomacy::Enemy;
		return HOLDiplomacy::Ally;
	}

	// The facts are the viewer's seat (Viewer, -1 for a spectator), the other seat (Slot) and what the book says of the pair.
	FHOLDiplomacyRowView MakeRow(bool bDiplomacyMode, int Viewer, bool bViewerDefeated, int Slot, bool bSlotAI, bool bSlotDefeated,
		int MyStance, int TheirStance, bool bVision, bool bViewerHasMarket, int TaxPercent)
	{
		FHOLDiplomacyRowView Row;
		Row.Slot = Slot;
		Row.bSelf = Slot == Viewer;
		Row.bAI = bSlotAI;
		Row.bDefeated = bSlotDefeated;
		Row.MyStance = MyStance;
		Row.TheirStance = TheirStance;
		Row.Relation = Math::Min(MyStance, TheirStance);
		// An AI is Enemy to everybody whatever is said to it (D367).
		if (bDiplomacyMode && bSlotAI)
		{
			Row.MyStance = HOLDiplomacy::Enemy;
			Row.Relation = HOLDiplomacy::Enemy;
		}
		const bool bViewerPlays = Viewer >= 0 && !bViewerDefeated;
		const bool bOther = !Row.bSelf && bViewerPlays;
		Row.bStanceEditable = bDiplomacyMode && bOther && !bSlotAI && !bSlotDefeated;
		Row.bVisionChecked = bVision && Row.Relation == HOLDiplomacy::Ally;
		Row.bVisionEnabled = Row.bStanceEditable && Row.Relation == HOLDiplomacy::Ally;
		Row.TaxPercent = TaxPercent;
		Row.bNoMarket = !bViewerHasMarket;
		Row.bTributeEnabled = bOther && bViewerHasMarket && !bSlotDefeated;
		Row.bMessageEnabled = bOther && !bSlotAI && !bSlotDefeated;
		Row.Describe = "slot " + Slot + " me " + StanceName(Row.MyStance) + " them " + StanceName(Row.TheirStance)
			+ (Row.bStanceEditable ? " stance on" : " stance locked")
			+ " | vision " + OnOff(Row.bVisionChecked) + " " + OnOff(Row.bVisionEnabled)
			+ " | tribute " + (Row.bTributeEnabled ? "on" : (Row.bNoMarket ? "no market" : "off")) + " tax " + TaxPercent
			+ " | message " + OnOff(Row.bMessageEnabled);
		return Row;
	}

	FString OnOff(bool bOn)
	{
		return bOn ? "on" : "off";
	}

	FString StanceName(int Stance)
	{
		if (Stance == HOLDiplomacy::Ally)
			return "ally";
		if (Stance == HOLDiplomacy::Neutral)
			return "neutral";
		return "enemy";
	}
	// E-diplomacy_locked (the user, 2026-11-05): no switch button, FOUR tribute buttons on a line, one per resource, each giving its own. The resources by
	// button, left to right (HOLResource: 0 food, 1 wood, 2 stone, 3 gold): gold, food, wood, stone, the order the user named them.
	const int TributeButtonCount = 4;

	int TributeResource(int Index)
	{
		if (Index == 1) return 0;
		if (Index == 2) return 1;
		if (Index == 3) return 2;
		return 3;
	}

	// The click's kind of a tribute button (the panel's other kinds are 0..10): 20 + the resource.
	const int TributeKindBase = 20;

	int TributeKind(int Resource)
	{
		return TributeKindBase + Resource;
	}

	// The resource a click's kind stands for, -1 for any other kind (the old single tribute kind 6 and the old switch kind 10 give none).
	int TributeResourceOfKind(int Kind)
	{
		if (Kind >= TributeKindBase && Kind < TributeKindBase + TributeButtonCount)
			return Kind - TributeKindBase;
		return -1;
	}

	// The name hol.HUD.Click reaches the control by: DipTribute_<slot>_<resource>.
	FName TributeControlName(int Slot, int Resource)
	{
		return FName(f"DipTribute_{Slot}_{Resource}");
	}

	// The resource of such a name (any of the lines), -1 for any other name.
	int TributeResourceOfName(FName Name)
	{
		for (int Slot = 0; Slot < 8; ++Slot)
		{
			for (int Resource = 0; Resource < TributeButtonCount; ++Resource)
			{
				if (Name == TributeControlName(Slot, Resource))
					return Resource;
			}
		}
		return -1;
	}
}

// The panel's texts by key (French source here, gathered from the .as files by GatherText; English and Japanese in
// Tools/Localization/fill_po.py, keys "HOLDiplomacyPanel,<Name>"). Literals only in these calls. Names are the GDD's: « Allié »,
// « Neutre », « Ennemi » (D369), « Diplomatie » (§ 14.3, D387), « vision » (D371), « tribut » (D372), « message privé » (D385).
namespace HOLDiplomacyPanelText
{
	const FString Title = "Diplomacy.Title";
	const FString Button = "Diplomacy.Button";
	const FString ButtonTip = "Diplomacy.ButtonTip";
	const FString Ally = "Diplomacy.Ally";
	const FString Neutral = "Diplomacy.Neutral";
	const FString Enemy = "Diplomacy.Enemy";
	const FString TaxNote = "Diplomacy.TaxNote";
	const FString ResFood = "Diplomacy.ResFood";
	const FString ResWood = "Diplomacy.ResWood";
	const FString ResStone = "Diplomacy.ResStone";
	const FString ResGold = "Diplomacy.ResGold";
	const FString Vision = "Diplomacy.Vision";
	const FString VisionTip = "Diplomacy.VisionTip";
	const FString Tribute = "Diplomacy.Tribute";
	const FString TributeTip = "Diplomacy.TributeTip";
	const FString TributeNoMarket = "Diplomacy.TributeNoMarket";
	const FString Message = "Diplomacy.Message";
	const FString MessageTip = "Diplomacy.MessageTip";
	const FString Locked = "Diplomacy.Locked";
	const FString Close = "Diplomacy.Close";
	const FString AIAlways = "Diplomacy.AIAlways";

	FText PatternText(FString Key)
	{
		if (Key == Title) return NSLOCTEXT("HOLDiplomacyPanel", "Title", "Diplomatie");
		if (Key == Button) return NSLOCTEXT("HOLDiplomacyPanel", "Button", "Diplomatie");
		if (Key == ButtonTip) return NSLOCTEXT("HOLDiplomacyPanel", "ButtonTip", "Ouvrir ou fermer la diplomatie (Ctrl + D)");
		if (Key == Ally) return NSLOCTEXT("HOLDiplomacyPanel", "Ally", "Allié");
		if (Key == Neutral) return NSLOCTEXT("HOLDiplomacyPanel", "Neutral", "Neutre");
		if (Key == Enemy) return NSLOCTEXT("HOLDiplomacyPanel", "Enemy", "Ennemi");
		if (Key == TaxNote) return NSLOCTEXT("HOLDiplomacyPanel", "TaxNote", "Tribut : taxe de {0} % gardée par le Marché");
		if (Key == ResFood) return NSLOCTEXT("HOLDiplomacyPanel", "ResFood", "nourriture");
		if (Key == ResWood) return NSLOCTEXT("HOLDiplomacyPanel", "ResWood", "bois");
		if (Key == ResStone) return NSLOCTEXT("HOLDiplomacyPanel", "ResStone", "pierre");
		if (Key == ResGold) return NSLOCTEXT("HOLDiplomacyPanel", "ResGold", "or");
		if (Key == Vision) return NSLOCTEXT("HOLDiplomacyPanel", "Vision", "Vision");
		if (Key == VisionTip) return NSLOCTEXT("HOLDiplomacyPanel", "VisionTip", "Partager ma vision avec ce joueur (alliés seulement)");
		if (Key == Tribute) return NSLOCTEXT("HOLDiplomacyPanel", "Tribute", "Tribut 100 {0}");
		if (Key == TributeTip) return NSLOCTEXT("HOLDiplomacyPanel", "TributeTip", "Donner 100 {0} à ce joueur ; le Marché garde {1} %");
		if (Key == TributeNoMarket) return NSLOCTEXT("HOLDiplomacyPanel", "TributeNoMarket", "Il faut un Marché terminé pour faire un tribut");
		if (Key == Message) return NSLOCTEXT("HOLDiplomacyPanel", "Message", "Message");
		if (Key == MessageTip) return NSLOCTEXT("HOLDiplomacyPanel", "MessageTip", "Message privé à ce joueur");
		if (Key == Locked) return NSLOCTEXT("HOLDiplomacyPanel", "Locked", "Postures verrouillées : équipes fixes");
		if (Key == Close) return NSLOCTEXT("HOLDiplomacyPanel", "Close", "Fermer");
		if (Key == AIAlways) return NSLOCTEXT("HOLDiplomacyPanel", "AIAlways", "Les IA sont toujours ennemies");
		return FText();
	}

	FString Format(FString Key, FString First = "", FString Second = "")
	{
		return PatternText(Key).ToString().Replace("{0}", First).Replace("{1}", Second);
	}

	// The tribute's resource names, in the game's order (HOLResource: 0 food, 1 wood, 2 stone, 3 gold).
	FString ResourceName(int Resource)
	{
		if (Resource == 0) return PatternText(ResFood).ToString();
		if (Resource == 1) return PatternText(ResWood).ToString();
		if (Resource == 2) return PatternText(ResStone).ToString();
		return PatternText(ResGold).ToString();
	}

	FText StanceLabel(int Stance)
	{
		if (Stance == HOLDiplomacy::Ally)
			return PatternText(Ally);
		if (Stance == HOLDiplomacy::Neutral)
			return PatternText(Neutral);
		return PatternText(Enemy);
	}
}
