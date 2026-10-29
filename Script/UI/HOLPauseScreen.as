// The pause screen as a view (P5, D513): what it shows, from what the machine knows of the pause (pure, no world), applied
// to the widgets by UHOLGameScreenScript. Over the match for everyone (players and spectators): who paused, the timer
// (before its end only he resumes, after it any player still in the match), the pauses left to the match; « Reprendre »
// greyed for whoever may not resume yet. Solo: no timer, no count. Describe is the stable line of the tests.
struct FHOLPauseScreenView
{
	bool bVisible = false;
	FString ByName;
	bool bFree = false;
	// Whole seconds left on the timer, 0 once over.
	int SecondsLeft = 0;
	int PausesLeft = 0;
	bool bCanResume = false;
	// paused by <name>, <S> s | timer over | free, <P> left, resume yes|no; or hidden.
	FString Describe = "hidden";
}

namespace HOLPauseScreen
{
	FHOLPauseScreenView Make(bool bPaused, bool bFree, FString ByName, int SecondsLeft, int PausesLeft, bool bCanResume)
	{
		FHOLPauseScreenView View;
		if (!bPaused)
			return View;
		View.bVisible = true;
		View.ByName = ByName;
		View.bFree = bFree;
		View.SecondsLeft = bFree ? 0 : Math::Max(0, SecondsLeft);
		View.PausesLeft = Math::Max(0, PausesLeft);
		View.bCanResume = bCanResume;
		const FString Timer = bFree ? "free" : (View.SecondsLeft > 0 ? f"{View.SecondsLeft} s" : "timer over");
		const FString Count = bFree ? "" : f", {View.PausesLeft} left";
		View.Describe = f"paused by {ByName}, {Timer}{Count}, resume " + (bCanResume ? "yes" : "no");
		return View;
	}
}

// The screen's texts by key (French source here, gathered from the .as files by GatherText; English and Japanese in
// Tools/Localization/fill_po.py, keys "HOLPauseScreen,<Name>"). Literals only in these calls.
namespace HOLPauseScreenText
{
	const FString Title = "Pause.Title";
	const FString By = "Pause.By";
	const FString TimerRunning = "Pause.TimerRunning";
	const FString TimerOver = "Pause.TimerOver";
	const FString Left = "Pause.Left";
	const FString Resume = "Pause.Resume";

	FText PatternText(FString Key)
	{
		if (Key == Title) return NSLOCTEXT("HOLPauseScreen", "Title", "Partie en pause");
		if (Key == By) return NSLOCTEXT("HOLPauseScreen", "By", "Mise en pause par {0}");
		if (Key == TimerRunning) return NSLOCTEXT("HOLPauseScreen", "TimerRunning", "Tout le monde pourra reprendre dans {0}");
		if (Key == TimerOver) return NSLOCTEXT("HOLPauseScreen", "TimerOver", "Tout le monde peut reprendre");
		if (Key == Left) return NSLOCTEXT("HOLPauseScreen", "Left", "Pauses restantes : {0}");
		if (Key == Resume) return NSLOCTEXT("HOLPauseScreen", "Resume", "Reprendre");
		return FText();
	}

	FString Format(FString Key, FString First = "")
	{
		return PatternText(Key).ToString().Replace("{0}", First);
	}

	// "" in solo (only one player, whose name may be the engine's default).
	FString ByText(FHOLPauseScreenView View) { return View.bVisible && !View.bFree ? Format(By, View.ByName) : ""; }
	// "" in solo (no timer).
	FString TimerText(FHOLPauseScreenView View)
	{
		if (!View.bVisible || View.bFree)
			return "";
		return View.SecondsLeft > 0 ? Format(TimerRunning, HOLHeroBlock::Clock(View.SecondsLeft)) : Format(TimerOver);
	}
	// "" in solo (no limit).
	FString LeftText(FHOLPauseScreenView View) { return View.bVisible && !View.bFree ? Format(Left, f"{View.PausesLeft}") : ""; }
}
