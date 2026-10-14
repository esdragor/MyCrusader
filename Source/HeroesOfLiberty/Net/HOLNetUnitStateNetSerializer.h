// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Iris/Serialization/NetSerializer.h"
#include "HOLNetUnitStateNetSerializer.generated.h"

/**
 * Element format of the unit replication (D270): Iris writes an FHOLNetUnitState in the exact bit widths it declares,
 * 71 bits, instead of 16 bits per integer.
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
}
