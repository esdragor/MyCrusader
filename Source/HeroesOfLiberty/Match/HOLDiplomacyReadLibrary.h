// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLDiplomacyReadLibrary.generated.h"

/**
 * What the Diplomacy panel (D7 of DiplomacyInProgress.md, GDD D387) reads of the game, for Angelscript (HOLDiplomacyRead:: in script): the
 * stances of the book as this machine has them (the replicated state of AHOLGameState), who is in the match, their names, factions, colours and
 * lives as the players' table shows them, the local player's Market and the tribute's tax. Read only: the orders go through AHOLPlayerController
 * (RequestStance, RequestShareVision, RequestPanelTribute).
 */
/**
 * A book of stances (HOLDiplomacy::FBook) as the script sees it (socle step 1 of the migration, D830, D836 (5)): the same fields, so that the
 * interface reads the replicated book (GetBook) and asks the C++ rule (IsAlly, FiresAtOnce...) instead of its own copy,
 * Script/Diplomacy/HOLDiplomacyRules.as (whose FHOLDiplomacyBook goes with it). Stances[From * Slots + To], Vision[From * Slots + To].
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLDiplomacyStances
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 Slots = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bLocked = false;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	TArray<int32> Stances;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	TArray<bool> bAI;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	TArray<bool> Vision;
};

/**
 * What HOLDiplomacy::SetStance did (HOLDiplomacy::FStanceChange), for the script (socle step 1, D836 (5)): the same fields. Named apart from the
 * script's own FHOLStanceChange (Script/Diplomacy/HOLDiplomacyRules.as), which goes with that file at the redirection lot. Result is a
 * HOLDiplomacy::EResult number (GetResultOk...).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLDiplomacyStanceChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 Result = 0;

	/** A stance really changed (the line told to all). */
	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bAnnounce = false;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 From = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 To = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 Stance = 0;

	/** The relation of the pair before and after. */
	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 Before = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 After = 0;

	/** The pair was allied and no longer is / became allied. */
	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bBroke = false;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bFormed = false;
};

/** What HOLDiplomacy::SetVision did (HOLDiplomacy::FVisionChange), for the script; named apart from the script's FHOLVisionChange for the same reason. */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLDiplomacyVisionChange
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 Result = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bAnnounce = false;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 From = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	int32 To = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bShare = false;
};

/**
 * The end by allied victory (HOLDiplomacy::FAlliedVictory) for the script (S1e part 2, D836 (5)): the winners, together, when it ends. Named apart
 * from the script's own FHOLAlliedVictory (Script/Diplomacy/HOLDiplomacyRules.as), which goes with that file.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLDiplomacyAlliedVictory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	bool bEnded = false;

	UPROPERTY(BlueprintReadWrite, Category = "Diplomacy")
	TArray<int32> Winners;
};

UCLASS()
class HEROESOFLIBERTY_API UHOLDiplomacyReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The match is in the Diplomacy mode (the stances can be set); false in the standard modes (locked: the teams). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool IsDiplomacyMode(const UObject* WorldContextObject);

	/** Slot holds a player (human or AI) of this match. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool IsSlotInMatch(const UObject* WorldContextObject, int32 Slot);

	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool IsSlotAI(const UObject* WorldContextObject, int32 Slot);

	/** The stance From has set toward To (0 Enemy, 1 Neutral, 2 Ally; HOLDiplomacy::StanceOf). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static int32 GetStance(const UObject* WorldContextObject, int32 From, int32 To);

	/** The relation of a pair (the most hostile of the two stances, D375). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static int32 GetRelation(const UObject* WorldContextObject, int32 A, int32 B);

	/** From lets To see what it sees (D371). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool GetVision(const UObject* WorldContextObject, int32 From, int32 To);

	/** The slot's colour in the palette (HOLPlayerColors), grey outside the match. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FLinearColor GetSlotColor(const UObject* WorldContextObject, int32 Slot);

	/** The slot's name as the players' table shows it from Viewer's seat: « Vous » on its own row, « Joueur N » otherwise (a named human: its name). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotNameText(const UObject* WorldContextObject, int32 Viewer, int32 Slot);

	/** The slot's lives as the players' table shows them (« 2 vies », « 1 vie »; empty once defeated, « Vaincu » is the state). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotLivesText(const UObject* WorldContextObject, int32 Viewer, int32 Slot);

	/** The slot's state as the players' table shows it (« Ruine », « Vaincu »...), empty standing. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotStateText(const UObject* WorldContextObject, int32 Viewer, int32 Slot);

	/** The slot's faction name in the current language (the new game screen's own table, § 13). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FString GetSlotFactionText(const UObject* WorldContextObject, int32 Slot);

	/** The slot owns a finished Market (D372: the tribute needs one). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static bool HasFinishedMarket(const UObject* WorldContextObject, int32 Slot);

	/** The tribute's tax in percent (HOLTribute::DefaultTaxPercent, D372). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetTributeTaxPercent();

	/** The panel's presentation (hol.UI.DiplomacyLayout 0|1|2): 0 three buttons, 1 cycle on the colour chip, 2 drop-down menu. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetPanelLayout();

	// Socle step 1 (D830, D836 (5)): the C++ rule HOLDiplomacy on a book, wrappers only. The results of SetStance / SetVision are the numbers
	// of HOLDiplomacy::EResult (0 Ok, 1 NoSuchSlot, 2 Self, 3 BadStance, 4 SenderIsAI, 5 Locked, 6 NotAlly).

	/** The book of this machine (AHOLGameState::GetDiplomacyBook, replicated); an empty book without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy", meta = (WorldContext = "WorldContextObject"))
	static FHOLDiplomacyStances GetBook(const UObject* WorldContextObject);

	/** HOLDiplomacy::NewBook: a Diplomacy match's book, every stance at the default. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static FHOLDiplomacyStances NewBook(int32 Slots, const TArray<bool>& IsAI);

	/** HOLDiplomacy::NewTeamBook: the locked book of a match with fixed teams. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static FHOLDiplomacyStances NewTeamBook(const TArray<int32>& TeamBySlot, const TArray<bool>& IsAI);

	/** HOLDiplomacy::SetStance on Book (the result's number, see above). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Diplomacy")
	static int32 SetStance(UPARAM(ref) FHOLDiplomacyStances& Book, int32 From, int32 To, int32 Stance);

	/** HOLDiplomacy::SetVision on Book (the result's number, see above). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Diplomacy")
	static int32 SetVision(UPARAM(ref) FHOLDiplomacyStances& Book, int32 From, int32 To, bool bShare);

	/** HOLDiplomacy::StanceOf. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 StanceOf(const FHOLDiplomacyStances& Book, int32 From, int32 To);

	/** HOLDiplomacy::Relation: the most hostile of the two stances (D375). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 Relation(const FHOLDiplomacyStances& Book, int32 A, int32 B);

	/** HOLDiplomacy::IsAlly. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool IsAlly(const FHOLDiplomacyStances& Book, int32 A, int32 B);

	/** HOLDiplomacy::IsAdversary: Neutral included (D401). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool IsAdversary(const FHOLDiplomacyStances& Book, int32 A, int32 B);

	/** HOLDiplomacy::FiresAtOnce: only at war (D369). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool FiresAtOnce(const FHOLDiplomacyStances& Book, int32 A, int32 B);

	/** HOLDiplomacy::SharesVision: To sees what From sees (D371). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool SharesVision(const FHOLDiplomacyStances& Book, int32 From, int32 To);

	/** HOLDiplomacy::CanLeaveRampart (D374). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool CanLeaveRampart(const FHOLDiplomacyStances& Book, int32 UnitOwner, int32 RampartOwner);

	/** HOLDiplomacy::InMatch: Slot is a seat of the book (0 <= Slot < Slots). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool InMatch(const FHOLDiplomacyStances& Book, int32 Slot);

	/** HOLDiplomacy::CanGarrison: only in one's own building (D374). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool CanGarrison(int32 UnitOwner, int32 BuildingOwner);

	/** HOLDiplomacy::DoorsOpenFor: the doors and ramparts of Owner are open to Other (its own and an ally's), shut to everybody else (D371, D374). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static bool DoorsOpenFor(const FHOLDiplomacyStances& Book, int32 Owner, int32 Other);

	/** HOLDiplomacy::AlliedVictory: the survivors (not Defeated; a slot beyond the list is alive) all allied to each other and at least one beaten: they win together (D373). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static FHOLDiplomacyAlliedVictory AlliedVictory(const FHOLDiplomacyStances& Book, const TArray<bool>& Defeated);

	// Constants and codes of HOLDiplomacy (D836 (5)): the script reads them here, its own copy goes at the redirection lot.

	/** HOLDiplomacy::Enemy (0): the lower the stance, the more hostile. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetStanceEnemy();

	/** HOLDiplomacy::Neutral (1). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetStanceNeutral();

	/** HOLDiplomacy::Ally (2). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetStanceAlly();

	/** HOLDiplomacy::DefaultStance: the stance at launch of a Diplomacy match. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetStanceDefault();

	/** HOLDiplomacy::EResult::Ok (0). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultOk();

	/** HOLDiplomacy::EResult::NoSuchSlot (1). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultNoSuchSlot();

	/** HOLDiplomacy::EResult::Self (2). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultSelf();

	/** HOLDiplomacy::EResult::BadStance (3). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultBadStance();

	/** HOLDiplomacy::EResult::SenderIsAI (4). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultSenderIsAI();

	/** HOLDiplomacy::EResult::Locked (5). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultLocked();

	/** HOLDiplomacy::EResult::NotAlly (6): vision only. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Diplomacy")
	static int32 GetResultNotAlly();

	/** HOLDiplomacy::SetStance on Book, with its whole fiche (Result, bAnnounce, Before, After, bBroke, bFormed...); SetStance keeps returning the number alone. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Diplomacy")
	static FHOLDiplomacyStanceChange ChangeStance(UPARAM(ref) FHOLDiplomacyStances& Book, int32 From, int32 To, int32 Stance);

	/** HOLDiplomacy::SetVision on Book, with its whole fiche (Result, bAnnounce, bShare...). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Diplomacy")
	static FHOLDiplomacyVisionChange ChangeVision(UPARAM(ref) FHOLDiplomacyStances& Book, int32 From, int32 To, bool bShare);
};
