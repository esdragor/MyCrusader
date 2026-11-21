// The DISPLAY line of one power box (GALADRIEL-1121sh): the player's text, separate from HOLPowerBook::Describe (the stable ASCII line of
// the tests and of hol.HUD.Report, which stays untouched). Pure rule (Make: row, state, cool-down -> text key + arguments), then the
// text table (NSLOCTEXT "HOLFactionSlot", keys PowerReady... translated in Content/Localization/Game/<culture>/Game.po) and the composition.
struct FHOLPowerDisplay
{
	// Key of the text in the table (namespace "HOLFactionSlot"): PowerReady, PowerCooling, PowerShort, PowerTier, PowerSoon.
	FString TextKey;
	FString Name;
	int Cost = 0;
	// {2} of the pattern: the seconds left while cooling, the tier required while tier-locked, else 0.
	int Extra = 0;
}

namespace HOLPowerDisplay
{
	FHOLPowerDisplay Make(const FHOLPowerRow& Row, EHOLPowerState State, int CooldownLeftMs)
	{
		FHOLPowerDisplay Display;
		Display.Name = Row.Key;
		Display.Cost = Row.Cost;
		if (State == EHOLPowerState::Ready)
		{
			Display.TextKey = "PowerReady";
		}
		else if (State == EHOLPowerState::Cooling)
		{
			Display.TextKey = "PowerCooling";
			Display.Extra = (CooldownLeftMs + 999) / 1000;
		}
		else if (State == EHOLPowerState::ShortOfResource)
		{
			Display.TextKey = "PowerShort";
		}
		else if (State == EHOLPowerState::TierLocked)
		{
			Display.TextKey = "PowerTier";
			Display.Extra = Row.MinTier;
		}
		else
		{
			Display.TextKey = "PowerSoon";
		}
		return Display;
	}

	FString Compose(const FString& Pattern, const FHOLPowerDisplay& Display)
	{
		return Pattern.Replace("{0}", Display.Name).Replace("{1}", f"{Display.Cost}").Replace("{2}", f"{Display.Extra}");
	}

	// The texts of the table (source = French; EN / JA in Game.po). Same keys as the display's TextKey.
	FText PatternOf(const FString& TextKey)
	{
		if (TextKey == "PowerReady")
			return NSLOCTEXT("HOLFactionSlot", "PowerReady", "{0} : coût {1}, prêt");
		if (TextKey == "PowerCooling")
			return NSLOCTEXT("HOLFactionSlot", "PowerCooling", "{0} : coût {1}, recharge {2} s");
		if (TextKey == "PowerShort")
			return NSLOCTEXT("HOLFactionSlot", "PowerShort", "{0} : coût {1}, ressource insuffisante");
		if (TextKey == "PowerTier")
			return NSLOCTEXT("HOLFactionSlot", "PowerTier", "{0} : coût {1}, palier {2} requis");
		return NSLOCTEXT("HOLFactionSlot", "PowerSoon", "{0} : coût {1}, bientôt");
	}

	// The name of a power by its book key. PROVISIONAL (Placeholders.txt): only the Dawn's first power has a text; any other key shows as is.
	FText NameOf(const FString& Key)
	{
		if (Key == "SacredLight")
			return NSLOCTEXT("HOLFactionSlot", "PowerNameSacredLight", "Lumière sacrée");
		return FText::FromString(Key);
	}

	// The line the HUD shows for one box of the slot, in the current language.
	FString LineOf(const FHOLFactionPowerBox& Box)
	{
		FHOLPowerRow Row;
		Row.Key = Box.Key;
		Row.Cost = Box.Cost;
		Row.MinTier = Box.MinTier;
		FHOLPowerDisplay Display = Make(Row, Box.State, Box.CooldownLeftMs);
		Display.Name = NameOf(Box.Key).ToString();
		return Compose(PatternOf(Display.TextKey).ToString(), Display);
	}
}
