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
