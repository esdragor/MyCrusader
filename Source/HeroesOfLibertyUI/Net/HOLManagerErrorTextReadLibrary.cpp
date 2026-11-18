// Copyright Epic Games, Inc. All Rights Reserved.

#include "Net/HOLManagerErrorTextReadLibrary.h"

#include "Net/HOLManagerErrorText.h"

namespace
{
	FHOLManagerError ToRule(const FHOLManagerErrorValue& Value)
	{
		FHOLManagerError Error;
		Error.Kind = static_cast<FHOLManagerError::EKind>(static_cast<uint8>(Value.Kind));
		Error.HttpCode = Value.HttpCode;
		Error.Code = Value.Code;
		Error.Message = Value.Message;
		return Error;
	}

	FHOLManagerErrorValue ToValue(const FHOLManagerError& Error)
	{
		FHOLManagerErrorValue Value;
		Value.Kind = static_cast<EHOLManagerErrorKind>(static_cast<uint8>(Error.Kind));
		Value.HttpCode = Error.HttpCode;
		Value.Code = Error.Code;
		Value.Message = Error.Message;
		return Value;
	}
}

FText UHOLManagerErrorTextReadLibrary::GetManagerErrorText(const FHOLManagerErrorValue& Error)
{
	return ::GetManagerErrorText(ToRule(Error));
}

FHOLManagerErrorValue UHOLManagerErrorTextReadLibrary::MakeVersionMismatchError(const FString& Served, const FString& Ours)
{
	return ToValue(::MakeVersionMismatchError(Served, Ours));
}

FHOLManagerErrorValue UHOLManagerErrorTextReadLibrary::MakeMatchLossError(const TEnumAsByte<ENetworkFailure::Type> Failure, const FString& Message)
{
	return ToValue(::MakeMatchLossError(Failure.GetValue(), Message));
}

FHOLManagerErrorValue UHOLManagerErrorTextReadLibrary::MakeMatchTravelError(const FString& Message)
{
	return ToValue(::MakeMatchTravelError(Message));
}

FHOLManagerErrorValue UHOLManagerErrorTextReadLibrary::MakeReturnToLobbyError(const FHOLManagerErrorValue& JoinError)
{
	return ToValue(::MakeReturnToLobbyError(ToRule(JoinError)));
}

bool UHOLManagerErrorTextReadLibrary::CanCreateLobbyAfter(const FHOLManagerErrorValue& ListError)
{
	return ::CanCreateLobbyAfter(ToRule(ListError));
}
