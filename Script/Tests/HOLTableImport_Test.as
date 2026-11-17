// The tables of Tools/Data read through the editor's own import (UDataTable::CreateTableFromCSVString, UHOLTableImportReadLibrary, namespace
// HOLTableImportRead): the script twin of the C++ tests Market.Table, Market.Rules, Siege.Technologies and of what Technologies reads
// from Technologies.csv (GLORFINDEL-1117k, registry in Docs/Rapports/GLORFINDEL-1117k.md). The C++ tests stay until these are seen red then green.
// A field is the text the reflection exports (a number as written, a list as "(A,B)", a bool as "True" / "False").

// A whole non-negative number from a field's text; bOk false if it is empty or has another character.
int TableImportInt(const FString& Text, bool& bOk)
{
	FString Clean = Text.TrimStartAndEnd();
	bOk = Clean.Len() > 0;
	int Value = 0;
	for (int Index = 0; Index < Clean.Len(); Index++)
	{
		int Digit = Clean[Index] - 48;
		if (Digit < 0 || Digit > 9)
		{
			bOk = false;
			break;
		}
		Value = Value * 10 + Digit;
	}
	return Value;
}

// The index of a name in an array of texts, -1 if it is not there.
int TableImportIndex(const TArray<FString>& Names, const FString& Name)
{
	for (int Index = 0; Index < Names.Num(); Index++)
	{
		if (Names[Index] == Name)
		{
			return Index;
		}
	}
	return -1;
}

// The text of a field of a row, empty (and bFound false) if the row or the column is missing.
FString TableImportField(const TArray<FString>& RowNames, const TArray<FString>& Columns, const TArray<FString>& Values, const FString& Row, const FString& Column, bool& bFound)
{
	int RowIndex = TableImportIndex(RowNames, Row);
	int ColumnIndex = TableImportIndex(Columns, Column);
	bFound = RowIndex >= 0 && ColumnIndex >= 0 && RowIndex * Columns.Num() + ColumnIndex < Values.Num();
	return bFound ? Values[RowIndex * Columns.Num() + ColumnIndex] : FString();
}

// The whole number of a field; -1 if the field is missing or not a number.
int TableImportNumber(const TArray<FString>& RowNames, const TArray<FString>& Columns, const TArray<FString>& Values, const FString& Row, const FString& Column)
{
	bool bFound = false;
	FString Text = TableImportField(RowNames, Columns, Values, Row, Column, bFound);
	bool bOk = false;
	int Value = TableImportInt(Text, bOk);
	return (bFound && bOk) ? Value : -1;
}

// Market.Table: the import reports no problem, the Standard row has the numbers of MK-Q1 = C (D582), every row is coherent.
void Test_TableImportMarketRows(FUnitTest& T)
{
	TArray<FString> RowNames;
	TArray<FString> Columns;
	TArray<FString> Values;
	TArray<FString> Problems;
	T.AssertTrue(HOLTableImportRead::ImportTable("Market", "Market.csv", RowNames, Columns, Values, Problems), "Market.csv imports through CreateTableFromCSVString");
	T.AssertEquals(0, Problems.Num(), "Market.csv: no import problem");
	T.AssertEquals(10, Columns.Num(), "the 10 columns of FHOLMarketRow");
	T.AssertEquals(RowNames.Num() * Columns.Num(), Values.Num(), "one value per row and column");
	T.AssertTrue(TableImportIndex(RowNames, "Standard") >= 0, "the Standard row is in the table");

	// 1. The numbers of MK-Q1 = C (D582).
	T.AssertEquals(80, TableImportNumber(RowNames, Columns, Values, "Standard", "FoodPrice"), "food base price");
	T.AssertEquals(80, TableImportNumber(RowNames, Columns, Values, "Standard", "WoodPrice"), "wood base price");
	T.AssertEquals(120, TableImportNumber(RowNames, Columns, Values, "Standard", "StonePrice"), "stone base price");
	T.AssertEquals(100, TableImportNumber(RowNames, Columns, Values, "Standard", "GoldPrice"), "gold base price");
	T.AssertEquals(30, TableImportNumber(RowNames, Columns, Values, "Standard", "FeePercent"), "tax in percent");
	T.AssertEquals(3, TableImportNumber(RowNames, Columns, Values, "Standard", "Step"), "points a lot");
	T.AssertEquals(1, TableImportNumber(RowNames, Columns, Values, "Standard", "RecoverPoints"), "points back");
	T.AssertEquals(10, TableImportNumber(RowNames, Columns, Values, "Standard", "RecoverEverySeconds"), "seconds between two returns");
	T.AssertEquals(20, TableImportNumber(RowNames, Columns, Values, "Standard", "MinPrice"), "floor");
	T.AssertEquals(300, TableImportNumber(RowNames, Columns, Values, "Standard", "MaxPrice"), "cap");

	// 2. Coherence, whatever the numbers become: bounds in order, every base price inside them, a real tax and step.
	for (int Index = 0; Index < RowNames.Num(); Index++)
	{
		FString Name = RowNames[Index];
		int Floor = TableImportNumber(RowNames, Columns, Values, Name, "MinPrice");
		int Cap = TableImportNumber(RowNames, Columns, Values, Name, "MaxPrice");
		T.AssertTrue(Floor >= 0 && Floor < Cap, f"{Name}: floor below cap");
		TArray<FString> Bases;
		Bases.Add("FoodPrice");
		Bases.Add("WoodPrice");
		Bases.Add("StonePrice");
		Bases.Add("GoldPrice");
		for (int Base = 0; Base < Bases.Num(); Base++)
		{
			int Price = TableImportNumber(RowNames, Columns, Values, Name, Bases[Base]);
			T.AssertTrue(Price >= Floor && Price <= Cap, f"{Name}: {Bases[Base]} {Price} within the bounds");
		}
		int Fee = TableImportNumber(RowNames, Columns, Values, Name, "FeePercent");
		T.AssertTrue(Fee >= 0 && Fee < 100, f"{Name}: tax between 0 and 99 percent");
		T.AssertTrue(TableImportNumber(RowNames, Columns, Values, Name, "Step") > 0, f"{Name}: the price moves with a lot");
		T.AssertTrue(TableImportNumber(RowNames, Columns, Values, Name, "RecoverPoints") > 0 && TableImportNumber(RowNames, Columns, Values, Name, "RecoverEverySeconds") > 0, f"{Name}: the price comes back");
	}
}

// Market.Rules: the settings the game reads from the imported Standard row (HOLMarket::SettingsFromRow), then the exchange of the script's rule on them.
void Test_TableImportMarketSettings(FUnitTest& T)
{
	FHOLMarketSettings S;
	T.AssertTrue(HOLTableImportRead::MarketSettingsFromRow("Standard", S.BasePrices, S.Step, S.FeePercent, S.MinPrice, S.MaxPrice, S.RecoverPoints, S.RecoverEverySeconds), "the Standard row of Market.csv gives settings");
	T.AssertEquals(4, S.BasePrices.Num(), "one base price per resource");
	if (S.BasePrices.Num() != 4)
	{
		return;
	}
	T.AssertEquals(80, S.BasePrices[0], "settings: food base");
	T.AssertEquals(80, S.BasePrices[1], "settings: wood base");
	T.AssertEquals(120, S.BasePrices[2], "settings: stone base");
	T.AssertEquals(100, S.BasePrices[3], "settings: gold base");
	T.AssertEquals(30, S.FeePercent, "settings: tax");
	T.AssertEquals(3, S.Step, "settings: step");
	T.AssertEquals(20, S.MinPrice, "settings: floor");
	T.AssertEquals(300, S.MaxPrice, "settings: cap");
	T.AssertEquals(1, S.RecoverPoints, "settings: points back");
	T.AssertEquals(10, S.RecoverEverySeconds, "settings: seconds between two returns");

	TArray<int> NoBases;
	int A = 0;
	T.AssertFalse(HOLTableImportRead::MarketSettingsFromRow("NoSuchRow", NoBases, A, A, A, A, A, A), "an unknown row gives no settings");

	const int Wood = 1;
	const int Stone = 2;
	const int Gold = 3;
	FHOLMarketBook Book = HOLMarket::NewBook(S);
	// Wood sold at 80 pays 56 and falls to 77.
	FHOLMarketTrade Sold = HOLMarket::Sell(S, Book, Wood, 1, 100, S.FeePercent);
	T.AssertTrue(Sold.bOk, "a lot of wood sold");
	T.AssertEquals(56, Sold.Gold, "it pays 56");
	T.AssertEquals(100, Sold.Units, "100 wood leave the stock");
	T.AssertEquals(77, Sold.Book.Prices[Wood], "wood falls to 77");
	T.AssertEquals(80, Book.Prices[Wood], "the book given is not changed");
	T.AssertEquals(84, HOLMarket::Sell(S, Book, Stone, 1, 100, S.FeePercent).Gold, "a lot of stone pays 84");

	// Lot by lot: three lots of wood at 80, 77, 74.
	FHOLMarketTrade Three = HOLMarket::Sell(S, Book, Wood, 3, 300, S.FeePercent);
	T.AssertEquals(56 + 53 + 51, Three.Gold, "three lots, each at the price left by the one before");
	T.AssertEquals(71, Three.Book.Prices[Wood], "three lots: wood at 71");

	// The refusals change nothing.
	T.AssertEquals("lots", HOLMarket::Sell(S, Book, Wood, 0, 500, S.FeePercent).Reason, "zero lots");
	T.AssertEquals("resource", HOLMarket::Sell(S, Book, Gold, 1, 500, S.FeePercent).Reason, "gold is not traded");
	T.AssertEquals("stock", HOLMarket::Sell(S, Book, Wood, 2, 150, S.FeePercent).Reason, "two lots from 150");

	// A purchase by lots of 100 gold, each lot at the price the one before left.
	FHOLMarketTrade Gold1 = HOLMarket::BuyWithGold(S, Book, Wood, 1, 1000, S.FeePercent);
	T.AssertTrue(Gold1.bOk, "100 gold spent on wood");
	T.AssertEquals(100, Gold1.Gold, "it spends 100 gold");
	T.AssertEquals(96, Gold1.Units, "it brings 96 wood (80 plus 30 % = 104 per 100 wood)");
	T.AssertEquals(83, Gold1.Book.Prices[Wood], "wood rises to 83");
	FHOLMarketTrade Gold2 = HOLMarket::BuyWithGold(S, Book, Wood, 2, 1000, S.FeePercent);
	T.AssertEquals(200, Gold2.Gold, "two lots of gold: 200 gold");
	T.AssertEquals(96 + 92, Gold2.Units, "two lots of gold: 96 + 92 wood (83 plus 30 % = 108)");
	T.AssertEquals(86, Gold2.Book.Prices[Wood], "two lots of gold: wood at 86");
	T.AssertEquals(64, HOLMarket::BuyWithGold(S, Book, Stone, 1, 1000, S.FeePercent).Units, "100 gold on stone brings 64");
	T.AssertEquals("gold", HOLMarket::BuyWithGold(S, Book, Wood, 1, 99, S.FeePercent).Reason, "with 99 gold: refused (gold)");
	T.AssertEquals("resource", HOLMarket::BuyWithGold(S, Book, Gold, 1, 1000, S.FeePercent).Reason, "gold is not bought with gold");

	// The bounds.
	FHOLMarketBook Low = Book;
	Low.Prices[Wood] = S.MinPrice + 1;
	T.AssertEquals(S.MinPrice, HOLMarket::Sell(S, Low, Wood, 3, 1000, S.FeePercent).Book.Prices[Wood], "the price stops at the floor");
	FHOLMarketBook High = Book;
	High.Prices[Wood] = S.MaxPrice - 1;
	T.AssertEquals(S.MaxPrice, HOLMarket::BuyWithGold(S, High, Wood, 3, 100000, S.FeePercent).Book.Prices[Wood], "the price stops at the cap");
}

// Siege.Technologies: the technologies researched at the siege workshop open at the tier of the engine they improve (GDD 11.1), the workshop is a real
// building type that produces that engine, and the statistic each changes is the one the engine reads.
void Test_TableImportSiegeTechnologies(FUnitTest& T)
{
	TArray<FString> TechNames;
	TArray<FString> TechColumns;
	TArray<FString> TechValues;
	TArray<FString> Problems;
	T.AssertTrue(HOLTableImportRead::ImportTable("Technology", "Technologies.csv", TechNames, TechColumns, TechValues, Problems), "Technologies.csv imports through CreateTableFromCSVString");
	T.AssertEquals(0, Problems.Num(), "Technologies.csv: no import problem");
	TArray<FString> BuildingNames;
	TArray<FString> BuildingColumns;
	TArray<FString> BuildingValues;
	T.AssertTrue(HOLTableImportRead::ImportTable("BuildingType", "BuildingTypes.csv", BuildingNames, BuildingColumns, BuildingValues, Problems), "BuildingTypes.csv imports through CreateTableFromCSVString");
	T.AssertEquals(0, Problems.Num(), "BuildingTypes.csv: no import problem");

	// Two technologies at the siege workshop (GDD 11.1).
	int AtTheWorkshop = 0;
	for (int Index = 0; Index < TechNames.Num(); Index++)
	{
		bool bFound = false;
		if (TableImportField(TechNames, TechColumns, TechValues, TechNames[Index], "Building", bFound) == "SiegeWorkshop")
		{
			AtTheWorkshop++;
		}
	}
	T.AssertEquals(2, AtTheWorkshop, "two technologies at the siege workshop (GDD 11.1)");

	// Which engine each improves (Reinforced Ram -> Ram, Heavy Munitions -> Mangonel).
	TArray<FString> Technologies;
	Technologies.Add("ReinforcedRam");
	Technologies.Add("HeavyMunitions");
	TArray<int> Engines;
	Engines.Add(int(EHOLSiegeEngine::Ram));
	Engines.Add(int(EHOLSiegeEngine::Mangonel));
	TArray<FString> EngineNames;
	EngineNames.Add("Ram");
	EngineNames.Add("Mangonel");
	for (int Line = 0; Line < Technologies.Num(); Line++)
	{
		FString Tech = Technologies[Line];
		T.AssertTrue(TableImportIndex(TechNames, Tech) >= 0, f"{Tech} is in the table");
		int MinTier = TableImportNumber(TechNames, TechColumns, TechValues, Tech, "MinTier");
		T.AssertEquals(HOLSiegeCheck::GetTier(Engines[Line]), MinTier, f"{Tech} opens at the tier of the engine it improves");
		T.AssertTrue(HOLSiegeCheck::InPrototype(Engines[Line]), f"{Tech} improves an engine of the prototype");

		// S2: the building the technology names is a real type, and the technology is not older than it.
		bool bFound = false;
		FString Building = TableImportField(TechNames, TechColumns, TechValues, Tech, "Building", bFound);
		T.AssertTrue(TableImportIndex(BuildingNames, Building) >= 0, f"{Tech}: its building {Building} is in the building type table");
		int Age = TableImportNumber(BuildingNames, BuildingColumns, BuildingValues, Building, "Age");
		T.AssertTrue(Age >= 0 && MinTier >= Age, f"{Tech}: not researched before its building exists (tier {MinTier}, building age {Age})");
		FString Produces = TableImportField(BuildingNames, BuildingColumns, BuildingValues, Building, "Produces", bFound);
		T.AssertTrue(bFound && Produces.Contains(EngineNames[Line]), f"{Tech}: the workshop produces the engine it improves ({Produces})");
	}

	// S5: the statistic each of the two technologies changes is the one the engine reads (not a name typed twice).
	T.AssertEquals("RamHealth", HOLSiegeCheck::GetHealthStat(n"Ram").ToString(), "the Ram reads RamHealth");
	T.AssertEquals("MangonelDamage", HOLSiegeCheck::GetDamageStat(n"Mangonel").ToString(), "the Mangonel reads MangonelDamage");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Mangonel").IsNone(), "the Mangonel's health has no technology");
	T.AssertTrue(HOLSiegeCheck::GetDamageStat(n"Ram").IsNone(), "the Ram's damage has no technology");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Spearman").IsNone(), "a Spearman has none");
	bool bFoundStats = false;
	FString RamStats = TableImportField(TechNames, TechColumns, TechValues, "ReinforcedRam", "Stats", bFoundStats);
	T.AssertTrue(bFoundStats && RamStats.Contains(HOLSiegeCheck::GetHealthStat(n"Ram").ToString()), f"Reinforced Ram changes what the Ram reads ({RamStats})");
	FString MangonelStats = TableImportField(TechNames, TechColumns, TechValues, "HeavyMunitions", "Stats", bFoundStats);
	T.AssertTrue(bFoundStats && MangonelStats.Contains(HOLSiegeCheck::GetDamageStat(n"Mangonel").ToString()), f"Heavy Munitions changes what the Mangonel reads ({MangonelStats})");
}

// Technologies: the rows the rules read (the farm's plough, level I then II) come out of the import with the fields the rules test needs.
void Test_TableImportTechnologyRows(FUnitTest& T)
{
	TArray<FString> RowNames;
	TArray<FString> Columns;
	TArray<FString> Values;
	TArray<FString> Problems;
	T.AssertTrue(HOLTableImportRead::ImportTable("Technology", "Technologies.csv", RowNames, Columns, Values, Problems), "Technologies.csv imports through CreateTableFromCSVString");
	T.AssertEquals(0, Problems.Num(), "no import problem");
	bool bFound = false;
	T.AssertEquals("Farm", TableImportField(RowNames, Columns, Values, "PloughI", "Building", bFound), "PloughI is researched at the farm");
	T.AssertEquals(1, TableImportNumber(RowNames, Columns, Values, "PloughI", "MinTier"), "PloughI opens at tier 1");
	T.AssertEquals(0, TableImportNumber(RowNames, Columns, Values, "PloughI", "MinHeroLevel"), "PloughI: the tier alone decides (MinHeroLevel 0)");
	T.AssertEquals("", TableImportField(RowNames, Columns, Values, "PloughI", "Requires", bFound), "PloughI requires nothing");
	T.AssertEquals(2, TableImportNumber(RowNames, Columns, Values, "PloughII", "MinTier"), "PloughII opens at tier 2");
	T.AssertEquals("PloughI", TableImportField(RowNames, Columns, Values, "PloughII", "Requires", bFound), "PloughII requires PloughI");
	T.AssertEquals("True", TableImportField(RowNames, Columns, Values, "PloughI", "Implemented", bFound), "PloughI is implemented");
	T.AssertEquals("False", TableImportField(RowNames, Columns, Values, "ReinforcedRam", "Implemented", bFound), "ReinforcedRam is not implemented yet (Soon)");
}
