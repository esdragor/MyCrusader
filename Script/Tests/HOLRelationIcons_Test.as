// E3-10 (D830, D836, D839): script port of the CHOICE half of HOLRelationIconsTests.cpp (HeroesOfLiberty.UI.RelationIcons.Choice),
// read through UHOLRelationIconsReadLibrary (HOLRelationIconsRead::, wrappers over HOLRelationIcons::GetIconPath, the C++ rule).
// An ally's row gets the green shield, an enemy's the red swords, the viewer's own row and a spectator's view nothing.
// The texture half (both textures exist, 64 px wide, UI texture group) stays in the C++ test: it needs UTexture2D::GetImportedSize.
void Test_RelationIconsChoice(FUnitTest& T)
{
	// The expected paths are written here, not read back from AllyPath / EnemyPath: the rule's output is compared to the HUD's real assets.
	T.AssertEquals(HOLRelationIconsRead::GetIconPath(HOLRelationIconsRead::GetRelationAlly()), "/Game/UI/Icons/T_Icon_Ally.T_Icon_Ally", "ally: the green shield");
	T.AssertEquals(HOLRelationIconsRead::GetIconPath(HOLRelationIconsRead::GetRelationEnemy()), "/Game/UI/Icons/T_Icon_Enemy.T_Icon_Enemy", "enemy: the red swords");
	T.AssertEquals(HOLRelationIconsRead::GetIconPath(HOLRelationIconsRead::GetRelationSelf()), "", "self: no icon");
	T.AssertEquals(HOLRelationIconsRead::GetIconPath(HOLRelationIconsRead::GetRelationNone()), "", "none (spectator): no icon");

	// The path the rule returns is the one the HUD loads (AllyPath / EnemyPath), and the two icons differ.
	T.AssertEquals(HOLRelationIconsRead::GetIconPath(HOLRelationIconsRead::GetRelationAlly()), HOLRelationIconsRead::GetAllyPath(), "ally: AllyPath");
	T.AssertEquals(HOLRelationIconsRead::GetIconPath(HOLRelationIconsRead::GetRelationEnemy()), HOLRelationIconsRead::GetEnemyPath(), "enemy: EnemyPath");
	T.AssertTrue(HOLRelationIconsRead::GetAllyPath() != HOLRelationIconsRead::GetEnemyPath(), "ally and enemy have different icons");

	// The four relations are distinct numbers (the int32 crossing the binding is not collapsed).
	T.AssertTrue(HOLRelationIconsRead::GetRelationAlly() != HOLRelationIconsRead::GetRelationEnemy(), "ally != enemy");
	T.AssertTrue(HOLRelationIconsRead::GetRelationSelf() != HOLRelationIconsRead::GetRelationNone(), "self != none");
}
