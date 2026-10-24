// Copyright Epic Games, Inc. All Rights Reserved.

#include "Algo/AnyOf.h"
#include "Engine/DataTable.h"
#include "Iris/Serialization/NetBitStreamReader.h"
#include "Iris/Serialization/NetBitStreamWriter.h"
#include "Iris/Serialization/NetSerializationContext.h"
#include "Misc/AutomationTest.h"
#include "Net/HOLCheatRules.h"
#include "Net/HOLManagerLink.h"
#include "Net/HOLNetUnitDisplay.h"
#include "Net/HOLNetUnitStateNetSerializer.h"
#include "Net/HOLUnitReplicationRules.h"
#include "Net/HOLUnitReplicator.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// M2.5 checks (D273): the pure rules of the unit replication, reduction (D269, D270), element format on the wire (D270),
// send rule (D269), server clock, prediction and smoothed corrections (D271).
namespace UE::HeroesOfLiberty::Tests::Net
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	constexpr float StepSeconds = 0.05f;

	/** What the server records when it sends a unit. */
	FHOLUnitSentRecord Send(const FVector& Position, const FVector& Velocity, const EHOLUnitActivity Activity, const int64 Step)
	{
		return { FHOLNetUnitState::Make(Position, Velocity, Activity, Step), Step };
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetReductionTest, "HeroesOfLiberty.Net.Reduction", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetReductionTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("57 bits per state (D301, a load bit of J5, J7-Q5: a 4th activity bit and the full health bit, D343: 7 step bits, D344: 13 per position)"),
		FHOLNetUnitState::TotalBits, 57);
	TestEqual(TEXT("8 bits more for a wounded unit, its exact health (J7-Q5 revised)"), FHOLNetUnitState::WoundedBits, 8);
	TestEqual(TEXT("12 bits more for an attacking one, its target's slot"), FHOLNetUnitState::AttackingBits, 12);
	TestEqual(TEXT("6 bits more for one facing its front at rest"), FHOLNetUnitState::FacingFrontBits, 6);
	TestEqual(TEXT("2 bits more for a carrier (J5, D303: its resource, never its amount)"), FHOLNetUnitState::CarriedLoadBits, 2);

	const FVector Position(12345.6, -9876.5, 123.4);
	const FVector Velocity(212.3, -147.9, 35.0);
	const FHOLNetUnitState State = FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Moving, 1000);
	const FVector Back = State.GetPosition();
	TestTrue(TEXT("position within half a step per axis"), FMath::Abs(Back.X - Position.X) <= 5.5 && FMath::Abs(Back.Y - Position.Y) <= 5.5);
	TestEqual(TEXT("no height: the client puts the unit on the ground (D301)"), Back.Z, 0.0);
	const FVector BackVelocity = State.GetVelocity();
	TestTrue(TEXT("horizontal velocity within half a step"),
		FMath::Abs(BackVelocity.X - Velocity.X) <= 2.5 && FMath::Abs(BackVelocity.Y - Velocity.Y) <= 2.5);
	TestEqual(TEXT("no vertical velocity"), BackVelocity.Z, 0.0);
	TestEqual(TEXT("activity kept"), State.Activity, EHOLUnitActivity::Moving);
	TestEqual(TEXT("low bits of the step"), State.Step, static_cast<uint8>(1000 % (1 << FHOLNetUnitState::StepBits)));
	TestEqual(TEXT("same values, same state"), FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Moving, 1000), State);
	const FHOLNetUnitState Carrier = FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Gathering, 1000, EHOLResource::Wood, 7);
	TestTrue(TEXT("a carrier"), Carrier.bCarrying);
	TestEqual(TEXT("load's resource kept"), static_cast<int32>(Carrier.LoadResource), static_cast<int32>(EHOLResource::Wood));
	TestEqual(TEXT("the amount is not sent (D303): 7 or 9 make the same state"), Carrier,
		FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Gathering, 1000, EHOLResource::Wood, 9));
	TestEqual(TEXT("nothing carried: the resource does not count"), FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Idle, 0, EHOLResource::Gold, 0),
		FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Idle, 0));
	// J5c.1: what a gatherer gathers, kept while it gathers only (it may carry another resource, D306).
	const FHOLNetUnitState Chopping = FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::Gathering, 1000, EHOLResource::Food, 3, EHOLResource::Wood);
	TestEqual(TEXT("gathered resource kept"), static_cast<int32>(Chopping.GatherResource), static_cast<int32>(EHOLResource::Wood));
	TestEqual(TEXT("load of another resource kept beside it"), static_cast<int32>(Chopping.LoadResource), static_cast<int32>(EHOLResource::Food));
	TestEqual(TEXT("not gathering: the gathered resource does not count"),
		FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::GatherTrip, 0, EHOLResource::Wood, 10, EHOLResource::Stone),
		FHOLNetUnitState::Make(Position, Velocity, EHOLUnitActivity::GatherTrip, 0, EHOLResource::Wood, 10));

	const FHOLNetUnitState Far = FHOLNetUnitState::Make(FVector(1.0e7, -1.0e7, 1.0e6), FVector(1.0e5, -1.0e5, 0.0), EHOLUnitActivity::Idle, 0);
	TestEqual(TEXT("X held at the range's edge"), static_cast<int32>(Far.X), (1 << (FHOLNetUnitState::HorizontalBits - 1)) - 1);
	TestEqual(TEXT("Y held at the range's edge"), static_cast<int32>(Far.Y), -(1 << (FHOLNetUnitState::HorizontalBits - 1)));
	TestEqual(TEXT("velocity held at the range's edge"), static_cast<int32>(Far.VelocityX), (1 << (FHOLNetUnitState::VelocityBits - 1)) - 1);
	TestTrue(TEXT("the 400 m maps fit"), (1 << (FHOLNetUnitState::HorizontalBits - 1)) * FHOLNetUnitState::PositionStep > 25000.f);
	TestTrue(TEXT("the largest maps fit, 220 tiles of 4 m centred on the origin (D344)"),
		(1 << (FHOLNetUnitState::HorizontalBits - 1)) * FHOLNetUnitState::PositionStep >= 44000.f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetElementFormatTest, "HeroesOfLiberty.Net.ElementFormat", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetElementFormatTest::RunTest(const FString& Parameters)
{
	using namespace UE::Net;
	const FNetSerializer& Serializer = UE_NET_GET_SERIALIZER(FHOLNetUnitStateNetSerializer);
	TestEqual(TEXT("one chunk of slots per change bit of an Iris array"), AHOLUnitChunk::NumSlots, 63);

	// Every field at both ends of its range and in between, through Iris' quantize, write, read and dequantize.
	const FHOLNetUnitState States[] = {
		FHOLNetUnitState::Make(FVector(12345.6, -9876.5, 123.4), FVector(212.3, -147.9, 0.0), EHOLUnitActivity::Moving, 1000),
		FHOLNetUnitState::Make(FVector(1.0e7, -1.0e7, -1.0e6), FVector(-1.0e5, 1.0e5, 0.0), EHOLUnitActivity::Idle, 255),
		FHOLNetUnitState::Make(FVector(-1.0e7, 1.0e7, 1.0e6), FVector(1.0e5, -1.0e5, 0.0), EHOLUnitActivity::Moving, 256),
		FHOLNetUnitState::Make(FVector(500.0, 600.0, 0.0), FVector::ZeroVector, EHOLUnitActivity::Gathering, 77, EHOLResource::Gold, 31, EHOLResource::Gold),
		// J5c.1: each resource gathered, in the activity's bits; with a load of another resource (D306), without a load.
		FHOLNetUnitState::Make(FVector(500.0, 600.0, 0.0), FVector(-5.0, 10.0, 0.0), EHOLUnitActivity::Gathering, 78, EHOLResource::Food, 0, EHOLResource::Food),
		FHOLNetUnitState::Make(FVector(500.0, 600.0, 0.0), FVector::ZeroVector, EHOLUnitActivity::Gathering, 79, EHOLResource::Food, 4, EHOLResource::Wood),
		FHOLNetUnitState::Make(FVector(500.0, 600.0, 0.0), FVector::ZeroVector, EHOLUnitActivity::Gathering, 80, EHOLResource::Food, 0, EHOLResource::Stone),
		FHOLNetUnitState::Make(FVector(-500.0, 600.0, 0.0), FVector(300.0, 0.0, 0.0), EHOLUnitActivity::GatherTrip, 81, EHOLResource::Stone, 10),
		// J6a.4: a builder at work, a load kept from its gathering (Gathering's own value on the wire).
		FHOLNetUnitState::Make(FVector(-500.0, 600.0, 0.0), FVector::ZeroVector, EHOLUnitActivity::Building, 82, EHOLResource::Wood, 6),
		FHOLNetUnitState(),
	};
	// J7b.4 (J7-Q5): wounded, attacking (its target's slot at both ends), dying, facing its front, a wounded carrier.
	TArray<FHOLNetUnitState> AllStates(States, UE_ARRAY_COUNT(States));
	const auto WithCombat = [](FHOLNetUnitState State, const int32 Health, const int32 MaxHealth, const uint16 TargetSlot, const bool bFront, const float Yaw)
	{
		State.SetCombat(Health, MaxHealth, TargetSlot, bFront, Yaw);
		return State;
	};
	const FVector Here(700.0, -800.0, 0.0);
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Idle, 90), 37, 100, 5, false, 0.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector(3.0, -4.0, 0.0), EHOLUnitActivity::Attacking, 91), 160, 160, 0, false, 0.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Attacking, 92), 1, 160, FHOLNetUnitState::InvalidTargetSlot, false, 0.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Attacking, 93), 80, 160, 1234, false, 0.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Dying, 94), 0, 160, 77, false, 0.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Idle, 95), 160, 160, 0, true, -135.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Idle, 96), 20, 160, 0, true, 179.f));
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::GatherTrip, 97, EHOLResource::Gold, 5), 99, 100, 0, true, 90.f));
	// J7-Q5 revised: the largest health the wire carries, and each type of the table one hit off its maximum and at 1.
	AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Idle, 98), FHOLNetUnitState::MaxWireHealth,
		FHOLNetUnitState::MaxWireHealth + 1, 0, false, 0.f));
	int32 TableMaxHealth = 0;
	if (const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous(); TestNotNull(TEXT("unit type table"), UnitTypes))
	{
		UnitTypes->ForeachRow<FHOLUnitTypeRow>(TEXT("Net test"), [&](const FName& Name, const FHOLUnitTypeRow& Row)
		{
			TableMaxHealth = FMath::Max(TableMaxHealth, Row.MaxHealth);
			TestTrue(*FString::Printf(TEXT("%s: MaxHealth %d fits the health bits"), *Name.ToString(), Row.MaxHealth), HOLUnitReplication::FitsWireHealth(Row.MaxHealth));
			AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Attacking, 99), Row.MaxHealth - 1, Row.MaxHealth, 3, false, 0.f));
			AllStates.Add(WithCombat(FHOLNetUnitState::Make(Here, FVector::ZeroVector, EHOLUnitActivity::Idle, 100), 1, Row.MaxHealth, 0, false, 0.f));
		});
	}
	TestTrue(*FString::Printf(TEXT("the table's largest maximum (%d) is the one the bits are set for: 8 bits while it is at most 256"), TableMaxHealth),
		TableMaxHealth > 1 && HOLUnitReplication::FitsWireHealth(TableMaxHealth) && (TableMaxHealth > 256 || FHOLNetUnitState::HealthBits == 8));
	for (const FHOLNetUnitState& State : AllStates)
	{
		alignas(16) uint8 Quantized[64] = {};
		alignas(16) uint8 Received[64] = {};
		if (!TestTrue(TEXT("quantized form fits the test buffer"), Serializer.QuantizedTypeSize <= sizeof(Quantized)))
		{
			return false;
		}
		FNetSerializationContext Context;
		FNetValidateArgs ValidateArgs;
		ValidateArgs.NetSerializerConfig = Serializer.DefaultConfig;
		ValidateArgs.Source = NetSerializerValuePointer(&State);
		TestTrue(TEXT("valid"), Serializer.Validate(Context, ValidateArgs));

		FNetQuantizeArgs QuantizeArgs;
		QuantizeArgs.NetSerializerConfig = Serializer.DefaultConfig;
		QuantizeArgs.Source = NetSerializerValuePointer(&State);
		QuantizeArgs.Target = NetSerializerValuePointer(Quantized);
		Serializer.Quantize(Context, QuantizeArgs);

		uint32 Buffer[8] = {};
		FNetBitStreamWriter Writer;
		Writer.InitBytes(Buffer, sizeof(Buffer));
		FNetSerializationContext WriteContext(&Writer);
		FNetSerializeArgs SerializeArgs;
		SerializeArgs.NetSerializerConfig = Serializer.DefaultConfig;
		SerializeArgs.Source = NetSerializerValuePointer(Quantized);
		Serializer.Serialize(WriteContext, SerializeArgs);
		Writer.CommitWrites();
		TestEqual(TEXT("57 bits on the wire (1 for the health of an unhurt unit), +2 for a carrier, +8 wounded, +12 attacking, +6 facing its front"), Writer.GetPosBits(),
			static_cast<uint32>(FHOLNetUnitState::TotalBits + (State.bCarrying ? FHOLNetUnitState::CarriedLoadBits : 0)
				+ (State.bFullHealth ? 0 : FHOLNetUnitState::WoundedBits)
				+ (State.Activity == EHOLUnitActivity::Attacking ? FHOLNetUnitState::AttackingBits : 0)
				+ (State.bFacesFront ? FHOLNetUnitState::FacingFrontBits : 0)));

		FNetBitStreamReader Reader;
		Reader.InitBits(Buffer, Writer.GetPosBits());
		FNetSerializationContext ReadContext(&Reader);
		FNetDeserializeArgs DeserializeArgs;
		DeserializeArgs.NetSerializerConfig = Serializer.DefaultConfig;
		DeserializeArgs.Target = NetSerializerValuePointer(Received);
		Serializer.Deserialize(ReadContext, DeserializeArgs);
		TestFalse(TEXT("read within the bits written"), Reader.IsOverflown());

		FHOLNetUnitState Back;
		FNetDequantizeArgs DequantizeArgs;
		DequantizeArgs.NetSerializerConfig = Serializer.DefaultConfig;
		DequantizeArgs.Source = NetSerializerValuePointer(Received);
		DequantizeArgs.Target = NetSerializerValuePointer(&Back);
		Serializer.Dequantize(ReadContext, DequantizeArgs);
		TestEqual(TEXT("same state after the trip"), Back, State);
		TestEqual(TEXT("exact health after the trip (0 unhurt, not sent)"), Back.Health, State.Health);
	}

	// J5c.1 (option A of the user): "gathering + resource" in the 4 values the activities leave free, at no bit more;
	// J6a.4: Building in Gathering's own value, the last one free. The reflection's last entry is the _MAX it adds.
	const UEnum* Activities = StaticEnum<EHOLUnitActivity>();
	for (int32 Index = 0; Index < Activities->NumEnums() - 1; ++Index)
	{
		const EHOLUnitActivity Activity = static_cast<EHOLUnitActivity>(Activities->GetValueByIndex(Index));
		TestTrue(*FString::Printf(TEXT("%s carried by the wire"), *Activities->GetNameStringByIndex(Index)), HOLUnitReplication::IsValidActivity(Activity));
		TestTrue(*FString::Printf(TEXT("%s below the gathering values on the wire, or after them (J7b.4)"), *Activities->GetNameStringByIndex(Index)),
			Activity == EHOLUnitActivity::Gathering || HOLUnitReplication::EncodeActivity(Activity, EHOLResource::Food) < HOLUnitReplication::GatheringWireBase
			|| HOLUnitReplication::EncodeActivity(Activity, EHOLResource::Food) >= HOLUnitReplication::GatheringWireBase + 4);
	}
	TestEqual(TEXT("the 4 resources end at the old 3 bits' end"), HOLUnitReplication::GatheringWireBase + 4, 8);
	TestEqual(TEXT("4 bits of activity (J7-Q5)"), FHOLNetUnitState::ActivityBits, 4);
	TArray<uint8> WireValues;
	for (const EHOLUnitActivity Activity : { EHOLUnitActivity::Idle, EHOLUnitActivity::Moving, EHOLUnitActivity::Gathering, EHOLUnitActivity::GatherTrip,
		EHOLUnitActivity::Building, EHOLUnitActivity::Attacking, EHOLUnitActivity::Dying })
	{
		for (const EHOLResource Resource : { EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })
		{
			const uint8 Wire = HOLUnitReplication::EncodeActivity(Activity, Resource);
			EHOLUnitActivity BackActivity = EHOLUnitActivity::Idle;
			EHOLResource BackResource = EHOLResource::Gold;
			HOLUnitReplication::DecodeActivity(Wire, BackActivity, BackResource);
			const FString What = FString::Printf(TEXT("%s %s"), *UEnum::GetValueAsString(Activity), *UEnum::GetValueAsString(Resource));
			TestTrue(*FString::Printf(TEXT("%s fits in the activity's bits"), *What), Wire < (1 << FHOLNetUnitState::ActivityBits));
			TestEqual(*FString::Printf(TEXT("%s: activity back"), *What), BackActivity, Activity);
			TestEqual(*FString::Printf(TEXT("%s: gathered resource back (food unless gathering)"), *What), static_cast<int32>(BackResource),
				static_cast<int32>(Activity == EHOLUnitActivity::Gathering ? Resource : EHOLResource::Food));
			WireValues.AddUnique(Wire);
		}
	}
	{
		const uint8 Wire = HOLUnitReplication::EncodeActivity(EHOLUnitActivity::Idle, EHOLResource::Food, true);
		EHOLUnitActivity BackActivity = EHOLUnitActivity::Moving;
		EHOLResource BackResource = EHOLResource::Gold;
		bool bFacesFront = false;
		HOLUnitReplication::DecodeActivity(Wire, BackActivity, BackResource, bFacesFront);
		TestTrue(TEXT("idle facing its front: idle, facing it"), BackActivity == EHOLUnitActivity::Idle && bFacesFront);
		TestEqual(TEXT("facing the front only while idle"), HOLUnitReplication::EncodeActivity(EHOLUnitActivity::Moving, EHOLResource::Food, true),
			static_cast<uint8>(EHOLUnitActivity::Moving));
		WireValues.AddUnique(Wire);
	}
	TestEqual(TEXT("11 values on the wire: 4 activities, gathering each resource, attacking, dying, facing the front (5 free)"), WireValues.Num(), 11);

	// Health (J7-Q5 revised): exact health points while wounded, 0 only dead.
	TestEqual(TEXT("dead: 0"), HOLUnitReplication::GetWireHealth(0, 160), static_cast<uint16>(0));
	TestEqual(TEXT("below 0: 0"), HOLUnitReplication::GetWireHealth(-7, 160), static_cast<uint16>(0));
	TestEqual(TEXT("1 health: 1, alive"), HOLUnitReplication::GetWireHealth(1, 160), static_cast<uint16>(1));
	TestEqual(TEXT("exact: 80"), HOLUnitReplication::GetWireHealth(80, 160), static_cast<uint16>(80));
	TestEqual(TEXT("one hit off the Man-at-arms' 155: 154"), HOLUnitReplication::GetWireHealth(154, 155), static_cast<uint16>(154));
	TestEqual(TEXT("held at the wire's largest"), HOLUnitReplication::GetWireHealth(1000, 2000), FHOLNetUnitState::MaxWireHealth);
	TestTrue(TEXT("256 fits 8 bits (255 wounded at most)"), HOLUnitReplication::FitsWireHealth(256));
	TestFalse(TEXT("257 does not"), HOLUnitReplication::FitsWireHealth(257));
	FHOLNetUnitState Wounded = FHOLNetUnitState::Make(FVector::ZeroVector, FVector::ZeroVector, EHOLUnitActivity::Idle, 0);
	Wounded.SetCombat(152, 160, 0);
	TestFalse(TEXT("one hit off: not full"), Wounded.bFullHealth);
	TestEqual(TEXT("one hit off: its exact health"), Wounded.Health, static_cast<uint16>(152));
	FHOLNetUnitState Dead = FHOLNetUnitState::Make(FVector::ZeroVector, FVector::ZeroVector, EHOLUnitActivity::Dying, 0);
	Dead.SetCombat(-3, 160, 0);
	TestTrue(TEXT("dead: wounded at 0"), !Dead.bFullHealth && Dead.Health == 0);
	FHOLNetUnitState Unhurt = FHOLNetUnitState::Make(FVector::ZeroVector, FVector::ZeroVector, EHOLUnitActivity::Idle, 0);
	Unhurt.SetCombat(160, 160, 33);
	TestEqual(TEXT("unhurt and not attacking: the default state"), Unhurt, FHOLNetUnitState::Make(FVector::ZeroVector, FVector::ZeroVector, EHOLUnitActivity::Idle, 0));
	FHOLNetUnitState Front = Unhurt;
	Front.SetCombat(160, 160, 0, true, 90.f);
	TestTrue(TEXT("front within half a direction"), FMath::Abs(Front.GetFrontYaw() - 90.f) <= 2.9f);
	TestEqual(TEXT("Building in Gathering's own value (J6a.4)"), HOLUnitReplication::EncodeActivity(EHOLUnitActivity::Building, EHOLResource::Wood),
		static_cast<uint8>(EHOLUnitActivity::Gathering));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetUnwrapStepTest, "HeroesOfLiberty.Net.UnwrapStep", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetUnwrapStepTest::RunTest(const FString& Parameters)
{
	using HOLUnitReplication::UnwrapStep;
	const auto Low = [](const int64 Step) { return static_cast<uint8>(Step % (int64(1) << FHOLNetUnitState::StepBits)); };
	TestEqual(TEXT("same step"), UnwrapStep(Low(1000), 1000), int64(1000));
	TestEqual(TEXT("a little behind"), UnwrapStep(Low(990), 1000), int64(990));
	TestEqual(TEXT("a little ahead"), UnwrapStep(Low(1010), 1000), int64(1010));
	TestEqual(TEXT("behind, across a wrap"), UnwrapStep(Low(1020), 1030), int64(1020));
	TestEqual(TEXT("ahead, across a wrap"), UnwrapStep(Low(1030), 1020), int64(1030));
	TestEqual(TEXT("63 steps behind"), UnwrapStep(Low(5000 - 63), 5000), int64(5000 - 63));
	TestEqual(TEXT("63 steps ahead"), UnwrapStep(Low(5000 + 63), 5000), int64(5000 + 63));
	TestTrue(TEXT("read right beyond the slow refresh (D343)"), (1 << (FHOLNetUnitState::StepBits - 1)) > 40 + 10);
	TestEqual(TEXT("first steps of the match"), UnwrapStep(Low(3), 0), int64(3));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetPredictionTest, "HeroesOfLiberty.Net.Prediction", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetPredictionTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Net;
	// Positions on the 11 cm step (D344): 1100 and 2200 cm travel exact.
	const FHOLNetUnitState State = FHOLNetUnitState::Make(FVector(1100.0, 2200.0, 50.0), FVector(300.0, -100.0, 0.0), EHOLUnitActivity::Moving, 0);
	TestEqual(TEXT("at the state's step"), HOLUnitReplication::PredictPosition(State, 0.0, StepSeconds), FVector(1100.0, 2200.0, 0.0));
	TestEqual(TEXT("one second later, in a straight line"), HOLUnitReplication::PredictPosition(State, 20.0, StepSeconds), FVector(1400.0, 2100.0, 0.0));
	TestEqual(TEXT("between two steps"), HOLUnitReplication::PredictPosition(State, 2.5, StepSeconds), FVector(1137.5, 2187.5, 0.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetSendRuleTest, "HeroesOfLiberty.Net.SendRule", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetSendRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Net;
	using HOLUnitReplication::GetSendReason;
	const FHOLUnitSendRules Rules;

	// A unit at rest: nothing until the slow refresh, which comes after a quarter of a cycle at least, and at most a cycle
	// and a quarter.
	const int32 LongestRefresh = Rules.RefreshSteps + Rules.RefreshSteps / 4;
	const FVector Rest(5000.0, 5000.0, 0.0);
	const FHOLUnitSentRecord Resting = Send(Rest, FVector::ZeroVector, EHOLUnitActivity::Idle, 0);
	int64 FirstRefresh = -1;
	for (int64 Step = 1; Step <= LongestRefresh && FirstRefresh < 0; ++Step)
	{
		if (GetSendReason(Resting, Rest, EHOLUnitActivity::Idle, 7, Step, StepSeconds, Rules) == EHOLUnitSendReason::Refresh)
		{
			FirstRefresh = Step;
		}
	}
	TestTrue(TEXT("at rest: refreshed between a quarter of a cycle and a cycle and a quarter"), FirstRefresh >= Rules.RefreshSteps / 4 && FirstRefresh <= LongestRefresh);
	TestEqual(TEXT("at rest, 1 step later: nothing"), GetSendReason(Resting, Rest, EHOLUnitActivity::Idle, 7, 1, StepSeconds, Rules), EHOLUnitSendReason::None);
	TestEqual(TEXT("starts walking: sent at once"), GetSendReason(Resting, Rest, EHOLUnitActivity::Moving, 7, 1, StepSeconds, Rules), EHOLUnitSendReason::Activity);
	// J5c.1: the gathered resource is part of the activity.
	const FHOLUnitSentRecord Chopping = { FHOLNetUnitState::Make(Rest, FVector::ZeroVector, EHOLUnitActivity::Gathering, 0, EHOLResource::Food, 0, EHOLResource::Wood), 0 };
	TestEqual(TEXT("starts gathering: sent at once"), GetSendReason(Resting, Rest, EHOLUnitActivity::Gathering, 7, 1, StepSeconds, Rules, EHOLResource::Wood),
		EHOLUnitSendReason::Activity);
	TestEqual(TEXT("still gathering wood: nothing"), GetSendReason(Chopping, Rest, EHOLUnitActivity::Gathering, 7, 1, StepSeconds, Rules, EHOLResource::Wood),
		EHOLUnitSendReason::None);
	TestEqual(TEXT("gathering another resource: sent at once"), GetSendReason(Chopping, Rest, EHOLUnitActivity::Gathering, 7, 1, StepSeconds, Rules, EHOLResource::Stone),
		EHOLUnitSendReason::Activity);
	// 5 cm either side of the threshold (D274: 35 cm).
	const double Under = Rules.ErrorThreshold - 5.0;
	const double Past = Rules.ErrorThreshold + 5.0;
	TestEqual(TEXT("pushed under the threshold: nothing"), GetSendReason(Resting, Rest + FVector(Under, 0.0, 0.0), EHOLUnitActivity::Idle, 7, 1, StepSeconds, Rules), EHOLUnitSendReason::None);
	TestEqual(TEXT("pushed past the threshold"), GetSendReason(Resting, Rest + FVector(Past, 0.0, 0.0), EHOLUnitActivity::Idle, 7, 1, StepSeconds, Rules), EHOLUnitSendReason::Error);
	TestEqual(TEXT("height changed: nothing, the client finds the ground (D301)"), GetSendReason(Resting, Rest + FVector(0.0, 0.0, Past), EHOLUnitActivity::Idle, 7, 1, StepSeconds, Rules), EHOLUnitSendReason::None);

	// J7c.4, S5 in combat: only a walking unit is sent with a velocity; a stander is sent still.
	TestTrue(TEXT("a walking unit sends its velocity"), HOLUnitReplication::SendsVelocity(EHOLUnitActivity::Moving));
	TestTrue(TEXT("a gatherer on its round trip sends its velocity"), HOLUnitReplication::SendsVelocity(EHOLUnitActivity::GatherTrip));
	for (const EHOLUnitActivity Still : { EHOLUnitActivity::Idle, EHOLUnitActivity::Gathering, EHOLUnitActivity::Building,
		EHOLUnitActivity::Attacking, EHOLUnitActivity::Dying })
	{
		TestFalse(FString::Printf(TEXT("activity %d is sent still"), static_cast<int32>(Still)), HOLUnitReplication::SendsVelocity(Still));
	}
	// A fighter in a melee crush, pushed 40 to 100 cm/s in random directions around where it strikes (10 s): sent with its
	// step's move, the server sends it again at each error past the threshold the extrapolated jitter makes; sent still,
	// only once it really drifted that far. The rule measures the error from the velocity sent: it holds either way.
	{
		const auto CountSends = [&Rules, &Rest](const bool bStill, double& OutWorstError)
		{
			FRandomStream Random(1234);
			FVector Position = Rest;
			FHOLUnitSentRecord Sent = Send(Rest, FVector::ZeroVector, EHOLUnitActivity::Attacking, 0);
			int32 Sends = 0;
			OutWorstError = 0.0;
			for (int64 Step = 1; Step <= 200; ++Step)
			{
				// Pushed off in a random direction, and back towards its place as it keeps striking: a jitter around Rest.
				const double Angle = Random.FRandRange(0.f, 2.f * UE_PI);
				const FVector StepMove = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0) * Random.FRandRange(40.f, 100.f) + (Rest - Position) * 0.5;
				Position += StepMove * StepSeconds;
				if (GetSendReason(Sent, Position, EHOLUnitActivity::Attacking, 7, Step, StepSeconds, Rules) != EHOLUnitSendReason::None)
				{
					Sent = Send(Position, bStill ? FVector::ZeroVector : StepMove, EHOLUnitActivity::Attacking, Step);
					++Sends;
				}
				OutWorstError = FMath::Max(OutWorstError, FVector::Dist2D(HOLUnitReplication::PredictPosition(Sent.State, static_cast<double>(Step - Sent.Step), StepSeconds), Position));
			}
			return Sends;
		};
		double StepMoveWorst = 0.0;
		double StillWorst = 0.0;
		const int32 StepMoveSends = CountSends(false, StepMoveWorst);
		const int32 StillSends = CountSends(true, StillWorst);
		AddInfo(FString::Printf(TEXT("pushed fighter, 10 s: %d sends with its step's move (worst %.1f cm), %d sent still (worst %.1f cm)"),
			StepMoveSends, StepMoveWorst, StillSends, StillWorst));
		TestTrue(TEXT("pushed fighter: fewer sends sent still"), StillSends < StepMoveSends);
		TestTrue(TEXT("pushed fighter sent still: error held under the threshold"), StillWorst <= Rules.ErrorThreshold);
	}

	// Walking in a straight line: the client's prediction holds, only the refresh sends.
	const FVector Velocity(300.0, 120.0, 0.0);
	const FHOLUnitSentRecord Walking = Send(Rest, Velocity, EHOLUnitActivity::Moving, 0);
	bool bErrorOnStraightLine = false;
	for (int64 Step = 1; Step < Rules.RefreshSteps; ++Step)
	{
		const FVector Position = Rest + Velocity * (Step * StepSeconds);
		bErrorOnStraightLine |= GetSendReason(Walking, Position, EHOLUnitActivity::Moving, 7, Step, StepSeconds, Rules) == EHOLUnitSendReason::Error;
	}
	TestFalse(TEXT("straight line: no error send"), bErrorOnStraightLine);

	// Walking a circle (20 m wide, 3 m/s, 10 s), sent whenever the rule says: the error the client would show at each
	// step never stays past the threshold, and the sends stay near the ~1.5 per second of D269.
	{
		constexpr double Radius = 1000.0;
		constexpr double Speed = 300.0;
		const auto PositionAt = [](const int64 Step) { const double Angle = Speed / Radius * Step * StepSeconds; return FVector(Radius * FMath::Cos(Angle), Radius * FMath::Sin(Angle), 0.0); };
		const auto VelocityAt = [](const int64 Step) { const double Angle = Speed / Radius * Step * StepSeconds; return FVector(-Speed * FMath::Sin(Angle), Speed * FMath::Cos(Angle), 0.0); };
		FHOLUnitSentRecord Sent = Send(PositionAt(0), VelocityAt(0), EHOLUnitActivity::Moving, 0);
		int32 Sends = 0;
		double WorstError = 0.0;
		for (int64 Step = 1; Step <= 200; ++Step)
		{
			if (GetSendReason(Sent, PositionAt(Step), EHOLUnitActivity::Moving, 7, Step, StepSeconds, Rules) != EHOLUnitSendReason::None)
			{
				Sent = Send(PositionAt(Step), VelocityAt(Step), EHOLUnitActivity::Moving, Step);
				++Sends;
			}
			WorstError = FMath::Max(WorstError, FVector::Dist(HOLUnitReplication::PredictPosition(Sent.State, Step - Sent.Step, StepSeconds), PositionAt(Step)));
		}
		AddInfo(FString::Printf(TEXT("circle of 20 m at 3 m/s: %.1f sends per second, worst error %.1f cm"), Sends / 10.0, WorstError));
		TestTrue(TEXT("circle: error held under the threshold"), WorstError <= Rules.ErrorThreshold);
		TestTrue(TEXT("circle: fewer than 3 sends per second"), Sends < 30);
	}

	// 2 000 units at rest, all sent at step 0: the refresh spreads them over the cycle instead of 2 000 at one step.
	{
		constexpr int32 NumUnits = 2000;
		TArray<FHOLUnitSentRecord> Records;
		Records.Init(Resting, NumUnits);
		int32 MostInOneStep = 0;
		int32 LongestGap = 0;
		int32 Sends = 0;
		for (int64 Step = 1; Step <= 6 * Rules.RefreshSteps; ++Step)
		{
			int32 ThisStep = 0;
			for (int32 UnitId = 0; UnitId < NumUnits; ++UnitId)
			{
				FHOLUnitSentRecord& Record = Records[UnitId];
				if (GetSendReason(Record, Rest, EHOLUnitActivity::Idle, UnitId, Step, StepSeconds, Rules) != EHOLUnitSendReason::None)
				{
					LongestGap = FMath::Max(LongestGap, static_cast<int32>(Step - Record.Step));
					Record = Send(Rest, FVector::ZeroVector, EHOLUnitActivity::Idle, Step);
					++ThisStep;
				}
			}
			MostInOneStep = FMath::Max(MostInOneStep, ThisStep);
			Sends += ThisStep;
		}
		AddInfo(FString::Printf(TEXT("2 000 units at rest: at most %d sends in one step, %.2f sends per unit and per second"),
			MostInOneStep, Sends / (NumUnits * 6.0 * Rules.RefreshSteps * StepSeconds)));
		TestTrue(TEXT("refresh spread: at most twice the even share per step"), MostInOneStep <= 2 * NumUnits / Rules.RefreshSteps);
		TestTrue(TEXT("refresh never later than a cycle and a quarter"), LongestGap <= LongestRefresh);
		TestTrue(TEXT("at rest: one send per cycle"), FMath::IsNearlyEqual(Sends / (NumUnits * 6.0), 1.0, 0.1));
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetServerClockTest, "HeroesOfLiberty.Net.ServerClock", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetServerClockTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Net;
	FHOLServerClock Clock;
	Clock.Reset(StepSeconds);
	TestFalse(TEXT("no sample, no estimate"), Clock.IsValid());

	// Server step 100 received at local time 10 s with 40 ms of one-way delay: the server is at step 100.8.
	Clock.AddSample(100, 10.0, 0.04);
	TestTrue(TEXT("first sample taken as is"), Clock.IsValid() && FMath::IsNearlyEqual(Clock.GetStep(10.0), 100.8, 1.0e-6));
	TestTrue(TEXT("runs at the step rate"), FMath::IsNearlyEqual(Clock.GetStep(11.0), 120.8, 1.0e-6));

	// A sample 20 ms late (jitter): followed by a tenth only.
	Clock.AddSample(120, 11.02, 0.04);
	TestTrue(TEXT("jitter: followed slowly"), FMath::IsNearlyEqual(Clock.GetStep(11.0), 120.8 - 0.04, 1.0e-6));

	// Steady samples on a new delay: the estimate converges.
	for (int32 Index = 0; Index < 200; ++Index)
	{
		Clock.AddSample(200 + Index, 15.0 + Index * StepSeconds, 0.06);
	}
	TestTrue(TEXT("steady samples: converged"), FMath::IsNearlyEqual(Clock.GetStep(15.0 + 199 * StepSeconds), 399.0 + 1.2, 0.01));

	// A hitch of the client: the first step read after it is half a second late, then they are on time again. Ignored.
	const double BeforeHitch = Clock.GetStep(30.0);
	Clock.AddSample(400, 25.5, 0.06);
	TestTrue(TEXT("hitch: a far later sample is ignored"), FMath::IsNearlyEqual(Clock.GetStep(30.0), BeforeHitch, 1.0e-6));

	// The server lost a second for good: the samples stay a second later, followed after LateSamplesToFollow of them.
	for (int32 Index = 0; Index < FHOLServerClock::LateSamplesToFollow; ++Index)
	{
		Clock.AddSample(400 + Index, 26.0 + Index * StepSeconds, 0.06);
	}
	TestTrue(TEXT("lasting shift: followed"), FMath::IsNearlyEqual(Clock.GetStep(26.0 + 9 * StepSeconds), 409.0 + 1.2, 1.0e-6));

	// The server ahead by a second (a lost burst of samples): taken as is.
	Clock.AddSample(1000, 30.0, 0.0);
	TestTrue(TEXT("far earlier sample: taken as is"), FMath::IsNearlyEqual(Clock.GetStep(30.0), 1000.0, 1.0e-6));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetCorrectionTest, "HeroesOfLiberty.Net.Correction", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetCorrectionTest::RunTest(const FString& Parameters)
{
	FHOLDisplayCorrection Correction;
	TestEqual(TEXT("no correction: no offset"), Correction.GetOffset(0.0), FVector::ZeroVector);

	Correction.Start(FVector(120.0, 0.0, 0.0), FVector(100.0, 0.0, 0.0), 1.0);
	TestEqual(TEXT("starts where the unit is drawn"), Correction.GetOffset(1.0), FVector(20.0, 0.0, 0.0));
	TestTrue(TEXT("halfway: half the gap"), Correction.GetOffset(1.1).Equals(FVector(10.0, 0.0, 0.0), 1.0e-4));
	TestEqual(TEXT("after ~0.2 s: gone"), Correction.GetOffset(1.0 + FHOLDisplayCorrection::SmoothingSeconds), FVector::ZeroVector);
	TestEqual(TEXT("later: still gone"), Correction.GetOffset(5.0), FVector::ZeroVector);

	Correction.Start(FVector(1000.0, 0.0, 0.0), FVector::ZeroVector, 2.0);
	TestEqual(TEXT("a gap past the snap distance is not smoothed"), Correction.GetOffset(2.0), FVector::ZeroVector);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetDisplayedPositionTest, "HeroesOfLiberty.Net.DisplayedPosition", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetDisplayedPositionTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Net;
	FHOLNetUnitDisplayFragment Display;
	// Positions on the 11 cm step (D344): 1100 and 1232 cm travel exact.
	Display.State = FHOLNetUnitState::Make(FVector(1100.0, 0.0, 0.0), FVector(300.0, 0.0, 0.0), EHOLUnitActivity::Moving, 100);
	Display.StateStep = 100;
	TestEqual(TEXT("half a second after the state"), Display.GetDisplayedPosition(110.0, 0.0, StepSeconds), FVector(1250.0, 0.0, 0.0));
	TestEqual(TEXT("a lost state is not walked on past 3 s"), Display.GetDisplayedPosition(200.0, 0.0, StepSeconds),
		FVector(1100.0 + 300.0 * FHOLNetUnitDisplayFragment::MaxPredictionSeconds, 0.0, 0.0));
	Display.GroundZ = 50.0;
	TestEqual(TEXT("on the ground the client found (D301)"), Display.GetDisplayedPosition(110.0, 0.0, StepSeconds), FVector(1250.0, 0.0, 50.0));
	Display.GroundZ = 0.0;

	// A new state 18 cm behind where the unit is drawn: drawn where it was, then the gap fades out.
	const FVector Drawn = Display.GetDisplayedPosition(110.0, 5.0, StepSeconds);
	Display.State = FHOLNetUnitState::Make(FVector(1232.0, 0.0, 0.0), FVector(300.0, 0.0, 0.0), EHOLUnitActivity::Moving, 110);
	Display.StateStep = 110;
	Display.Correction.Start(Drawn, Display.GetDisplayedPosition(110.0, 5.0, StepSeconds), 5.0);
	TestTrue(TEXT("no jump at the new state"), Display.GetDisplayedPosition(110.0, 5.0, StepSeconds).Equals(Drawn, 1.0e-3));
	TestTrue(TEXT("on the new state once smoothed"), Display.GetDisplayedPosition(114.0, 5.2, StepSeconds).Equals(FVector(1292.0, 0.0, 0.0), 1.0e-3));
	return true;
}

// J7c, retour d'essai 8 (hit and run): a client replays a shooter's cadence (no bit sent, J7c-Q4); its cool-down keeps
// running down while the shooter walks, so its arrows fall on the server's steps across a stop, a walk and a new stop.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetShotReplayTest, "HeroesOfLiberty.Net.ShotReplay", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetShotReplayTest::RunTest(const FString& Parameters)
{
	constexpr int32 StepMs = 50;
	constexpr int32 IntervalMs = 1625;
	constexpr int32 WindUpPercent = 12;  // the Archer's (Tools/Data/UnitTypes.csv)
	// The server's steps: Attacking on [AttackFrom, AttackTo] spans, idle between (TickIdle), from step 100.
	const auto ServerShots = [](TConstArrayView<TPair<int64, int64>> Spans)
	{
		TArray<int64> Shots;
		int32 Cooldown = 0;
		bool bAttacking = false;
		for (int64 Step = 100; Step <= Spans.Last().Value; ++Step)
		{
			const bool bInSpan = Algo::AnyOf(Spans, [Step](const TPair<int64, int64>& Span) { return Step >= Span.Key && Step <= Span.Value; });
			if (!bInSpan)
			{
				bAttacking = false;
				HOLCombat::TickIdle(Cooldown, StepMs);
				continue;
			}
			if (!bAttacking)
			{
				Cooldown = HOLCombat::StartAttack(Cooldown, IntervalMs, WindUpPercent);
				bAttacking = true;
			}
			if (HOLCombat::TickAttack(Cooldown, StepMs, IntervalMs))
			{
				Shots.Add(Step);
			}
		}
		return Shots;
	};
	// The client: told each Attacking state at its step, replays the steps after it (as UHOLNetUnitDisplayProcessor).
	const auto ClientShots = [](TConstArrayView<TPair<int64, int64>> Spans)
	{
		TArray<int64> Shots;
		FHOLNetUnitDisplayFragment Display;
		for (const TPair<int64, int64>& Span : Spans)
		{
			Display.StartShots(Span.Key, StepMs, IntervalMs, WindUpPercent);
			while (Display.ShotStep < Span.Value)
			{
				++Display.ShotStep;
				if (HOLCombat::TickAttack(Display.ShotCooldownMs, StepMs, IntervalMs))
				{
					Shots.Add(Display.ShotStep);
				}
			}
			Display.StopShots();
		}
		return Shots;
	};
	const auto Check = [this, &ServerShots, &ClientShots](const TCHAR* What, TConstArrayView<TPair<int64, int64>> Spans)
	{
		const TArray<int64> Server = ServerShots(Spans);
		const TArray<int64> Client = ClientShots(Spans);
		FString Steps;
		for (const int64 Shot : Server)
		{
			Steps += FString::Printf(TEXT(" %lld"), Shot);
		}
		TestTrue(FString::Printf(TEXT("%s: the client's arrows on the server's steps (%s)"), What, *Steps), Server == Client && !Server.IsEmpty());
	};
	const TPair<int64, int64> Stand[] = { { 100, 200 } };
	Check(TEXT("standing"), Stand);
	// Shoots at 103, walks 0.5 s (111 to 120), stops: the cool-down left (770 ms) and not the wind-up.
	const TPair<int64, int64> ShortWalk[] = { { 100, 110 }, { 121, 200 } };
	Check(TEXT("short walk"), ShortWalk);
	const TArray<int64> ShortWalkShots = ServerShots(ShortWalk);
	TestTrue(TEXT("short walk: the next arrow as had it stood (step 136), not a wind-up after the stop (124)"),
		ShortWalkShots.Num() >= 2 && ShortWalkShots[0] == 103 && ShortWalkShots[1] == 136);
	const TPair<int64, int64> LongWalk[] = { { 100, 110 }, { 171, 260 } };
	Check(TEXT("long walk: reloaded, the wind-up"), LongWalk);
	const TPair<int64, int64> Several[] = { { 100, 109 }, { 115, 118 }, { 130, 160 }, { 162, 162 }, { 165, 240 } };
	Check(TEXT("several stops"), Several);
	return true;
}

// J7b.4 (J7-Q5 revised): a change of health, of target or of front is a change of action, sent at once: every hit.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetCombatSendRuleTest, "HeroesOfLiberty.Net.CombatSendRule", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetCombatSendRuleTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Net;
	const FHOLUnitSendRules Rules;
	const FVector Here(1000.0, 2000.0, 0.0);
	const auto Make = [&Here](const EHOLUnitActivity Activity, const int32 Health, const uint16 TargetSlot, const bool bFront = false, const float Yaw = 0.f)
	{
		FHOLNetUnitState State = FHOLNetUnitState::Make(Here, FVector::ZeroVector, Activity, 100);
		State.SetCombat(Health, 160, TargetSlot, bFront, Yaw);
		return State;
	};
	// UnitId 1 and step 101: not its refresh step (40 steps cycle).
	const FHOLUnitSentRecord Sent{ Make(EHOLUnitActivity::Attacking, 150, 7), 100 };
	TestEqual(TEXT("same health: nothing"), HOLUnitReplication::GetSendReason(Sent, Here, Make(EHOLUnitActivity::Attacking, 150, 7), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::None);
	TestEqual(TEXT("one point less (150 -> 149, a level kept it): at once"), HOLUnitReplication::GetSendReason(Sent, Here, Make(EHOLUnitActivity::Attacking, 149, 7), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	TestEqual(TEXT("healed one point (150 -> 151, the same field): at once"), HOLUnitReplication::GetSendReason(Sent, Here, Make(EHOLUnitActivity::Attacking, 151, 7), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	TestEqual(TEXT("another target: at once"), HOLUnitReplication::GetSendReason(Sent, Here, Make(EHOLUnitActivity::Attacking, 150, 8), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	TestEqual(TEXT("stops striking: at once"), HOLUnitReplication::GetSendReason(Sent, Here, Make(EHOLUnitActivity::Idle, 150, 7), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	TestEqual(TEXT("dies: at once"), HOLUnitReplication::GetSendReason(Sent, Here, Make(EHOLUnitActivity::Dying, 0, 7), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	const FHOLUnitSentRecord Resting{ Make(EHOLUnitActivity::Idle, 160, 0, true, 90.f), 100 };
	TestEqual(TEXT("same front: nothing"), HOLUnitReplication::GetSendReason(Resting, Here, Make(EHOLUnitActivity::Idle, 160, 0, true, 91.f), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::None);
	TestEqual(TEXT("another front: at once"), HOLUnitReplication::GetSendReason(Resting, Here, Make(EHOLUnitActivity::Idle, 160, 0, true, 180.f), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	TestEqual(TEXT("first wound: at once"), HOLUnitReplication::GetSendReason(Resting, Here, Make(EHOLUnitActivity::Idle, 159, 0, true, 90.f), 1, 101, StepSeconds, Rules),
		EHOLUnitSendReason::Activity);
	// The target's slot (TargetSlotBits): 0 to 4 093 real, 4 094 a building (D345.2b), InvalidTargetSlot unknown though a
	// slot from the 66th chunk's last on.
	TestTrue(TEXT("target in slot 0: known"), Make(EHOLUnitActivity::Attacking, 150, 0).HasTargetSlot());
	TestTrue(TEXT("target in slot 4093: known"), Make(EHOLUnitActivity::Attacking, 150, 4093).HasTargetSlot());
	TestFalse(TEXT("slot 4094 is a building, not a unit"), Make(EHOLUnitActivity::Attacking, 150, FHOLNetUnitState::BuildingTargetSlot).HasTargetSlot());
	TestTrue(TEXT("slot 4094: a building target"), Make(EHOLUnitActivity::Attacking, 150, FHOLNetUnitState::BuildingTargetSlot).HasBuildingTarget());
	TestFalse(TEXT("a unit target is not a building"), Make(EHOLUnitActivity::Attacking, 150, 7).HasBuildingTarget());
	TestFalse(TEXT("target not replicated yet: unknown"), Make(EHOLUnitActivity::Attacking, 150, FHOLNetUnitState::InvalidTargetSlot).HasTargetSlot());
	TestFalse(TEXT("target in slot 5000, past 12 bits: unknown"), Make(EHOLUnitActivity::Attacking, 150, 5000).HasTargetSlot());
	TestFalse(TEXT("not attacking: no target"), Make(EHOLUnitActivity::Idle, 150, 7).HasTargetSlot());
	return true;
}

// R3.4 (R-Q7): who may cheat. The solo game keeps its cheats; a server refuses a client's unless launched with -HOLCheats;
// hol.Cheat.Refuse refuses even then.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetCheatRuleTest, "HeroesOfLiberty.Net.CheatRule", UE::HeroesOfLiberty::Tests::Net::Flags)

bool FHOLNetCheatRuleTest::RunTest(const FString& Parameters)
{
	TestTrue(TEXT("solo: allowed"), FHOLCheatRules::AreCheatsAllowed(NM_Standalone, false, false));
	TestFalse(TEXT("dedicated server: refused"), FHOLCheatRules::AreCheatsAllowed(NM_DedicatedServer, false, false));
	TestFalse(TEXT("listen server: refused"), FHOLCheatRules::AreCheatsAllowed(NM_ListenServer, false, false));
	TestTrue(TEXT("dedicated server with -HOLCheats: allowed"), FHOLCheatRules::AreCheatsAllowed(NM_DedicatedServer, true, false));
	TestTrue(TEXT("listen server with -HOLCheats: allowed"), FHOLCheatRules::AreCheatsAllowed(NM_ListenServer, true, false));
	TestFalse(TEXT("dedicated server with -HOLCheats and hol.Cheat.Refuse: refused"), FHOLCheatRules::AreCheatsAllowed(NM_DedicatedServer, true, true));
	TestFalse(TEXT("client: never (its cheats go to the server)"), FHOLCheatRules::AreCheatsAllowed(NM_Client, true, false));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetManagerRoutesTest, "HeroesOfLiberty.Net.ManagerRoutes", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetManagerRoutesTest::RunTest(const FString& Parameters)
{
	// R3.6, R3.7: the routes and bodies of Manager/PROTOCOL.md ("Routes du serveur Unreal").
	TestEqual(TEXT("listening route"), FHOLManagerRules::MakeRouteUrl(TEXT("http://127.0.0.1:7700"), TEXT("M20261023-153012-3"), TEXT("listening")),
		FString(TEXT("http://127.0.0.1:7700/matches/M20261023-153012-3/listening")));
	TestEqual(TEXT("a trailing slash of the manager's address dropped"), FHOLManagerRules::MakeRouteUrl(TEXT("http://127.0.0.1:7700/"), TEXT("QC1"), TEXT("empty")),
		FString(TEXT("http://127.0.0.1:7700/matches/QC1/empty")));
	TestEqual(TEXT("listening body: the port"), FHOLManagerRules::MakeListeningBody(7778), FString(TEXT("{\"port\":7778}")));
	TestEqual(TEXT("finished body: the winning slots"), FHOLManagerRules::MakeFinishedBody({ 0, 2 }), FString(TEXT("{\"winners\":[0,2]}")));
	TestEqual(TEXT("finished body: no winner"), FHOLManagerRules::MakeFinishedBody({}), FString(TEXT("{\"winners\":[]}")));
	TestEqual(TEXT("started and empty: an empty object"), FHOLManagerRules::MakeEmptyBody(), FString(TEXT("{}")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLNetServerQuitTest, "HeroesOfLiberty.Net.ServerQuit", UE::HeroesOfLiberty::Tests::Net::Flags)
bool FHOLNetServerQuitTest::RunTest(const FString& Parameters)
{
	// R3.8 (M6-Q9): one match per server; a server launched by the manager quits by itself.
	using EQuit = FHOLManagerRules::EQuit;
	auto Check = [this](const TCHAR* What, const EQuit Got, const EQuit Expected) { TestEqual(What, (int32)Got, (int32)Expected); };
	Check(TEXT("managed, empty after the start: quits"), FHOLManagerRules::ShouldQuit(true, true, 0, -1.0, 120.0), EQuit::Empty);
	Check(TEXT("managed, empty before the start (waiting for players): stays"), FHOLManagerRules::ShouldQuit(true, false, 0, -1.0, 120.0), EQuit::None);
	Check(TEXT("managed, players in the match: stays"), FHOLManagerRules::ShouldQuit(true, true, 2, -1.0, 120.0), EQuit::None);
	Check(TEXT("managed, finished 60 s ago with players: stays"), FHOLManagerRules::ShouldQuit(true, true, 2, 60.0, 120.0), EQuit::None);
	Check(TEXT("managed, finished 120 s ago with players: quits"), FHOLManagerRules::ShouldQuit(true, true, 2, 120.0, 120.0), EQuit::Finished);
	Check(TEXT("managed, finished and empty: quits at once"), FHOLManagerRules::ShouldQuit(true, true, 0, 5.0, 120.0), EQuit::Empty);
	Check(TEXT("not managed (solo, tests, net_match.ps1), empty after the start: stays"), FHOLManagerRules::ShouldQuit(false, true, 0, -1.0, 120.0), EQuit::None);
	Check(TEXT("not managed, finished long ago: stays"), FHOLManagerRules::ShouldQuit(false, true, 2, 600.0, 120.0), EQuit::None);
	return true;
}

#endif
