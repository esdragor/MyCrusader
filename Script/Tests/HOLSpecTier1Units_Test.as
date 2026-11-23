// The three units of tier 1 of the Templars' commandery lanes (D893, GDD 13.7 grid of D133), PROVISIONAL data rows only (their unlock in the
// building, by the chosen lane, is the next line of the queue): Iron = Sergent du Temple, Faith = Frere infirmier, Stone = Arbaletrier a pavois.
// Row names: TempleSergeant, InfirmaryBrother, PaviseCrossbowman (the display names are the GDD's). The lane is not a column of
// Tools/Data/UnitTypes.csv (no C++ on this line, Retours/Q-commanderie-palier1.md): the lane of each row is the literal below, from the GDD.
// The test reads the real table: the figures through the AI reader (the row the combat code reads), the faction column from the CSV text.
// RED until the three rows are in Tools/Data/UnitTypes.csv and Content/Units/UnitTypes.uasset is regenerated (regen_tables.ps1).

// The row of a type in the CSV text: the line that starts with "<Name>,". Empty when absent.
FString SpecTier1RowText(const FString& Text, const FString& TypeName)
{
	TArray<FString> Lines;
	Text.ParseIntoArray(Lines, "\n");
	for (int Line = 0; Line < Lines.Num(); Line++)
	{
		if (Lines[Line].StartsWith(TypeName + ","))
		{
			return Lines[Line];
		}
	}
	return "";
}

// One row per lane (Iron, Faith, Stone): it exists in the table, with a cost and health above zero, and it belongs to the Templars alone.
void Test_SpecTier1UnitsExist(FUnitTest& T)
{
	TArray<FString> Rows;
	TArray<FString> Lanes;
	TArray<FString> DisplayNames;
	Rows.Add("TempleSergeant");
	Lanes.Add("Iron");
	DisplayNames.Add("Sergent du Temple");
	Rows.Add("InfirmaryBrother");
	Lanes.Add("Faith");
	DisplayNames.Add("Frère infirmier");
	Rows.Add("PaviseCrossbowman");
	Lanes.Add("Stone");
	DisplayNames.Add("Arbalétrier à pavois");

	FString UnitText;
	const bool bRead = HOLMatchConfigRead::LoadProjectFileText("Tools/Data/UnitTypes.csv", UnitText);
	T.AssertTrue(bRead, "Tools/Data/UnitTypes.csv is there");
	if (!bRead)
	{
		return;
	}
	T.AssertEquals(3, Rows.Num(), "one tier 1 unit per lane");
	for (int Row = 0; Row < Rows.Num(); Row++)
	{
		const FString Label = Lanes[Row] + " lane, " + Rows[Row];
		FHOLAITypeFigures Unit;
		const bool bKnown = HOLAIOrder::ReadAIUnitFigures(FName(Rows[Row]), Unit);
		T.AssertTrue(bKnown, Label + ": a row of the unit type table");
		if (bKnown)
		{
			T.AssertTrue(Unit.Health > 0, Label + ": health above zero");
			T.AssertTrue(Unit.CostFood + Unit.CostWood + Unit.CostStone + Unit.CostGold > 0, Label + ": a cost above zero");
			T.AssertTrue(Unit.Population > 0, Label + ": takes a population place");
		}
		const FString RowText = SpecTier1RowText(UnitText, Rows[Row]);
		T.AssertTrue(RowText.Len() > 0, Label + ": the line is in UnitTypes.csv");
		T.AssertTrue(RowText.Contains("," + DisplayNames[Row] + ","), Label + ": the GDD display name " + DisplayNames[Row]);
		T.AssertTrue(RowText.Contains(",Templars,"), Label + ": the faction column is the Templars alone");
	}
}
