// Rebindable shortcuts, pure rule (B6; GDD 7.3 table of D30, "tous reconfigurables par le joueur", D453; GDD 15 command panel of D340;
// D245 the Commands tab greyed until M4). A table action -> key in data (D30's defaults, AZERTY) and the rule of a change: an unknown
// action, an action out of the scope, an empty key, a reserved key (Escape, Enter) or a key another action holds is refused, the table
// untouched; RestoreDefaults gives D30's table back. No world access, tested in Script\Tests\HOLRebindRules_Test.as. Not here (C++, M4):
// the Commands tab (greyed, HOLSettingsScreen.cpp), Enhanced Input (HOLUIInputData.h), the grid's keys (HOLCommandGrid), saving the table.
// Retours\Q-rebind.md asks when to open the tab and for which shortcuts; default A = Scope ScopeAll (one line to change below).
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

	// Keys no action may take: Escape opens the game menu (HOLGameMenuRules::GetEscapeAction), Enter writes in the chat (D385).
	bool IsReserved(const FString& Key)
	{
		const FString Upper = Key.ToUpper();
		return Upper == "ESCAPE" || Upper == "ENTER";
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

	// An action of the command grid (D340), the only ones a ScopeGrid table lets the player move.
	bool IsGridAction(const FString& Action)
	{
		return Action.Find("Grid.") == 0;
	}

	// What a change of Action to Key would give, the table untouched: the first refusal met, or Accepted.
	int Check(const FHOLKeyBindings& B, const FString& Action, const FString& Key, int InScope)
	{
		if (B.Actions.FindIndex(Action) < 0)
		{
			return UnknownAction;
		}
		if (InScope == ScopeGrid && !IsGridAction(Action))
		{
			return OutOfScope;
		}
		if (Key.IsEmpty())
		{
			return EmptyKey;
		}
		if (IsReserved(Key))
		{
			return ReservedKey;
		}
		const FString Holder = ActionOf(B, Key);
		if (!Holder.IsEmpty() && Holder != Action)
		{
			// Two actions on the same key: refused, never a silent swap (the player frees the key first).
			return Conflict;
		}
		return Accepted;
	}

	// Moves Action to Key when Check accepts it; the table is left as it was on any refusal.
	int RebindIn(FHOLKeyBindings& B, const FString& Action, const FString& Key, int InScope)
	{
		const int Result = Check(B, Action, Key, InScope);
		if (Result == Accepted)
		{
			B.Keys[B.Actions.FindIndex(Action)] = Key;
		}
		return Result;
	}

	int Rebind(FHOLKeyBindings& B, const FString& Action, const FString& Key)
	{
		return RebindIn(B, Action, Key, Scope);
	}

	// The Commands tab's "Valeurs par défaut": D30's whole table back.
	void RestoreDefaults(FHOLKeyBindings& B)
	{
		B = Defaults();
	}
}
