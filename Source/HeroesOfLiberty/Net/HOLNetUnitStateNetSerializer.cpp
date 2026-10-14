// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLNetUnitStateNetSerializer.h"

#include "Iris/ReplicationState/PropertyNetSerializerInfoRegistry.h"
#include "Iris/Serialization/NetBitStreamReader.h"
#include "Iris/Serialization/NetBitStreamWriter.h"
#include "Iris/Serialization/NetSerializerDelegates.h"
#include "Misc/ScopeExit.h"
#include "Net/HOLUnitReplicationRules.h"

#include <atomic>

namespace UE::Net
{

struct FHOLNetUnitStateNetSerializer
{
	static constexpr uint32 Version = 0;

	/** The same integers, as plain data (the USTRUCT's default values make it non-POD for Iris). */
	struct FQuantizedType
	{
		int16 X;
		int16 Y;
		int16 VelocityX;
		int16 VelocityY;
		uint8 Activity;
		uint8 Step;
	};

	typedef FHOLNetUnitState SourceType;
	typedef FQuantizedType QuantizedType;
	typedef FHOLNetUnitStateNetSerializerConfig ConfigType;

	inline static const ConfigType DefaultConfig;

	static void Serialize(FNetSerializationContext& Context, const FNetSerializeArgs& Args);
	static void Deserialize(FNetSerializationContext& Context, const FNetDeserializeArgs& Args);
	static void Quantize(FNetSerializationContext& Context, const FNetQuantizeArgs& Args);
	static void Dequantize(FNetSerializationContext& Context, const FNetDequantizeArgs& Args);
	static bool IsEqual(FNetSerializationContext& Context, const FNetIsEqualArgs& Args);
	static bool Validate(FNetSerializationContext& Context, const FNetValidateArgs& Args);

	class FNetSerializerRegistryDelegates final : private UE::Net::FNetSerializerRegistryDelegates
	{
	public:
		virtual ~FNetSerializerRegistryDelegates();

	private:
		virtual void OnPreFreezeNetSerializerRegistry() override;
	};

	static FHOLNetUnitStateNetSerializer::FNetSerializerRegistryDelegates NetSerializerRegistryDelegates;

private:
	static void WriteSigned(FNetBitStreamWriter& Writer, const int32 Value, const uint32 Bits)
	{
		Writer.WriteBits(static_cast<uint32>(Value) & ((1U << Bits) - 1U), Bits);
	}

	static int16 ReadSigned(FNetBitStreamReader& Reader, const uint32 Bits)
	{
		const int32 SignBit = 1 << (Bits - 1U);
		return static_cast<int16>((static_cast<int32>(Reader.ReadBits(Bits)) ^ SignBit) - SignBit);
	}

	static bool FitsIn(const int32 Value, const uint32 Bits)
	{
		const int32 Max = (1 << (Bits - 1U)) - 1;
		return Value >= -Max - 1 && Value <= Max;
	}
};

UE_NET_IMPLEMENT_SERIALIZER(FHOLNetUnitStateNetSerializer);

FHOLNetUnitStateNetSerializer::FNetSerializerRegistryDelegates FHOLNetUnitStateNetSerializer::NetSerializerRegistryDelegates;

namespace HOLNetUnitState
{
	/** Iris may serialize for several connections in parallel. */
	std::atomic<int64> NumSerialized = 0;
	std::atomic<int64> NumDeserialized = 0;

	UE::HeroesOfLiberty::Net::FHOLUnitStateBitProbe BitProbe;
	/** Absolute position in the received stream after the last state read. */
	uint32 LastStateEndBits = 0;

	void ProbeState(const uint32 StartBits, const uint32 EndBits)
	{
		using FProbe = UE::HeroesOfLiberty::Net::FHOLUnitStateBitProbe;
		++BitProbe.NumStates;
		if (StartBits == LastStateEndBits)
		{
			++BitProbe.NumAdjacent;
		}
		else if (StartBits < LastStateEndBits)
		{
			++BitProbe.NumPacketStarts;
			BitProbe.PacketStartBits += StartBits;
		}
		else if (const uint32 Gap = StartBits - LastStateEndBits; Gap < FProbe::NumBuckets * FProbe::BucketBits)
		{
			++BitProbe.NumGaps;
			BitProbe.GapBits += Gap;
			++BitProbe.GapHistogram[Gap / FProbe::BucketBits];
		}
		else
		{
			++BitProbe.NumOther;
		}
		LastStateEndBits = EndBits;
	}
}

void FHOLNetUnitStateNetSerializer::Serialize(FNetSerializationContext& Context, const FNetSerializeArgs& Args)
{
	HOLNetUnitState::NumSerialized.fetch_add(1, std::memory_order_relaxed);
	const QuantizedType& State = *reinterpret_cast<const QuantizedType*>(Args.Source);
	FNetBitStreamWriter& Writer = *Context.GetBitStreamWriter();
	WriteSigned(Writer, State.X, SourceType::HorizontalBits);
	WriteSigned(Writer, State.Y, SourceType::HorizontalBits);
	WriteSigned(Writer, State.VelocityX, SourceType::VelocityBits);
	WriteSigned(Writer, State.VelocityY, SourceType::VelocityBits);
	Writer.WriteBits(State.Activity, SourceType::ActivityBits);
	Writer.WriteBits(State.Step, SourceType::StepBits);
}

void FHOLNetUnitStateNetSerializer::Deserialize(FNetSerializationContext& Context, const FNetDeserializeArgs& Args)
{
	HOLNetUnitState::NumDeserialized.fetch_add(1, std::memory_order_relaxed);
	FNetBitStreamReader& Reader = *Context.GetBitStreamReader();
	const uint32 StartBits = Reader.GetAbsolutePosBits();
	ON_SCOPE_EXIT { HOLNetUnitState::ProbeState(StartBits, Reader.GetAbsolutePosBits()); };
	QuantizedType& State = *reinterpret_cast<QuantizedType*>(Args.Target);
	State.X = ReadSigned(Reader, SourceType::HorizontalBits);
	State.Y = ReadSigned(Reader, SourceType::HorizontalBits);
	State.VelocityX = ReadSigned(Reader, SourceType::VelocityBits);
	State.VelocityY = ReadSigned(Reader, SourceType::VelocityBits);
	State.Activity = static_cast<uint8>(Reader.ReadBits(SourceType::ActivityBits));
	State.Step = static_cast<uint8>(Reader.ReadBits(SourceType::StepBits));
}

void FHOLNetUnitStateNetSerializer::Quantize(FNetSerializationContext& Context, const FNetQuantizeArgs& Args)
{
	const SourceType& Source = *reinterpret_cast<const SourceType*>(Args.Source);
	QuantizedType& Target = *reinterpret_cast<QuantizedType*>(Args.Target);
	Target.X = Source.X;
	Target.Y = Source.Y;
	Target.VelocityX = Source.VelocityX;
	Target.VelocityY = Source.VelocityY;
	Target.Activity = static_cast<uint8>(Source.Activity);
	Target.Step = Source.Step;
}

void FHOLNetUnitStateNetSerializer::Dequantize(FNetSerializationContext& Context, const FNetDequantizeArgs& Args)
{
	const QuantizedType& Source = *reinterpret_cast<const QuantizedType*>(Args.Source);
	SourceType& Target = *reinterpret_cast<SourceType*>(Args.Target);
	Target.X = Source.X;
	Target.Y = Source.Y;
	Target.VelocityX = Source.VelocityX;
	Target.VelocityY = Source.VelocityY;
	Target.Activity = static_cast<EHOLUnitActivity>(Source.Activity);
	Target.Step = Source.Step;
}

bool FHOLNetUnitStateNetSerializer::IsEqual(FNetSerializationContext& Context, const FNetIsEqualArgs& Args)
{
	if (Args.bStateIsQuantized)
	{
		const QuantizedType& A = *reinterpret_cast<const QuantizedType*>(Args.Source0);
		const QuantizedType& B = *reinterpret_cast<const QuantizedType*>(Args.Source1);
		return A.X == B.X && A.Y == B.Y && A.VelocityX == B.VelocityX && A.VelocityY == B.VelocityY
			&& A.Activity == B.Activity && A.Step == B.Step;
	}
	return *reinterpret_cast<const SourceType*>(Args.Source0) == *reinterpret_cast<const SourceType*>(Args.Source1);
}

bool FHOLNetUnitStateNetSerializer::Validate(FNetSerializationContext& Context, const FNetValidateArgs& Args)
{
	// FHOLNetUnitState::Make holds every value in its range; a state built otherwise would be cut on the wire.
	const SourceType& State = *reinterpret_cast<const SourceType*>(Args.Source);
	return FitsIn(State.X, SourceType::HorizontalBits) && FitsIn(State.Y, SourceType::HorizontalBits)
		&& FitsIn(State.VelocityX, SourceType::VelocityBits)
		&& FitsIn(State.VelocityY, SourceType::VelocityBits)
		&& static_cast<uint32>(State.Activity) < (1U << SourceType::ActivityBits);
}

static const FName PropertyNetSerializerRegistry_NAME_HOLNetUnitState("HOLNetUnitState");
UE_NET_IMPLEMENT_NAMED_STRUCT_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_HOLNetUnitState, FHOLNetUnitStateNetSerializer);

FHOLNetUnitStateNetSerializer::FNetSerializerRegistryDelegates::~FNetSerializerRegistryDelegates()
{
	UE_NET_UNREGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_HOLNetUnitState);
}

void FHOLNetUnitStateNetSerializer::FNetSerializerRegistryDelegates::OnPreFreezeNetSerializerRegistry()
{
	UE_NET_REGISTER_NETSERIALIZER_INFO(PropertyNetSerializerRegistry_NAME_HOLNetUnitState);
}

}

int64 UE::HeroesOfLiberty::Net::GetNumSerializedUnitStates()
{
	return UE::Net::HOLNetUnitState::NumSerialized.load(std::memory_order_relaxed);
}

int64 UE::HeroesOfLiberty::Net::GetNumDeserializedUnitStates()
{
	return UE::Net::HOLNetUnitState::NumDeserialized.load(std::memory_order_relaxed);
}

int32 UE::HeroesOfLiberty::Net::FHOLUnitStateBitProbe::GapPercentile(const double Share) const
{
	const int64 Wanted = FMath::CeilToInt64(NumGaps * Share);
	int64 Count = 0;
	for (int32 Bucket = 0; Bucket < NumBuckets; ++Bucket)
	{
		Count += GapHistogram[Bucket];
		if (Count >= Wanted && Count > 0)
		{
			return (Bucket + 1) * BucketBits;
		}
	}
	return NumBuckets * BucketBits;
}

const UE::HeroesOfLiberty::Net::FHOLUnitStateBitProbe& UE::HeroesOfLiberty::Net::GetUnitStateBitProbe()
{
	return UE::Net::HOLNetUnitState::BitProbe;
}

void UE::HeroesOfLiberty::Net::ResetUnitStateBitProbe()
{
	UE::Net::HOLNetUnitState::BitProbe = FHOLUnitStateBitProbe();
}
