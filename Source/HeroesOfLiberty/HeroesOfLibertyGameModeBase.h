// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Match/HOLMatchConfig.h"
#include "HeroesOfLibertyGameModeBase.generated.h"

class AHOLBuilding;

/**
 * Game mode of a match: reads the match configuration when the map starts (D243) and gives each joining human
 * the next free human slot (a slot frees itself when its player state leaves PlayerArray). Without a configuration
 * it runs with the default one (one human in slot 0), so the stress test and BattleMap opened directly keep working.
 * On a match map (one with start slots, D256) the players get the match controller and its RTS camera, and no pawn
 * (D255), and every occupied slot gets its main centre when the match starts (D259); a map without start slots
 * (StressTest, BattleMap) keeps the engine's controller and default pawn, and no building.
 */
UCLASS()
class HEROESOFLIBERTY_API AHeroesOfLibertyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHeroesOfLibertyGameModeBase();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	/** Refuses a client of another version than the server (R3.3, HOLVersion::IsAccepted), or without the server's access key (L11, HOLAccessKey). */
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	/** Reads the player's ?Name= (URL-decoded): his PlayerState name and, in PostLogin, his slot (R3.2). */
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options,
		const FString& Portal) override;
	/** The slot of the player's name (FHOLMatchConfig::FindSlotForName); none: a spectator (M6-Q8). */
	virtual void PostLogin(APlayerController* NewPlayer) override;
	/**
	 * The last player gone after the start: "empty" to the lobby manager (R3.7). R7: a named player gone during the match
	 * keeps his slot for hol.Match.DisconnectWindowSeconds (AHOLGameState::KeepAbsentSlot).
	 */
	virtual void Logout(AController* Exiting) override;
	/** The server listens by now: "listening" to the lobby manager, with the port listened on (R3.6). */
	virtual void StartPlay() override;
	/**
	 * D361: a networked match never pauses (FHOLCheatRules::IsPauseAllowed): the solo game alone keeps the engine's pause.
	 * The engine asks it for a client's console "pause" (ServerPause, PC its controller: refused and logged) and when it
	 * clears the pausers (no PC).
	 */
	virtual bool AllowPausing(APlayerController* PC = nullptr) override;

	const FHOLMatchConfig& GetMatchConfig() const { return MatchConfig; }

private:
	/** A main centre on the start of each occupied slot (human or AI), at the slot's colour. */
	void SpawnMainCentres();

public:
	/**
	 * V5.1 (Regicide): a King of the slot born by its main forum (FHOLStartRules::GetKingLocation), outside the population.
	 * At the start for each occupied slot with the option, or by hol.Victory.SpawnKing. False: no unit system.
	 */
	static bool SpawnKing(UWorld& World, int32 Slot, const AHOLBuilding& Forum);

	/**
	 * IA1.1: the slot's Num starting Peasants, born in front of its main forum (FHOLStartRules::GetStartingPeasantLocations),
	 * at the start of the match. False: no unit system.
	 */
	static bool SpawnStartingPeasants(UWorld& World, int32 Slot, const AHOLBuilding& Forum, int32 Num);

	/**
	 * J8a.2 (J8a-Q2): the slot's hero, born by its main forum (FHOLStartRules::GetHeroLocation) at the start of the match,
	 * outside the population (his row's Population 0). False: no unit system.
	 */
	static bool SpawnStartingHero(UWorld& World, int32 Slot, const AHOLBuilding& Forum);

private:

	/**
	 * The buildings of a slot get its player's controller as network owner, so that their production queue reaches
	 * that player only (D282). Called when the player gets its slot and when the buildings appear, whichever is last.
	 */
	void AssignBuildingOwners(int32 Slot);

	/** "started" to the lobby manager once every human slot of the configuration is held (R3.7), for a configuration without names. */
	void SendStartedIfAllHere();
	/**
	 * R3.9: a configuration with names waits for them (hol.Match.StartWaitSeconds at most), the simulation and the orders
	 * frozen (R3.10); at the start, "started" to the lobby manager. Called at the start of play, at each login and each second.
	 */
	void UpdateStartWait();

	/** R7.3: each second while a slot is kept for its player gone, the slots whose window ran out are defeated. */
	void UpdateAbsences();
	FTimerHandle AbsenceTimer;

	/**
	 * "empty" to the lobby manager once no player is left, checked on the next frame: players leaving on the same frame
	 * have all sent their "left" by then (a controller already being destroyed made "empty" come first, 2026-10-27).
	 */
	void SendEmptyIfNobodyLeft();
	bool bEmptyCheckQueued = false;

	FTimerHandle StartWaitTimer;
	/** Game time of the start of the wait. */
	double StartWaitSince = 0.0;
	/** The missing names last logged. */
	TArray<FString> LoggedMissing;
	/** "finished" to the lobby manager, with the winning slots (R3.7). */
	void OnMatchOver();

	FHOLMatchConfig MatchConfig;

	/** The ?Name= of each player between InitNewPlayer and PostLogin (absent: no entry). */
	TMap<TWeakObjectPtr<APlayerController>, FString> LoginNames;
};
