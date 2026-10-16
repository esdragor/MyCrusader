// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Presentation/HOLUnitAnimation.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J5c.1, J5c.2 and J6a.4 checks: the animation and the prop a client shows on a unit (D234, HOLUnitAnimation::Choose) and
// the Peasant's sequences and props in the type table and its animation provider.
namespace UE::HeroesOfLiberty::Tests::Animation
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The Peasant's row as Tools/Data/UnitTypes.csv has it, written here so the rule is tested on its own. */
	FHOLUnitTypeConstants MakePeasant()
	{
		FHOLUnitTypeConstants Type;
		Type.Speed = 600.f;
		Type.MeshScale = 1.5f;
		Type.IdleSequence = 0;
		Type.WalkSequence = 1;
		Type.WalkPlayRate = 2.44f;
		Type.GatherSequences.Food = 4;
		Type.GatherSequences.Wood = 2;
		Type.GatherSequences.Stone = 3;
		Type.GatherSequences.Gold = 3;
		Type.GatherPlayRate = 1.f;
		Type.CarryWalkSequences.Food = 5;
		Type.CarryWalkSequences.Wood = 6;
		Type.CarryWalkSequences.Stone = 7;
		Type.CarryWalkSequences.Gold = 7;
		Type.CarryWalkPlayRate = 3.57f;
		Type.CarryIdleSequences.Food = 8;
		Type.CarryIdleSequences.Wood = 9;
		Type.CarryIdleSequences.Stone = 10;
		Type.CarryIdleSequences.Gold = 10;
		Type.GatherProps.Food = 0;
		Type.GatherProps.Wood = 1;
		Type.GatherProps.Stone = 2;
		Type.GatherProps.Gold = 2;
		Type.CarryProps.Food = 3;
		Type.CarryProps.Wood = 4;
		Type.CarryProps.Stone = 5;
		Type.CarryProps.Gold = 6;
		Type.BuildSequence = 11;
		Type.BuildPlayRate = 1.f;
		Type.BuildProp = 7;
		return Type;
	}

	FHOLUnitAnimationInput MakeInput(const EHOLUnitActivity Activity, const float Speed, const int32 PlayingSequence = INDEX_NONE)
	{
		FHOLUnitAnimationInput Input;
		Input.Activity = Activity;
		Input.Speed = Speed;
		Input.PlayingSequence = PlayingSequence;
		return Input;
	}

	FHOLUnitAnimationInput WithLoad(FHOLUnitAnimationInput Input, const EHOLResource Resource)
	{
		Input.bCarrying = true;
		Input.LoadResource = Resource;
		return Input;
	}

	FHOLUnitAnimationInput Gathering(const EHOLResource Resource, const float Speed = 0.f, const int32 PlayingSequence = INDEX_NONE)
	{
		FHOLUnitAnimationInput Input = MakeInput(EHOLUnitActivity::Gathering, Speed, PlayingSequence);
		Input.GatherResource = Resource;
		return Input;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAnimationChoiceTest, "HeroesOfLiberty.Animation.Choice", UE::HeroesOfLiberty::Tests::Animation::Flags)
bool FHOLAnimationChoiceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Animation;
	using HOLUnitAnimation::Choose;
	const FHOLUnitTypeConstants Peasant = MakePeasant();
	const float Walking = Peasant.Speed;
	const auto Sequence = [&Peasant](const FHOLUnitAnimationInput& Input) { return Choose(Peasant, Input).Sequence; };
	const auto Prop = [&Peasant](const FHOLUnitAnimationInput& Input) { return static_cast<int32>(Choose(Peasant, Input).Prop); };

	// Idle and walk as before (D234), the walk slowed by the mesh scale.
	const FHOLUnitAnimationChoice Idle = Choose(Peasant, MakeInput(EHOLUnitActivity::Idle, 0.f));
	TestEqual(TEXT("standing: idle"), Idle.Sequence, 0);
	TestEqual(TEXT("idle at its own rate"), Idle.PlayRate, 1.f);
	TestFalse(TEXT("idle: its own orientation"), Idle.bFacesSource);
	TestEqual(TEXT("idle: no prop"), static_cast<int32>(Idle.Prop), 0);
	const FHOLUnitAnimationChoice Walk = Choose(Peasant, MakeInput(EHOLUnitActivity::Moving, Walking));
	TestEqual(TEXT("walking: walk"), Walk.Sequence, 1);
	TestEqual(TEXT("walk rate: WalkPlayRate / MeshScale"), Walk.PlayRate, 2.44f / 1.5f, UE_KINDA_SMALL_NUMBER);
	TestEqual(TEXT("walk: no prop"), static_cast<int32>(Walk.Prop), 0);

	// Gathering: the resource's gesture, whatever the speed or the load.
	const FHOLUnitAnimationChoice Chop = Choose(Peasant, Gathering(EHOLResource::Wood));
	TestEqual(TEXT("gathering wood: the axe"), Chop.Sequence, 2);
	TestEqual(TEXT("a gesture is not slowed by the mesh scale"), Chop.PlayRate, 1.f);
	TestTrue(TEXT("gathering: faces its source"), Chop.bFacesSource);
	TestEqual(TEXT("gathering stone: the pick"), Sequence(Gathering(EHOLResource::Stone)), 3);
	TestEqual(TEXT("gathering gold: the pick"), Sequence(Gathering(EHOLResource::Gold)), 3);
	TestEqual(TEXT("gathering food: the picking of berries"), Sequence(Gathering(EHOLResource::Food)), 4);
	TestEqual(TEXT("pushed aside while gathering: keeps chopping"), Sequence(Gathering(EHOLResource::Wood, Walking, 2)), 2);
	TestEqual(TEXT("gathering wood with a load of food (D306): the axe"), Sequence(WithLoad(Gathering(EHOLResource::Wood), EHOLResource::Food)), 2);

	// The tool of the gesture (J5c.2): axe for wood, pickaxe for stone and gold, bare hands for berries; the load is not
	// shown while gathering.
	TestEqual(TEXT("chopping: the axe"), static_cast<int32>(Chop.Prop), 1);
	TestEqual(TEXT("mining stone: the pickaxe"), Prop(Gathering(EHOLResource::Stone)), 2);
	TestEqual(TEXT("mining gold: the pickaxe"), Prop(Gathering(EHOLResource::Gold)), 2);
	TestEqual(TEXT("picking berries: bare hands"), Prop(Gathering(EHOLResource::Food)), 0);
	TestEqual(TEXT("chopping with a load of food: the axe, no basket"), Prop(WithLoad(Gathering(EHOLResource::Wood), EHOLResource::Food)), 1);

	// Building (J6a.4): the hammering and the hammer at its site, whatever the speed or the load, facing the site; walking
	// there (Moving) is a walk.
	const FHOLUnitAnimationChoice Hammer = Choose(Peasant, MakeInput(EHOLUnitActivity::Building, 0.f));
	TestEqual(TEXT("building: the hammering"), Hammer.Sequence, 11);
	TestEqual(TEXT("building: the hammer"), static_cast<int32>(Hammer.Prop), 7);
	TestEqual(TEXT("the hammering is not slowed by the mesh scale"), Hammer.PlayRate, 1.f);
	TestTrue(TEXT("building: faces its site"), Hammer.bFacesSite);
	TestFalse(TEXT("building: not a source to face"), Hammer.bFacesSource);
	TestFalse(TEXT("gathering: not a site to face"), Chop.bFacesSite);
	TestEqual(TEXT("pushed aside while building: keeps hammering"), Sequence(MakeInput(EHOLUnitActivity::Building, Walking, 11)), 11);
	TestEqual(TEXT("building with a load of wood (kept, D296): the hammering"),
		Sequence(WithLoad(MakeInput(EHOLUnitActivity::Building, 0.f), EHOLResource::Wood)), 11);
	TestEqual(TEXT("building with a load of wood: the hammer, no logs"), Prop(WithLoad(MakeInput(EHOLUnitActivity::Building, 0.f), EHOLResource::Wood)), 7);
	TestEqual(TEXT("walking to its site: walk"), Sequence(MakeInput(EHOLUnitActivity::Moving, Walking)), 1);
	TestEqual(TEXT("walking to its site with stone: carrying the sack"),
		Sequence(WithLoad(MakeInput(EHOLUnitActivity::Moving, Walking), EHOLResource::Stone)), 7);
	TestFalse(TEXT("the hammering is not a walk"), HOLUnitAnimation::IsWalkSequence(Peasant, 11));
	TestEqual(TEXT("between the thresholds, from the hammering, no longer building: idle"), Sequence(MakeInput(EHOLUnitActivity::Moving,
		Walking * (HOLUnitAnimation::WalkStartSpeedShare + HOLUnitAnimation::WalkStopSpeedShare) / 2.f, 11)), 0);

	// Walking with a load: the load's carrying walk, on any walk (its round trip, an order, a push).
	const FHOLUnitAnimationChoice CarryWood = Choose(Peasant, WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Wood));
	TestEqual(TEXT("to the deposit with wood: carrying wood"), CarryWood.Sequence, 6);
	TestEqual(TEXT("carrying walk rate: CarryWalkPlayRate / MeshScale"), CarryWood.PlayRate, 3.57f / 1.5f, UE_KINDA_SMALL_NUMBER);
	TestFalse(TEXT("walking: faces its walk"), CarryWood.bFacesSource);
	TestEqual(TEXT("with food: carrying food"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Food)), 5);
	TestEqual(TEXT("with stone: carrying the sack"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Stone)), 7);
	TestEqual(TEXT("ordered away with gold (load kept, D296): carrying the sack"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::Moving, Walking), EHOLResource::Gold)), 7);
	TestEqual(TEXT("back to its source, nothing carried: walk"), Sequence(MakeInput(EHOLUnitActivity::GatherTrip, Walking)), 1);

	// The load carried (J5c.2), walking and standing: basket, logs, hotte of stone or of gold.
	TestEqual(TEXT("carrying wood: the logs"), static_cast<int32>(CarryWood.Prop), 4);
	TestEqual(TEXT("carrying food: the basket"), Prop(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Food)), 3);
	TestEqual(TEXT("carrying stone: the hotte of stone"), Prop(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Stone)), 5);
	TestEqual(TEXT("carrying gold: the hotte of gold"), Prop(WithLoad(MakeInput(EHOLUnitActivity::Moving, Walking), EHOLResource::Gold)), 6);
	TestEqual(TEXT("back to its source, nothing carried: no prop"), Prop(MakeInput(EHOLUnitActivity::GatherTrip, Walking)), 0);
	const FHOLUnitAnimationChoice StandWood = Choose(Peasant, WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, 0.f), EHOLResource::Wood));
	TestEqual(TEXT("standing with wood: the loaded idle of wood"), StandWood.Sequence, 9);
	TestEqual(TEXT("standing with wood: the logs stay shown"), static_cast<int32>(StandWood.Prop), 4);
	TestEqual(TEXT("a loaded idle plays at its own rate"), StandWood.PlayRate, 1.f);
	TestFalse(TEXT("standing with a load: its own orientation"), StandWood.bFacesSource);
	TestEqual(TEXT("standing with food: the loaded idle of food"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::Idle, 0.f), EHOLResource::Food)), 8);
	TestEqual(TEXT("standing with food: the basket"), Prop(WithLoad(MakeInput(EHOLUnitActivity::Idle, 0.f), EHOLResource::Food)), 3);
	TestEqual(TEXT("standing with stone: the loaded idle of stone"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::Idle, 0.f), EHOLResource::Stone)), 10);
	TestEqual(TEXT("standing with gold: the loaded idle of stone"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::Moving, 0.f), EHOLResource::Gold)), 10);
	TestEqual(TEXT("standing with gold: the hotte of gold"), Prop(WithLoad(MakeInput(EHOLUnitActivity::Moving, 0.f), EHOLResource::Gold)), 6);

	// Hysteresis: between the two thresholds, a walk (plain or carrying) keeps walking, anything else stands.
	const float Between = Walking * (HOLUnitAnimation::WalkStartSpeedShare + HOLUnitAnimation::WalkStopSpeedShare) / 2.f;
	TestEqual(TEXT("between the thresholds, from idle: idle"), Sequence(MakeInput(EHOLUnitActivity::Moving, Between, 0)), 0);
	TestEqual(TEXT("between the thresholds, from the walk: walk"), Sequence(MakeInput(EHOLUnitActivity::Moving, Between, 1)), 1);
	TestEqual(TEXT("between the thresholds, from carrying wood: carrying wood"),
		Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Between, 6), EHOLResource::Wood)), 6);
	TestEqual(TEXT("between the thresholds, from the axe: idle"), Sequence(MakeInput(EHOLUnitActivity::GatherTrip, Between, 2)), 0);
	TestEqual(TEXT("between the thresholds, from the loaded idle of wood: loaded idle"),
		Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Between, 9), EHOLResource::Wood)), 9);
	TestEqual(TEXT("between the thresholds, from carrying wood: the logs stay shown"),
		Prop(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Between, 6), EHOLResource::Wood)), 4);
	TestEqual(TEXT("above the start threshold, from the loaded idle: carrying walk"),
		Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking, 9), EHOLResource::Wood)), 6);
	TestEqual(TEXT("first frame (no sequence yet), between the thresholds: idle"), Sequence(MakeInput(EHOLUnitActivity::Moving, Between)), 0);
	TestTrue(TEXT("the carrying walks are walks"), HOLUnitAnimation::IsWalkSequence(Peasant, 5) && HOLUnitAnimation::IsWalkSequence(Peasant, 7));
	TestFalse(TEXT("a gesture is not a walk"), HOLUnitAnimation::IsWalkSequence(Peasant, 3));
	TestFalse(TEXT("a loaded idle is not a walk"), HOLUnitAnimation::IsWalkSequence(Peasant, 8) || HOLUnitAnimation::IsWalkSequence(Peasant, 10));
	TestFalse(TEXT("no sequence is not a walk"), HOLUnitAnimation::IsWalkSequence(Peasant, INDEX_NONE));

	// A type without the sequences (a soldier, -1 everywhere): idle and walk only.
	FHOLUnitTypeConstants Soldier;
	Soldier.Speed = 300.f;
	Soldier.MeshScale = 1.5f;
	Soldier.WalkPlayRate = 1.22f;
	TestEqual(TEXT("no gesture: idle"), Choose(Soldier, Gathering(EHOLResource::Wood)).Sequence, Soldier.IdleSequence);
	TestFalse(TEXT("no gesture: its own orientation"), Choose(Soldier, Gathering(EHOLResource::Wood)).bFacesSource);
	const FHOLUnitAnimationChoice SoldierBuilding = Choose(Soldier, MakeInput(EHOLUnitActivity::Building, 0.f));
	TestEqual(TEXT("no building gesture: idle"), SoldierBuilding.Sequence, Soldier.IdleSequence);
	TestFalse(TEXT("no building gesture: its own orientation"), SoldierBuilding.bFacesSite);
	TestEqual(TEXT("no building gesture: no prop"), static_cast<int32>(SoldierBuilding.Prop), 0);
	TestEqual(TEXT("no carrying walk: walk"), Choose(Soldier, WithLoad(MakeInput(EHOLUnitActivity::Moving, 300.f), EHOLResource::Wood)).Sequence,
		Soldier.WalkSequence);
	const FHOLUnitAnimationChoice SoldierStanding = Choose(Soldier, WithLoad(MakeInput(EHOLUnitActivity::Idle, 0.f), EHOLResource::Gold));
	TestEqual(TEXT("no loaded idle: idle"), SoldierStanding.Sequence, Soldier.IdleSequence);
	TestEqual(TEXT("no props: none"), static_cast<int32>(SoldierStanding.Prop), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAnimationTableTest, "HeroesOfLiberty.Animation.Table", UE::HeroesOfLiberty::Tests::Animation::Flags)
bool FHOLAnimationTableTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Animation;
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Peasant = UnitTypes ? UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Peasant"), TEXT("Animation test")) : nullptr;
	if (!TestNotNull(TEXT("Peasant in the unit type table"), Peasant))
	{
		return false;
	}
	const FHOLUnitTypeConstants Constants = Peasant->ToConstants(TEXT("Peasant"));
	const FHOLUnitTypeConstants Expected = MakePeasant();
	TestTrue(TEXT("gathering sequences reach the shared fragment"), Constants.GatherSequences == Expected.GatherSequences);
	TestTrue(TEXT("carrying walks reach the shared fragment"), Constants.CarryWalkSequences == Expected.CarryWalkSequences);
	TestEqual(TEXT("gathering play rate"), Constants.GatherPlayRate, Expected.GatherPlayRate);
	// The project's rule (D234): Speed / planted-foot speed of the walk, 600 / 168.2 for A_SW_Unarmed_Walk's legs.
	TestEqual(TEXT("carrying walk play rate (600 / 168.2)"), Constants.CarryWalkPlayRate, 3.57f, 0.005f);
	TestEqual(TEXT("walk play rate unchanged (600 / 246.2)"), Constants.WalkPlayRate, 2.44f, 0.005f);
	TestTrue(TEXT("loaded idles reach the shared fragment"), Constants.CarryIdleSequences == Expected.CarryIdleSequences);
	TestTrue(TEXT("gathering props reach the shared fragment"), Constants.GatherProps == Expected.GatherProps);
	TestTrue(TEXT("carrying props reach the shared fragment"), Constants.CarryProps == Expected.CarryProps);
	TestEqual(TEXT("building sequence"), Constants.BuildSequence, Expected.BuildSequence);
	TestEqual(TEXT("building play rate"), Constants.BuildPlayRate, Expected.BuildPlayRate);
	TestEqual(TEXT("building prop: the hammer"), Constants.BuildProp, Expected.BuildProp);
	// The props live in the Peasant's own mesh, drawn with the material that hides all but the one named (J5c.2).
	TestEqual(TEXT("Peasant's mesh"), Peasant->SkinnedMesh.ToSoftObjectPath().GetAssetName(), TEXT("SKM_Peasant"));

	// The provider plays them by index: each index is the sequence made for it (Tools/Editor/make_skinned_mannequin.py).
	const UAnimSequenceTransformProviderData* Provider = Peasant->AnimationProvider.LoadSynchronous();
	if (!TestNotNull(TEXT("Peasant's animation provider"), Provider))
	{
		return false;
	}
	const auto SequenceName = [Provider](const int32 Index)
	{
		const TArray<FAnimSequenceTransformProviderSequence>& Sequences = Provider->GetSequences();
		return Sequences.IsValidIndex(Index) && Sequences[Index].Sequence ? Sequences[Index].Sequence->GetName() : FString(TEXT("none"));
	};
	TestEqual(TEXT("idle"), SequenceName(Constants.IdleSequence), TEXT("A_SW_Idle"));
	TestEqual(TEXT("walk"), SequenceName(Constants.WalkSequence), TEXT("A_SW_Walk_F"));
	TestEqual(TEXT("gathering food"), SequenceName(Constants.GatherSequences.Food), TEXT("A_Peasant_GatherBerries"));
	TestEqual(TEXT("gathering wood"), SequenceName(Constants.GatherSequences.Wood), TEXT("A_Peasant_ChopWood"));
	TestEqual(TEXT("gathering stone"), SequenceName(Constants.GatherSequences.Stone), TEXT("A_Peasant_MinePick"));
	TestEqual(TEXT("gathering gold"), SequenceName(Constants.GatherSequences.Gold), TEXT("A_Peasant_MinePick"));
	TestEqual(TEXT("carrying food"), SequenceName(Constants.CarryWalkSequences.Food), TEXT("A_Peasant_CarryFood"));
	TestEqual(TEXT("carrying wood"), SequenceName(Constants.CarryWalkSequences.Wood), TEXT("A_Peasant_CarryWood"));
	TestEqual(TEXT("carrying stone"), SequenceName(Constants.CarryWalkSequences.Stone), TEXT("A_Peasant_CarryStone"));
	TestEqual(TEXT("carrying gold"), SequenceName(Constants.CarryWalkSequences.Gold), TEXT("A_Peasant_CarryStone"));
	TestEqual(TEXT("standing with food"), SequenceName(Constants.CarryIdleSequences.Food), TEXT("A_Peasant_IdleCarryFood"));
	TestEqual(TEXT("standing with wood"), SequenceName(Constants.CarryIdleSequences.Wood), TEXT("A_Peasant_IdleCarryWood"));
	TestEqual(TEXT("standing with stone"), SequenceName(Constants.CarryIdleSequences.Stone), TEXT("A_Peasant_IdleCarryStone"));
	TestEqual(TEXT("standing with gold"), SequenceName(Constants.CarryIdleSequences.Gold), TEXT("A_Peasant_IdleCarryStone"));
	TestEqual(TEXT("building"), SequenceName(Constants.BuildSequence), TEXT("A_Peasant_Build"));
	// Sequences 0-7 are the soldiers' provider's, in its order (the same indices for both meshes).
	const FHOLUnitTypeRow* ArcherRow = UnitTypes->FindRow<FHOLUnitTypeRow>(TEXT("Archer"), TEXT("Animation test"));
	const UAnimSequenceTransformProviderData* SoldierProvider = ArcherRow ? ArcherRow->AnimationProvider.LoadSynchronous() : nullptr;
	if (TestNotNull(TEXT("soldiers' animation provider"), SoldierProvider))
	{
		const TArray<FAnimSequenceTransformProviderSequence>& Shared = SoldierProvider->GetSequences();
		bool bSameStart = Shared.Num() <= Provider->GetSequences().Num();
		for (int32 Index = 0; bSameStart && Index < Shared.Num(); ++Index)
		{
			bSameStart = Shared[Index].Sequence == Provider->GetSequences()[Index].Sequence;
		}
		TestTrue(TEXT("the Peasant's provider starts with the soldiers' sequences, same order"), bSameStart);
	}
	TestEqual(TEXT("the Peasant's provider: 12 sequences"), Provider->GetSequences().Num(), 12);

	// The soldiers keep idle 0 and walk 1 of the same provider, and nothing else.
	for (const TCHAR* Soldier : { TEXT("LightInfantry"), TEXT("Archer") })
	{
		const FHOLUnitTypeRow* Row = UnitTypes->FindRow<FHOLUnitTypeRow>(Soldier, TEXT("Animation test"));
		if (TestNotNull(*FString::Printf(TEXT("%s in the unit type table"), Soldier), Row))
		{
			TestTrue(*FString::Printf(TEXT("%s: idle 0, walk 1, no gesture, carrying walk, loaded idle, building nor prop"), Soldier), Row->IdleSequence == 0
				&& Row->WalkSequence == 1 && Row->GatherSequences == FHOLResourceSequences() && Row->CarryWalkSequences == FHOLResourceSequences()
				&& Row->CarryIdleSequences == FHOLResourceSequences() && Row->GatherProps == FHOLResourceProps() && Row->CarryProps == FHOLResourceProps()
				&& Row->BuildSequence == INDEX_NONE && Row->BuildProp == 0);
			TestEqual(*FString::Printf(TEXT("%s keeps the mannequin's mesh"), Soldier), Row->SkinnedMesh.ToSoftObjectPath().GetAssetName(),
				TEXT("SKM_Mannequin_Classic"));
		}
	}
	return true;
}

#endif
