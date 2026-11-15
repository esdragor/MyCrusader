// Rebindable shortcuts, pure rule (B6; GDD 7.3 table of D30, "tous reconfigurables par le joueur", D453; GDD 15 command panel of D340;
// D245 the Commands tab greyed until M4). STUB of the red test: D30's table in data, but every change accepted.
struct FHOLKeyBindings
{
	// Action i is pressed by Keys[i] ("A", "F4", "Shift+1", "Ctrl+Shift+1"; read whatever its case).
	TArray<FString> Actions;
	TArray<FString> Keys;
}

namespace HOLRebind
{
	const int Accepted = 0;
	const int Conflict = 1;
	const int ReservedKey = 2;
	const int UnknownAction = 3;
	const int OutOfScope = 4;
	const int EmptyKey = 5;

	const int ScopeGrid = 0;
	const int ScopeAll = 1;
	const int Scope = ScopeAll;

	void Add(FHOLKeyBindings& B, const FString& Action, const FString& Key)
	{
		B.Actions.Add(Action);
		B.Keys.Add(Key);
	}

	FHOLKeyBindings Defaults()
	{
		FHOLKeyBindings B;
		const FString Grid = "AZERQSDFWXCV";
		for (int Cell = 0; Cell < 12; ++Cell)
		{
			Add(B, "Grid.Cell" + Cell, Grid.Mid(Cell, 1));
		}
		Add(B, "Select.MilitaryBuildings", "F1");
		Add(B, "Select.EconomicBuildings", "F2");
		Add(B, "Select.TechnologyBuildings", "F3");
		Add(B, "Select.Hero", "F4");
		Add(B, "Select.NextSubgroup", "Tab");
		Add(B, "Select.IdlePeasants", ".");
		Add(B, "Select.Army", "Ctrl+A");
		for (int Group = 1; Group <= 10; ++Group)
		{
			const FString Digit = "" + (Group % 10);
			Add(B, "Group.Select" + Digit, Digit);
			Add(B, "Group.Add" + Digit, "Shift+" + Digit);
			Add(B, "Group.Remove" + Digit, "Ctrl+Shift+" + Digit);
		}
		return B;
	}

	bool IsReserved(const FString& Key)
	{
		return false;
	}

	FString KeyOf(const FHOLKeyBindings& B, const FString& Action)
	{
		const int Index = B.Actions.FindIndex(Action);
		return Index >= 0 ? B.Keys[Index] : "";
	}

	FString ActionOf(const FHOLKeyBindings& B, const FString& Key)
	{
		for (int Index = 0; Index < B.Keys.Num(); ++Index)
		{
			if (B.Keys[Index].ToUpper() == Key.ToUpper())
			{
				return B.Actions[Index];
			}
		}
		return "";
	}

	int RebindIn(FHOLKeyBindings& B, const FString& Action, const FString& Key, int InScope)
	{
		const int Index = B.Actions.FindIndex(Action);
		if (Index >= 0)
		{
			B.Keys[Index] = Key;
		}
		return Accepted;
	}

	int Rebind(FHOLKeyBindings& B, const FString& Action, const FString& Key)
	{
		return RebindIn(B, Action, Key, Scope);
	}

	void RestoreDefaults(FHOLKeyBindings& B)
	{
	}
}
