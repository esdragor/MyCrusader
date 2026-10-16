// Copyright Epic Games, Inc. All Rights Reserved.

#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceTransformProviderData.h"
#include "Engine/DataTable.h"
#include "Misc/AutomationTest.h"
#include "Presentation/HOLUnitAnimation.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J5c.1 checks: the animation a client plays on a unit (D234, HOLUnitAnimation::Choose) and the Peasant's sequences in
// the type table and its animation provider.
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

	// Idle and walk as before (D234), the walk slowed by the mesh scale.
	const FHOLUnitAnimationChoice Idle = Choose(Peasant, MakeInput(EHOLUnitActivity::Idle, 0.f));
	TestEqual(TEXT("standing: idle"), Idle.Sequence, 0);
	TestEqual(TEXT("idle at its own rate"), Idle.PlayRate, 1.f);
	TestFalse(TEXT("idle: its own orientation"), Idle.bFacesSource);
	const FHOLUnitAnimationChoice Walk = Choose(Peasant, MakeInput(EHOLUnitActivity::Moving, Walking));
	TestEqual(TEXT("walking: walk"), Walk.Sequence, 1);
	TestEqual(TEXT("walk rate: WalkPlayRate / MeshScale"), Walk.PlayRate, 2.44f / 1.5f, UE_KINDA_SMALL_NUMBER);

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

	// Walking with a load: the load's carrying walk, on any walk (its round trip, an order, a push).
	const FHOLUnitAnimationChoice CarryWood = Choose(Peasant, WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Wood));
	TestEqual(TEXT("to the deposit with wood: carrying wood"), CarryWood.Sequence, 6);
	TestEqual(TEXT("carrying walk rate: CarryWalkPlayRate / MeshScale"), CarryWood.PlayRate, 3.57f / 1.5f, UE_KINDA_SMALL_NUMBER);
	TestFalse(TEXT("walking: faces its walk"), CarryWood.bFacesSource);
	TestEqual(TEXT("with food: carrying food"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Food)), 5);
	TestEqual(TEXT("with stone: carrying the sack"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Walking), EHOLResource::Stone)), 7);
	TestEqual(TEXT("ordered away with gold (load kept, D296): carrying the sack"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::Moving, Walking), EHOLResource::Gold)), 7);
	TestEqual(TEXT("standing with a load: idle"), Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, 0.f), EHOLResource::Wood)), 0);
	TestEqual(TEXT("back to its source, nothing carried: walk"), Sequence(MakeInput(EHOLUnitActivity::GatherTrip, Walking)), 1);

	// Hysteresis: between the two thresholds, a walk (plain or carrying) keeps walking, anything else stands.
	const float Between = Walking * (HOLUnitAnimation::WalkStartSpeedShare + HOLUnitAnimation::WalkStopSpeedShare) / 2.f;
	TestEqual(TEXT("between the thresholds, from idle: idle"), Sequence(MakeInput(EHOLUnitActivity::Moving, Between, 0)), 0);
	TestEqual(TEXT("between the thresholds, from the walk: walk"), Sequence(MakeInput(EHOLUnitActivity::Moving, Between, 1)), 1);
	TestEqual(TEXT("between the thresholds, from carrying wood: carrying wood"),
		Sequence(WithLoad(MakeInput(EHOLUnitActivity::GatherTrip, Between, 6), EHOLResource::Wood)), 6);
	TestEqual(TEXT("between the thresholds, from the axe: idle"), Sequence(MakeInput(EHOLUnitActivity::GatherTrip, Between, 2)), 0);
	TestEqual(TEXT("first frame (no sequence yet), between the thresholds: idle"), Sequence(MakeInput(EHOLUnitActivity::Moving, Between)), 0);
	TestTrue(TEXT("the carrying walks are walks"), HOLUnitAnimation::IsWalkSequence(Peasant, 5) && HOLUnitAnimation::IsWalkSequence(Peasant, 7));
	TestFalse(TEXT("a gesture is not a walk"), HOLUnitAnimation::IsWalkSequence(Peasant, 3));
	TestFalse(TEXT("no sequence is not a walk"), HOLUnitAnimation::IsWalkSequence(Peasant, INDEX_NONE));

	// A type without the sequences (a soldier, -1 everywhere): idle and walk only.
	FHOLUnitTypeConstants Soldier;
	Soldier.Speed = 300.f;
	Soldier.MeshScale = 1.5f;
	Soldier.WalkPlayRate = 1.22f;
	TestEqual(TEXT("no gesture: idle"), Choose(Soldier, Gathering(EHOLResource::Wood)).Sequence, Soldier.IdleSequence);
	TestFalse(TEXT("no gesture: its own orientation"), Choose(Soldier, Gathering(EHOLResource::Wood)).bFacesSource);
	TestEqual(TEXT("no carrying walk: walk"), Choose(Soldier, WithLoad(MakeInput(EHOLUnitActivity::Moving, 300.f), EHOLResource::Wood)).Sequence,
		Soldier.WalkSequence);
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

	// The soldiers keep idle 0 and walk 1 of the same provider, and nothing else.
	for (const TCHAR* Soldier : { TEXT("LightInfantry"), TEXT("Archer") })
	{
		const FHOLUnitTypeRow* Row = UnitTypes->FindRow<FHOLUnitTypeRow>(Soldier, TEXT("Animation test"));
		if (TestNotNull(*FString::Printf(TEXT("%s in the unit type table"), Soldier), Row))
		{
			TestTrue(*FString::Printf(TEXT("%s: idle 0, walk 1, no gesture nor carrying walk"), Soldier), Row->IdleSequence == 0 && Row->WalkSequence == 1
				&& Row->GatherSequences == FHOLResourceSequences() && Row->CarryWalkSequences == FHOLResourceSequences());
		}
	}
	return true;
}

#endif
