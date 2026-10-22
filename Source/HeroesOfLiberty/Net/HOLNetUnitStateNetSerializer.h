// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Iris/Serialization/NetSerializer.h"
#include "HOLNetUnitStateNetSerializer.generated.h"

/**
 * Element format of the unit replication (D270): Iris writes an FHOLNetUnitState in the exact bit widths it declares,
 * 57 bits (D301, D343, D344), instead of 16 bits per integer.
 */
USTRUCT()
struct FHOLNetUnitStateNetSerializerConfig : public FNetSerializerConfig
{
	GENERATED_BODY()
};

namespace UE::Net
{
	UE_NET_DECLARE_SERIALIZER(FHOLNetUnitStateNetSerializer, HEROESOFLIBERTY_API);
}

namespace UE::HeroesOfLiberty::Net
{
	/** Unit states written to the wire since start, all connections together (S5: what is really sent). */
	HEROESOFLIBERTY_API int64 GetNumSerializedUnitStates();
	/** Unit states read off the wire since start (S5, client: what really arrived). */
	HEROESOFLIBERTY_API int64 GetNumDeserializedUnitStates();

	/**
	 * Where the states read by the client sit in what Iris received (D300): the bits between two states are what Iris
	 * wrote between them. None between two states of one array write; a chunk's headers between the last state of a
	 * chunk and the first of the next one in the same packet. Game thread (client) only.
	 */
	struct FHOLUnitStateBitProbe
	{
		static constexpr int32 BucketBits = 1;
		static constexpr int32 NumBuckets = 1024;

		int64 NumStates = 0;
		/** States right after the previous one. */
		int64 NumAdjacent = 0;
		/** States after a gap of less than NumBuckets * BucketBits bits: the first state of a chunk write. */
		int64 NumGaps = 0;
		int64 GapBits = 0;
		int32 GapHistogram[NumBuckets] = {};
		/** States before the previous one in the stream: the first state of a packet, at that many bits from its start. */
		int64 NumPacketStarts = 0;
		int64 PacketStartBits = 0;
		/** Larger gaps (other objects in between). */
		int64 NumOther = 0;

		/** Gap (bits) below which that share of the gaps fall. */
		int32 GapPercentile(double Share) const;
	};
	HEROESOFLIBERTY_API const FHOLUnitStateBitProbe& GetUnitStateBitProbe();
	HEROESOFLIBERTY_API void ResetUnitStateBitProbe();
}
