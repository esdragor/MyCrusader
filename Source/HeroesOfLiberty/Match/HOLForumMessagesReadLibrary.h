// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Match/HOLForumMessages.h"
#include "UObject/Object.h"
#include "HOLForumMessagesReadLibrary.generated.h"

/**
 * What a test or the script says of a match for the messages of the ruin (the part of FHOLMatchConfig the messages read): slot by slot
 * (index = slot, a missing entry = Closed / team 0 / no name), the state (0 Closed, 1 Human, 2 AI), the team and the lobby's name.
 * Every AI is Normal. Wrapper data only: no rule here.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumScenario
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	TArray<int32> States;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	TArray<int32> Teams;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	TArray<FString> Names;
};

/** FHOLForumEvent with plain ints: Kind 0 Fell, 1 HoldStarted, 2 Razed, 3 Defeated, 4 KingDied; Reason 0 None, 1 Disconnected. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumEventValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	int32 Kind = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	int32 Slot = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	int32 OtherSlot = -1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	int32 Lives = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	int32 Reason = 0;
};

/** FHOLForumMessage for the script: Source = Text.BuildSourceString(), Display = Text.ToString() (the current culture's). */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumMessageValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	FString Source;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	FString Display;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	bool bAlert = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	TArray<int32> NamedSlots;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	FString Shown;
};

/** HOLForumMessages::FPiece for the script. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLForumPieceValue
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	FString Text;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroesOfLiberty|ForumMessages")
	int32 Slot = -1;
};

/**
 * The messages of the ruin (HOLForumMessages, Match/HOLForumMessages.h, C++, same module) for Angelscript (namespace
 * HOLForumMessagesRead there): D836 keeps the rule in C++, the script reads it here so that HOLForumMessagesTests.cpp can be ported
 * (socle step 1, D830, D839). Wrappers only: no rule here. The thread and the alert, stateful C++ structs, are held by the two small
 * UObjects below.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLForumMessagesReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLForumMessages::GetMessage for Event, the camps and names of Scenario, as ViewerSlot sees it (INDEX_NONE: a spectator). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static FHOLForumMessageValue GetMessage(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, int32 ViewerSlot, bool bSeesForum);

	/** HOLForumMessages::ElidesDe. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static bool ElidesDe(const FString& Name);

	/** HOLForumMessages::ElidePattern. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static FString ElidePattern(const FString& Pattern, int32 NameIndex, const FString& Name);

	/**
	 * HOLForumMessages::ShouldAnnounceHold for a ruin (phase Ruin) whose Hold is HoldKind (0 None, 1 Holding, 2 Contested, 3 Receding),
	 * held by HolderSlot for HoldSteps; Announced is the caller's bit set (updated).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static bool ShouldAnnounceHold(int32 HoldKind, int32 HolderSlot, int32 HoldSteps, UPARAM(ref) int32& Announced);

	/** HOLForumMessages::SplitNames(Message of Event as ViewerSlot reads it, ViewerSlot, Config). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static TArray<FHOLForumPieceValue> SplitNames(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, int32 ViewerSlot);

	/** The current culture's name ("fr", "en"...). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static FString GetCultureName();

	/** Sets the current culture; returns the previous name, to put back at the end of the test. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static FString SetCultureName(const FString& Culture);

	/** FHOLForumThread::MaxLines. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static int32 GetThreadMaxLines();

	/** FHOLForumAlert::MaxWaiting. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static int32 GetAlertMaxWaiting();
};

/** A FHOLForumThread for the script. */
UCLASS()
class HEROESOFLIBERTY_API UHOLForumThreadHolder : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static UHOLForumThreadHolder* Create();

	/** FHOLForumThread::Add of the message of Event (Scenario, ViewerSlot) at NowSeconds. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	void AddEvent(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, int32 ViewerSlot, double NowSeconds);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	FString Describe() const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	int32 GetLineCount() const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	double GetLineAddedSeconds(int32 Index) const;

	/** The text (Text.ToString()) of line Index. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	FString GetLineText(int32 Index) const;

	/** FHOLForumThread::GetOpacity of line Index (float32 on the C++ side, widened). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	double GetLineOpacity(int32 Index, double NowSeconds) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	bool Expire(double NowSeconds);

private:
	FHOLForumThread Thread;
};

/** A FHOLForumAlert for the script. */
UCLASS()
class HEROESOFLIBERTY_API UHOLForumAlertHolder : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	static UHOLForumAlertHolder* Create();

	/** FHOLForumAlert::Show of the message of Event (Scenario, ViewerSlot) for InSlot. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	bool ShowEvent(const FHOLForumEventValue& Event, const FHOLForumScenario& Scenario, int32 ViewerSlot, int32 InSlot, bool bMatchOverForViewer, double NowSeconds);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	bool Advance(double NowSeconds, bool bMatchOverForViewer);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	bool Expire(double NowSeconds);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	double GetOpacity(double NowSeconds) const;

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	FString Describe() const;

	/** FHOLForumAlert::Slot. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	int32 GetSlot() const;

	/** FHOLForumAlert::bSound. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	bool HasSound() const;

	/** FHOLForumAlert::Waiting.Num(). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ForumMessages")
	int32 GetWaitingCount() const;

private:
	FHOLForumAlert Alert;
};
