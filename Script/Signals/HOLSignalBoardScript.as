// The server's board of map signals (SG2, SignalsInProgress.md), the script side of UHOLSignalSubsystem: the C++ part gives the stances
// (FHOLSignalBookInput, read from the replicated game state) and the match time; the rules are HOLSignalRules.as (HOLSignal::Post, Sees).
// Server and solo only (AHOLPlayerController::ServerPostSignal calls it; a client never decides). Tested by HeroesOfLiberty.Signals.ServerBoard.
class UHOLSignalBoardScript : UHOLSignalSubsystem
{
	FHOLSignalBoard Board;

	FHOLDiplomacyBook ToBook(const FHOLSignalBookInput& Input)
	{
		FHOLDiplomacyBook Book = HOLDiplomacy::NewBook(Input.Slots, Input.IsAI);
		Book.bLocked = Input.bLocked;
		for (int Index = 0; Index < Book.Stances.Num() && Index < Input.Stances.Num(); Index++)
			Book.Stances[Index] = Input.Stances[Index];
		return Book;
	}

	UFUNCTION(BlueprintOverride)
	int PostSignal(const FHOLSignalBookInput& Input, int Sender, bool bDefeated, int Kind, FVector Location, int NowMs)
	{
		return HOLSignal::Post(Board, ToBook(Input), Sender, bDefeated, Kind, Location, NowMs);
	}

	UFUNCTION(BlueprintOverride)
	bool SeesSignal(const FHOLSignalBookInput& Input, int Sender, int Viewer, bool bViewerSpectator)
	{
		return HOLSignal::Sees(ToBook(Input), Sender, Viewer, bViewerSpectator);
	}

	UFUNCTION(BlueprintOverride)
	int NumSignals()
	{
		return Board.Signals.Num();
	}
}
