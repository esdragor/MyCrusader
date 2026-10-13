// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * What the scripted checks (hol.Test.Run) ask of the interface, which lives in a module this one does not depend on
 * (D247): the interface module fills these in when its root layout is built. Unset on a dedicated server.
 */
namespace HOLTestHooks
{
	/** True while a screen of the interface is still fading in or out. */
	HEROESOFLIBERTY_API TFunction<bool()>& IsInterfaceBusy();

	/** The screens on top of the interface layers, for the log of a capture ("menu=X modal=Y"). */
	HEROESOFLIBERTY_API TFunction<FString()>& DescribeInterface();
}
