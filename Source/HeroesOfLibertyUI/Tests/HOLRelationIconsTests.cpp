#include "Engine/Texture2D.h"
#include "HUD/HOLRelationIcons.h"
#include "Match/HOLForumRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// E4-1: an ally's row gets the green shield, an enemy's the red swords, the viewer's own row and a spectator's view
// nothing; both textures exist where the HUD loads them.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRelationIconsTest, "HeroesOfLiberty.UI.RelationIcons.Choice",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLRelationIconsTest::RunTest(const FString& Parameters)
{
	const auto PathOf = [](const EHOLPlayerRelation Relation)
	{
		const TCHAR* Path = HOLRelationIcons::GetIconPath(Relation);
		return Path ? FString(Path) : FString();
	};
	TestEqual(TEXT("ally"), PathOf(EHOLPlayerRelation::Ally), FString(TEXT("/Game/UI/Icons/T_Icon_Ally.T_Icon_Ally")));
	TestEqual(TEXT("enemy"), PathOf(EHOLPlayerRelation::Enemy), FString(TEXT("/Game/UI/Icons/T_Icon_Enemy.T_Icon_Enemy")));
	TestEqual(TEXT("self"), PathOf(EHOLPlayerRelation::Self), FString());
	TestEqual(TEXT("none"), PathOf(EHOLPlayerRelation::None), FString());

	for (const TCHAR* Path : { HOLRelationIcons::AllyPath, HOLRelationIcons::EnemyPath })
	{
		const UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, Path);
		if (!TestNotNull(FString::Printf(TEXT("texture %s"), Path), Texture))
		{
			continue;
		}
		// The imported size: GetSizeX() is 0 without a renderer (-nullrhi).
		TestEqual(FString::Printf(TEXT("%s width"), Path), Texture->GetImportedSize().X, 64);
		TestEqual(FString::Printf(TEXT("%s UI group"), Path), static_cast<int32>(Texture->LODGroup), static_cast<int32>(TEXTUREGROUP_UI));
	}
	return true;
}

#endif
