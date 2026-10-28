// The hero's block as a view (J8a.5 loop 3, HOLHeroBlock.as): what it shows from what it knows, on quotes made by the real
// HOLHeroReturnQuote::Compute (death at 1 000, level 1: ready at 1 760, 150 then down to 0 at 2 960).

FHOLHeroReturnQuote HeroBlockQuote(int Step, EHOLHeroState State, int Gold, bool bHasPlace)
{
	return HOLHeroReturnQuote::Compute(Step, State, 1000, 1, Gold, bHasPlace, false);
}

// Alive: level, HP bar and text, XP bar empty and greyed, 3 greyed abilities, no delay, no button.
void Test_HeroBlockAlive(FUnitTest& T)
{
	const FHOLHeroBlockView View = HOLHeroBlock::Make(false, false, EHOLHeroState::Alive, 1, 300, 450, -1,
		HOLHeroReturnQuote::Compute(500, EHOLHeroState::Alive, -1, 1, 500, true, false));
	T.AssertTrue(View.bVisible, "visible");
	T.AssertFalse(View.bPortraitDark, "portrait lit");
	T.AssertEquals(1, View.Level, "level 1");
	T.AssertEquals("Niveau 1", HOLHeroBlockText::LevelText(View), "Niveau 1");
	T.AssertEquals(300, View.Health, "300 HP");
	T.AssertEquals(450, View.MaxHealth, "of 450");
	T.AssertTrue(Math::Abs(View.HealthFraction - 300.0 / 450.0) < 0.001, "two thirds of the bar");
	T.AssertEquals("PV 300 / 450", HOLHeroBlockText::HealthText(View), "PV 300 / 450");
	T.AssertTrue(View.bXPGreyed, "XP greyed");
	T.AssertTrue(View.XPFraction == 0.0, "XP empty");
	T.AssertEquals(3, View.NumAbilities, "3 abilities");
	T.AssertTrue(View.bAbilitiesGreyed, "abilities greyed");
	T.AssertEquals("", View.DelayKey, "no delay");
	T.AssertFalse(View.bButtonVisible, "no button");
	T.AssertEquals("alive level 1 hp 300/450", View.Describe, "described");
}

// Dead, not ready: portrait dark, HP at 0 (whatever the sheet says), "Prêt dans 0:23" then "1:01", no button.
void Test_HeroBlockDeadWaiting(FUnitTest& T)
{
	// 23 s left: 460 steps before 1 760, the order taken at 1 300 (sent at 1 299).
	const FHOLHeroReturnQuote Quote = HeroBlockQuote(1299, EHOLHeroState::Dead, 500, true);
	T.AssertEquals(23, Quote.SecondsToReady, "the quote says 23 s");
	const FHOLHeroBlockView View = HOLHeroBlock::Make(false, false, EHOLHeroState::Dead, 1, 120, 450, -1, Quote);
	T.AssertTrue(View.bVisible, "visible");
	T.AssertTrue(View.bPortraitDark, "portrait dark");
	T.AssertEquals(0, View.Health, "0 HP");
	T.AssertTrue(View.HealthFraction == 0.0, "HP bar empty");
	T.AssertEquals("PV 0 / 450", HOLHeroBlockText::HealthText(View), "PV 0 / 450");
	T.AssertEquals("HeroBlock.ReadyIn", View.DelayKey, "the delay's key");
	T.AssertEquals("0:23", View.DelayClock, "0:23");
	T.AssertEquals("Prêt dans 0:23", HOLHeroBlockText::DelayText(View), "Prêt dans 0:23");
	T.AssertFalse(View.bButtonVisible, "no button yet");
	T.AssertEquals("dead ready in 23 s", View.Describe, "described");
	// 61 s left: 1 220 steps, the order taken at 540.
	const FHOLHeroBlockView Long = HOLHeroBlock::Make(false, false, EHOLHeroState::Dead, 1, 0, 450, -1,
		HeroBlockQuote(539, EHOLHeroState::Dead, 500, true));
	T.AssertEquals("1:01", Long.DelayClock, "1:01");
	T.AssertEquals("Prêt dans 1:01", HOLHeroBlockText::DelayText(Long), "Prêt dans 1:01");
	T.AssertEquals("dead ready in 61 s", Long.Describe, "described");
}

// Ready and buyable: the button lit, its price (76 at 1 760 + 599), no tooltip.
void Test_HeroBlockReadyBuyable(FUnitTest& T)
{
	const FHOLHeroBlockView View = HOLHeroBlock::Make(false, false, EHOLHeroState::ReadyToReturn, 1, 0, 450, -1,
		HeroBlockQuote(1760 + 598, EHOLHeroState::ReadyToReturn, 500, true));
	T.AssertTrue(View.bVisible, "visible");
	T.AssertTrue(View.bPortraitDark, "portrait dark");
	T.AssertEquals(0, View.Health, "0 HP");
	T.AssertEquals("", View.DelayKey, "no delay");
	T.AssertTrue(View.bButtonVisible, "button shown");
	T.AssertTrue(View.bButtonEnabled, "button lit");
	T.AssertEquals(76, View.Price, "76");
	T.AssertFalse(View.bPriceRed, "price not red");
	T.AssertEquals("", View.TooltipKey, "no tooltip");
	T.AssertEquals("Ramener le héros — 76 or", HOLHeroBlockText::ButtonText(View), "the button's text");
	T.AssertEquals("ready price 76 buyable", View.Describe, "described");
	// Ready one step before the sheet says so (still Dead): the button already.
	const FHOLHeroBlockView Early = HOLHeroBlock::Make(false, false, EHOLHeroState::Dead, 1, 0, 450, -1,
		HeroBlockQuote(1759, EHOLHeroState::Dead, 500, true));
	T.AssertEquals("ready price 150 buyable", Early.Describe, "ready from the quote");
}

// Ready, gold short: the button greyed, the price red, "Pas assez d'or".
void Test_HeroBlockReadyNoGold(FUnitTest& T)
{
	const FHOLHeroBlockView View = HOLHeroBlock::Make(false, false, EHOLHeroState::ReadyToReturn, 1, 0, 450, -1,
		HeroBlockQuote(1759, EHOLHeroState::ReadyToReturn, 149, true));
	T.AssertTrue(View.bButtonVisible, "button shown");
	T.AssertFalse(View.bButtonEnabled, "button greyed");
	T.AssertEquals(150, View.Price, "150");
	T.AssertTrue(View.bPriceRed, "price red");
	T.AssertEquals("HeroBlock.NoGold", View.TooltipKey, "the tooltip's key");
	T.AssertEquals("Pas assez d'or", HOLHeroBlockText::TooltipText(View), "Pas assez d'or");
	T.AssertEquals("ready price 150 no gold", View.Describe, "described");
}

// Ready, no forum to come back to: the button greyed, the price not red, "Aucun forum pour revenir" (before the gold).
void Test_HeroBlockReadyNoPlace(FUnitTest& T)
{
	const FHOLHeroBlockView View = HOLHeroBlock::Make(false, false, EHOLHeroState::ReadyToReturn, 1, 0, 450, -1,
		HeroBlockQuote(1759, EHOLHeroState::ReadyToReturn, 500, false));
	T.AssertTrue(View.bButtonVisible, "button shown");
	T.AssertFalse(View.bButtonEnabled, "button greyed");
	T.AssertFalse(View.bPriceRed, "price not red");
	T.AssertEquals("HeroBlock.NoPlace", View.TooltipKey, "the tooltip's key");
	T.AssertEquals("Aucun forum pour revenir", HOLHeroBlockText::TooltipText(View), "Aucun forum pour revenir");
	T.AssertEquals("ready price 150 no place", View.Describe, "described");
	const FHOLHeroBlockView Both = HOLHeroBlock::Make(false, false, EHOLHeroState::ReadyToReturn, 1, 0, 450, -1,
		HeroBlockQuote(1759, EHOLHeroState::ReadyToReturn, 10, false));
	T.AssertEquals("ready price 150 no place", Both.Describe, "no place said before no gold");
}

// Unknown (no death step or no clock here): dead and shown, nothing invented: "…", no button, no price.
void Test_HeroBlockUnknown(FUnitTest& T)
{
	const FHOLHeroBlockView View = HOLHeroBlock::Make(false, false, EHOLHeroState::Dead, 1, 0, 450, -1,
		HOLHeroReturnQuote::Compute(-1, EHOLHeroState::Dead, 1000, 1, 500, true, false));
	T.AssertTrue(View.bVisible, "visible");
	T.AssertTrue(View.bPortraitDark, "portrait dark");
	T.AssertEquals(0, View.Health, "0 HP");
	T.AssertEquals("HeroBlock.Unknown", View.DelayKey, "the unknown key");
	T.AssertEquals("…", HOLHeroBlockText::DelayText(View), "…");
	T.AssertFalse(View.bButtonVisible, "no button");
	T.AssertEquals(-1, View.Price, "no price");
	T.AssertEquals("unknown", View.Describe, "described");
}

// Hidden: a spectator, a defeated player, no hero (None).
void Test_HeroBlockHidden(FUnitTest& T)
{
	const FHOLHeroReturnQuote Alive = HOLHeroReturnQuote::Compute(500, EHOLHeroState::Alive, -1, 1, 500, true, false);
	const FHOLHeroBlockView Spectator = HOLHeroBlock::Make(true, false, EHOLHeroState::Alive, 1, 450, 450, -1, Alive);
	T.AssertFalse(Spectator.bVisible, "spectator: hidden");
	T.AssertEquals("hidden", Spectator.Describe, "spectator described");
	const FHOLHeroBlockView Defeated = HOLHeroBlock::Make(false, true, EHOLHeroState::Alive, 1, 450, 450, -1, Alive);
	T.AssertFalse(Defeated.bVisible, "defeated: hidden");
	T.AssertEquals("hidden", Defeated.Describe, "defeated described");
	const FHOLHeroBlockView NoHero = HOLHeroBlock::Make(false, false, EHOLHeroState::None, 0, 0, 450, -1,
		HOLHeroReturnQuote::Compute(500, EHOLHeroState::None, -1, 0, 500, true, false));
	T.AssertFalse(NoHero.bVisible, "None: hidden");
	T.AssertFalse(NoHero.bButtonVisible, "None: no button");
	T.AssertEquals("hidden", NoHero.Describe, "None described");
}

// The clock m:ss.
void Test_HeroBlockClock(FUnitTest& T)
{
	T.AssertEquals("0:00", HOLHeroBlock::Clock(0), "0:00");
	T.AssertEquals("0:09", HOLHeroBlock::Clock(9), "0:09");
	T.AssertEquals("1:00", HOLHeroBlock::Clock(60), "1:00");
	T.AssertEquals("12:05", HOLHeroBlock::Clock(725), "12:05");
}

// The portrait's emblem (J8a.5 loop 7): the texture imported by Tools/Editor/make_ui_icons.py loads by its path.
void Test_HeroBlockEmblem(FUnitTest& T)
{
	UTexture2D Emblem = HOLHeroBlock::LoadEmblem();
	T.AssertTrue(Emblem != nullptr, f"the emblem at {HOLHeroBlock::EmblemPath}");
	T.AssertEquals("T_Icon_HeroEmblem", Emblem != nullptr ? Emblem.GetName().ToString() : "", "named T_Icon_HeroEmblem");
}
