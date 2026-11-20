// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLSignalInputReadLibrary.h"

int32 UHOLSignalInputReadLibrary::KindOfAltClick(const FVector2D& Press, const FVector2D& Release)
{
	return HOLSignalInput::KindOfAltClick(Press, Release);
}

void UHOLSignalInputReadLibrary::Expire(TArray<FHOLReceivedSignal>& Signals, const double Now)
{
	HOLSignalInput::Expire(Signals, Now);
}

FLinearColor UHOLSignalInputReadLibrary::ColorOf(const int32 Kind)
{
	return HOLSignalInput::ColorOf(Kind);
}

double UHOLSignalInputReadLibrary::PulseScale(const double AgeSeconds)
{
	return HOLSignalInput::PulseScale(AgeSeconds);
}

double UHOLSignalInputReadLibrary::MarkLifetime()
{
	return HOLSignalInput::MarkLifetime();
}

double UHOLSignalInputReadLibrary::PulseMin()
{
	return HOLSignalInput::PulseMin();
}

double UHOLSignalInputReadLibrary::PulseMax()
{
	return HOLSignalInput::PulseMax();
}

FHOLReceivedSignal UHOLSignalInputReadLibrary::MakeSignal(const int32 Kind, const double ReceivedAt)
{
	FHOLReceivedSignal Signal;
	Signal.Kind = Kind;
	Signal.ReceivedAt = ReceivedAt;
	return Signal;
}
