// B6 (GDD 7.3 and 15, D30, D340): the rebindable shortcuts' pure rule, HOLRebind (Script\Input\HOLRebindRules.as). Every value asserted
// after a change is read back from the table the rule changed (ActionOf / KeyOf), never copied from the call's own arguments alone.

void Test_RebindDefaultsAreD30(FUnitTest& T)
{
	const FHOLKeyBindings D = HOLRebind::Defaults();
	T.AssertEquals(HOLRebind::KeyOf(D, "Grid.Cell0"), "A", "the grid's first cell (AZERTY, D340)");
	T.AssertEquals(HOLRebind::KeyOf(D, "Grid.Cell4"), "Q", "attack-move (D340)");
	T.AssertEquals(HOLRebind::KeyOf(D, "Grid.Cell5"), "S", "stop (D340)");
	T.AssertEquals(HOLRebind::KeyOf(D, "Grid.Cell11"), "V", "hold (D340)");
	T.AssertEquals(HOLRebind::KeyOf(D, "Select.Hero"), "F4", "the hero (D30)");
	T.AssertEquals(HOLRebind::KeyOf(D, "Group.Add1"), "Shift+1", "add to a group (D453: not Ctrl)");
	T.AssertEquals(HOLRebind::KeyOf(D, "Group.Remove0"), "Ctrl+Shift+0", "remove from a group");
	T.AssertEquals(D.Actions.Num(), 49, "12 cells, F1-F4, Tab, idle Peasants, the army, 3 x 10 groups");
	for (int Index = 0; Index < D.Actions.Num(); ++Index)
	{
		T.AssertEquals(HOLRebind::ActionOf(D, D.Keys[Index]), D.Actions[Index], "D30's table holds no key twice: " + D.Keys[Index]);
		T.AssertFalse(HOLRebind::IsReserved(D.Keys[Index]), "no default on a reserved key: " + D.Keys[Index]);
	}
}

void Test_RebindFreeKeyMovesTheAction(FUnitTest& T)
{
	FHOLKeyBindings B = HOLRebind::Defaults();
	T.AssertEquals(HOLRebind::Rebind(B, "Grid.Cell0", "P"), HOLRebind::Accepted, "P is free");
	T.AssertEquals(HOLRebind::ActionOf(B, "P"), "Grid.Cell0", "P now presses the first cell");
	T.AssertEquals(HOLRebind::ActionOf(B, "p"), "Grid.Cell0", "a key is read whatever its case");
	T.AssertEquals(HOLRebind::ActionOf(B, "A"), "", "A presses nothing any more");
	T.AssertEquals(HOLRebind::Rebind(B, "Grid.Cell1", "A"), HOLRebind::Accepted, "the freed key can be taken by another action");
	T.AssertEquals(HOLRebind::ActionOf(B, "A"), "Grid.Cell1", "A now presses the second cell");
	T.AssertEquals(HOLRebind::Rebind(B, "Select.Hero", "F4"), HOLRebind::Accepted, "an action back on its own key is no conflict");
}

void Test_RebindSameKeyTwoActionsRefused(FUnitTest& T)
{
	FHOLKeyBindings B = HOLRebind::Defaults();
	const FString Held = HOLRebind::KeyOf(B, "Grid.Cell1");
	T.AssertEquals(HOLRebind::Rebind(B, "Grid.Cell0", Held), HOLRebind::Conflict, "two actions on the same key: refused");
	T.AssertEquals(HOLRebind::ActionOf(B, Held), "Grid.Cell1", "the key still presses the action that held it");
	T.AssertEquals(HOLRebind::KeyOf(B, "Grid.Cell0"), HOLRebind::KeyOf(HOLRebind::Defaults(), "Grid.Cell0"), "the refused action keeps its key");
	T.AssertEquals(HOLRebind::Rebind(B, "Select.Army", "f4"), HOLRebind::Conflict, "the hero's key, whatever its case: refused");
	T.AssertEquals(HOLRebind::ActionOf(B, "Ctrl+A"), "Select.Army", "the army keeps Ctrl+A");
}

void Test_RebindReservedKeyRefused(FUnitTest& T)
{
	FHOLKeyBindings B = HOLRebind::Defaults();
	T.AssertEquals(HOLRebind::Rebind(B, "Grid.Cell0", "Escape"), HOLRebind::ReservedKey, "Escape is the game menu's");
	T.AssertEquals(HOLRebind::Rebind(B, "Select.Hero", "enter"), HOLRebind::ReservedKey, "Enter writes in the chat (D385)");
	T.AssertEquals(HOLRebind::ActionOf(B, "Escape"), "", "Escape presses no action");
	T.AssertEquals(HOLRebind::ActionOf(B, "F4"), "Select.Hero", "the hero keeps F4");
}

void Test_RebindUnknownActionOrEmptyKeyRefused(FUnitTest& T)
{
	FHOLKeyBindings B = HOLRebind::Defaults();
	T.AssertEquals(HOLRebind::Rebind(B, "Grid.Cell12", "P"), HOLRebind::UnknownAction, "the grid has 12 cells");
	T.AssertEquals(HOLRebind::ActionOf(B, "P"), "", "nothing took P");
	T.AssertEquals(HOLRebind::Rebind(B, "Grid.Cell0", ""), HOLRebind::EmptyKey, "an action needs a key");
	T.AssertEquals(HOLRebind::ActionOf(B, "A"), "Grid.Cell0", "the first cell keeps A");
}

void Test_RebindRestoreDefaults(FUnitTest& T)
{
	FHOLKeyBindings B = HOLRebind::Defaults();
	HOLRebind::Rebind(B, "Grid.Cell0", "P");
	HOLRebind::Rebind(B, "Select.Army", "Ctrl+Z");
	HOLRebind::RestoreDefaults(B);
	T.AssertEquals(HOLRebind::ActionOf(B, "A"), "Grid.Cell0", "A presses the first cell again");
	T.AssertEquals(HOLRebind::ActionOf(B, "P"), "", "P presses nothing again");
	T.AssertEquals(HOLRebind::ActionOf(B, "Ctrl+A"), "Select.Army", "Ctrl+A selects the army again");
	T.AssertEquals(HOLRebind::ActionOf(B, "Ctrl+Z"), "", "Ctrl+Z presses nothing again");
	T.AssertEquals(B.Actions.Num(), HOLRebind::Defaults().Actions.Num(), "the whole table is back");
}

void Test_RebindGridScopeLocksTheRest(FUnitTest& T)
{
	FHOLKeyBindings B = HOLRebind::Defaults();
	T.AssertEquals(HOLRebind::RebindIn(B, "Select.Hero", "P", HOLRebind::ScopeGrid), HOLRebind::OutOfScope, "grid only: F4 stays");
	T.AssertEquals(HOLRebind::ActionOf(B, "F4"), "Select.Hero", "the hero keeps F4");
	T.AssertEquals(HOLRebind::RebindIn(B, "Grid.Cell0", "P", HOLRebind::ScopeGrid), HOLRebind::Accepted, "grid only: a cell can move");
	T.AssertEquals(HOLRebind::ActionOf(B, "P"), "Grid.Cell0", "P presses the first cell");
}
