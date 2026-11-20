// The server's specialization board (SP2, SpecializationInProgress.md), the script side of UHOLSpecializationSubsystem: one
// FHOLSpecializationState per seat; the C++ part gives the seat's tier (AHOLGameState::GetTier), its Forum (complete: answer B of
// Q-nomad-forum Q2; standing: Q2 answer A, paused) and the world's time; the rules are HOLSpecializationRules.as (Start, Cancel, Advance).
// Server and solo only (AHOLPlayerController::ServerStartSpecialization calls it; a client never decides).
// Tested by HeroesOfLiberty.Specialization.ServerBoard. Two options per tier for now (the Templars' third commandery comes with their faction).
class UHOLSpecializationBoardScript : UHOLSpecializationSubsystem
{
	TArray<FHOLSpecializationState> States;

	// ELROND-1120e (morceau 7 du socle): the faction of each seat, set at the seat's initialisation (SetSeatFaction, the model of SetSeatNomad).
	// STUB of the first commit: the faction is ignored.
	TArray<int> SeatFactions;

	void SetSeatFaction(int Slot, int Faction)
	{
	}

	FHOLSpecializationState& StateOf(int Slot)
	{
		while (States.Num() <= Slot)
			States.Add(HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount));
		return States[Slot];
	}

	UFUNCTION(BlueprintOverride)
	int StartSpecialization(int Slot, int Tier, int Option, int PlayerTier, bool bForumComplete)
	{
		if (Slot < 0)
			return HOLSpecialization::BadTier;
		return HOLSpecialization::Start(StateOf(Slot), Tier, Option, PlayerTier, bForumComplete);
	}

	UFUNCTION(BlueprintOverride)
	int CancelSpecialization(int Slot)
	{
		if (Slot < 0)
			return HOLSpecialization::NothingToCancel;
		return HOLSpecialization::Cancel(StateOf(Slot));
	}

	UFUNCTION(BlueprintOverride)
	bool IsSpecializationOpen(int Slot, int Tier, int PlayerTier, bool bForumComplete)
	{
		return Slot >= 0 && HOLSpecialization::IsOpen(StateOf(Slot), Tier, PlayerTier, bForumComplete);
	}

	UFUNCTION(BlueprintOverride)
	int AdvanceSpecialization(int Slot, int ElapsedMs, bool bForumStanding)
	{
		if (Slot < 0)
			return 0;
		return HOLSpecialization::Advance(StateOf(Slot), ElapsedMs, bForumStanding);
	}

	// N2 step 1 (answer B of Q-nomad-forum Q2): one FHOLNomadState per seat (HOLNomadRules.as), standard until SetSeatNomad.
	TArray<FHOLNomadState> NomadStates;

	FHOLNomadState& NomadOf(int Slot)
	{
		while (NomadStates.Num() <= Slot)
			NomadStates.Add(HOLNomad::NewState(false));
		return NomadStates[Slot];
	}

	UFUNCTION(BlueprintOverride)
	void SetSeatNomad(int Slot, bool bNomad)
	{
		if (Slot >= 0)
			NomadOf(Slot) = HOLNomad::NewState(bNomad);
	}

	UFUNCTION(BlueprintOverride)
	void NoteSeatForumComplete(int Slot)
	{
		if (Slot >= 0)
			HOLNomad::NoteForumComplete(NomadOf(Slot));
	}

	UFUNCTION(BlueprintOverride)
	bool SeatHasCompleteForum(int Slot)
	{
		return Slot >= 0 && HOLNomad::HasCompleteForum(NomadOf(Slot));
	}

	// N2 (b), D354: no Peasant left before the first complete Forum is the defeat of a Nomad seat (HOLNomad::IsDefeated);
	// a standard seat is never before its first Forum, so never defeated here.
	UFUNCTION(BlueprintOverride)
	bool SeatIsNomadDefeated(int Slot, int PeasantCount)
	{
		return Slot >= 0 && HOLNomad::IsDefeated(NomadOf(Slot), PeasantCount);
	}

	// N2 step 2a (answer A of Q-nomad-forum Q1): a Nomad seat starts with the standard stock plus the secondary centre's price (D783),
	// HOLNomad::StartingAmount per resource (0 food, 1 wood, 2 stone, 3 gold). Only called with bNomad (StartingStock).
	UFUNCTION(BlueprintOverride)
	int NomadStartingAmount(int Resource, int StandardAmount)
	{
		HOLSecondaryCentre::FHOLCentreCost Price = HOLSecondaryCentre::Cost();
		int ForumPrice = Resource == 0 ? Price.Food : (Resource == 1 ? Price.Wood : (Resource == 2 ? Price.Stone : (Resource == 3 ? Price.Gold : 0)));
		return HOLNomad::StartingAmount(StandardAmount, ForumPrice, true);
	}

	// BUG nomad_start point 1 (2026-11-14): a Nomad seat's first forum costs the secondary centre's price (D783), per resource
	// (0 food, 1 wood, 2 stone, 3 gold); read by UHOLBuildingSettings::GetBuildCost (grid, placement, refund).
	UFUNCTION(BlueprintOverride)
	int NomadForumCost(int Resource)
	{
		HOLSecondaryCentre::FHOLCentreCost Price = HOLSecondaryCentre::Cost();
		return Resource == 0 ? Price.Food : (Resource == 1 ? Price.Wood : (Resource == 2 ? Price.Stone : (Resource == 3 ? Price.Gold : 0)));
	}

	// BUG nomad_start point 2 (2026-11-14, D207, D786, D702): the Nomad's starting groups, read by the game mode's start
	// (AHeroesOfLibertyGameModeBase::SpawnSlotStart): the size of group Group (0 past the last), far enough from another camp,
	// the least distance to a site's centre.
	UFUNCTION(BlueprintOverride)
	int NomadGroupSizeAt(int Peasants, int Group)
	{
		TArray<int> Sizes = HOLNomad::SplitIntoGroups(Peasants);
		return Group >= 0 && Group < Sizes.Num() ? Sizes[Group] : 0;
	}

	UFUNCTION(BlueprintOverride)
	bool NomadGroupFarEnough(int DistanceToClosestEnemyCm)
	{
		return HOLNomad::FarEnough(DistanceToClosestEnemyCm);
	}

	UFUNCTION(BlueprintOverride)
	int NomadSiteMinDistanceCm(int RadiusCm)
	{
		return HOLSiteOption::NomadMinDistanceCm(RadiusCm);
	}

	UFUNCTION(BlueprintOverride)
	FHOLSpecializationView ViewSpecialization(int Slot)
	{
		FHOLSpecializationView View;
		View.Chosen.Add(-1);
		if (Slot < 0)
			return View;
		FHOLSpecializationState& State = StateOf(Slot);
		for (int Tier = HOLSpecialization::FirstTier; Tier <= HOLSpecialization::LastTier; Tier++)
			View.Chosen.Add(HOLSpecialization::ChosenOption(State, Tier));
		View.PendingTier = State.PendingTier;
		View.PendingOption = State.PendingTier > 0 && State.PendingTier < State.RunningOption.Num() ? State.RunningOption[State.PendingTier] : -1;
		View.RemainingMs = HOLSpecialization::RemainingMs(State);
		return View;
	}
}
