// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLSignalInput.h"

#include "HAL/IConsoleManager.h"

namespace UE::HeroesOfLiberty::SignalInput
{
	static TAutoConsoleVariable<float> CVarSignalWheelPixels(TEXT("hol.Signal.WheelPixels"), 16.f,
		TEXT("SG3: an Alt + click that moved less than this (pixels) places the neutral signal; a longer drag picks the wheel's kind by its direction"));
	static TAutoConsoleVariable<float> CVarSignalIconSeconds(TEXT("hol.Signal.IconSeconds"), 5.f,
		TEXT("SG3: how long a received signal's icon stays on the minimap, seconds (D784: 5 s, the board's life)"));
	static TAutoConsoleVariable<int32> CVarSignalIconStyle(TEXT("hol.Signal.IconStyle"), 0,
		TEXT("SG3 (Retours/Q-signals-ui.md Q2): 0 one colour per kind (A), 1 all white"));
}

int32 HOLSignalInput::KindOfAltClick(const FVector2D& Press, const FVector2D& Release)
{
	using namespace UE::HeroesOfLiberty::SignalInput;
	const FVector2D Drag = Release - Press;
	if (Drag.Size() < FMath::Max(CVarSignalWheelPixels.GetValueOnGameThread(), 1.f))
	{
		return 0;
	}
	// Clockwise from the screen's top (y grows downwards), five sectors of 72 degrees centred on 0, 72, 144, 216, 288.
	double Degrees = FMath::RadiansToDegrees(FMath::Atan2(Drag.X, -Drag.Y));
	if (Degrees < 0.0)
	{
		Degrees += 360.0;
	}
	return 1 + FMath::FloorToInt32((Degrees + 36.0) / 72.0) % 5;
}

void HOLSignalInput::Expire(TArray<FHOLReceivedSignal>& Signals, const double Now)
{
	const double Seconds = FMath::Max(UE::HeroesOfLiberty::SignalInput::CVarSignalIconSeconds.GetValueOnGameThread(), 0.1f);
	Signals.RemoveAll([Now, Seconds](const FHOLReceivedSignal& Signal) { return Now - Signal.ReceivedAt >= Seconds; });
}

FLinearColor HOLSignalInput::ColorOf(const int32 Kind)
{
	if (UE::HeroesOfLiberty::SignalInput::CVarSignalIconStyle.GetValueOnGameThread() == 1)
	{
		return FLinearColor::White;
	}
	// Q2 default A: neutral white, attack red, defend blue, need help orange, rally green, scout yellow.
	static const FLinearColor Colors[] = {
		FLinearColor::White, FLinearColor(0.95f, 0.15f, 0.1f), FLinearColor(0.2f, 0.45f, 1.f),
		FLinearColor(1.f, 0.55f, 0.f), FLinearColor(0.2f, 0.85f, 0.25f), FLinearColor(1.f, 0.9f, 0.15f) };
	return Colors[FMath::Clamp(Kind, 0, 5)];
}
