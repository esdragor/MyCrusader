// The wait and absence banners of the game HUD (R3.10, R7.5; H-1 of the HUD plan), in script: "En attente de Bob (1:52)" while the match waits for its
// named players, and "Bob : connexion perdue - 2:41" / "Bob : de retour" while a player is gone or just back. Top centre under the top bar, the absence
// banner 52 px under the wait one; both let the clicks through. The rules are pure (namespace HOLWaitBanner: when each is shown, which text, which journal
// line); UHOLWaitBannerView builds the two panels and reads the game state through HOLWaitBannerRead (C++ binding, UHOLWaitBannerReadLibrary) at each
// Refresh (4 per second). The journal lines are the C++ screen's, word for word (quick_check net and lobby_match read them):
//   "Wait banner: shown, <text>"  "Wait banner: hidden"  "Absence banner: shown, <line> | <line>"  "Absence banner: hidden"

struct FHOLWaitBannerView
{
	bool bVisible = false;
	FString Text = "";
	// What the journal line is logged on: the missing players (wait), or "<lines>;<name>+|-;..." (absence). A new value = a new line; the countdown's
	// seconds are not in it.
	FString Signature = "";
	// wait <players> <m:ss> | absence <n> | hidden: the stable line of the tests.
	FString Describe = "hidden";
}

// The texts (French source here, same namespace and keys as the C++ LOCTEXT of UHOLGameScreen: "HOLGameScreen,WaitingFor", so the translations stay).
// The absence lines come ready from the game state (FHOLDisconnectRules::FormatAbsentLine / FormatBackLine, C++ shared with the server's report).
namespace HOLWaitBannerText
{
	FText WaitingFor()
	{
		return NSLOCTEXT("HOLGameScreen", "WaitingFor", "En attente de {Players} ({Time})");
	}

	FString FormatWaiting(FString Players, FString Time)
	{
		return WaitingFor().ToString().Replace("{Players}", Players).Replace("{Time}", Time);
	}
}

namespace HOLWaitBanner
{
	// The change a refresh makes to a banner, as the C++ logged it.
	// Functions, not constants: a constant of a namespace is not visible from another file (the test).
	int ChangeNone() { return 0; }
	int ChangeShown() { return 1; }
	int ChangeHidden() { return 2; }

	// The missing players, "Bob, Carol" (FString::Join(Missing, ", ")).
	FString JoinNames(TArray<FString> Names, FString Separator)
	{
		FString Joined = "";
		for (int Index = 0; Index < Names.Num(); Index++)
		{
			Joined += (Index > 0 ? Separator : "") + Names[Index];
		}
		return Joined;
	}

	// The wait banner: shown while the game state waits for players AND names at least one missing. SecondsLeft is the whole seconds before the wait's end
	// by the server's clock (the binding rounds the remaining time up, never under 0): m:ss.
	FHOLWaitBannerView WaitView(bool bWaiting, TArray<FString> Missing, int SecondsLeft)
	{
		FHOLWaitBannerView View;
		if (!bWaiting || Missing.Num() == 0)
			return View;
		const FString Players = JoinNames(Missing, ", ");
		const FString Clock = HOLHeroBlock::Clock(SecondsLeft);
		View.bVisible = true;
		View.Signature = Players;
		View.Text = HOLWaitBannerText::FormatWaiting(Players, Clock);
		View.Describe = "wait " + Players + " " + Clock;
		return View;
	}

	// The absence banner: one line per notice that has something to say (the game state's lines, in the order of the departures), shown while there is
	// one. Names and bBack are the notices as the game state holds them, ALL of them (a back notice past its time has no line but stays in the signature,
	// as the C++ screen's did); bBack: the player is back (BackAt > 0).
	FHOLWaitBannerView AbsenceView(TArray<FString> Lines, TArray<FString> Names, TArray<bool> bBack)
	{
		FHOLWaitBannerView View;
		if (Lines.Num() == 0)
			return View;
		FString Signature = f"{Lines.Num()}";
		for (int Index = 0; Index < Names.Num(); Index++)
		{
			const bool bIsBack = Index < bBack.Num() && bBack[Index];
			Signature += ";" + Names[Index] + (bIsBack ? "+" : "-");
		}
		View.bVisible = true;
		View.Signature = Signature;
		View.Text = JoinNames(Lines, "\n");
		View.Describe = f"absence {Lines.Num()}";
		return View;
	}

	// What a refresh does to the journal and to the panel's visibility, from the signature shown so far ("" : not shown): hidden once when it goes
	// away, shown each time the signature changes (who is missing, who is gone or back), nothing on a plain countdown tick.
	int Change(FString ShownSignature, FHOLWaitBannerView View)
	{
		if (!View.bVisible)
			return ShownSignature.IsEmpty() ? ChangeNone() : ChangeHidden();
		return View.Signature != ShownSignature ? ChangeShown() : ChangeNone();
	}

	// The C++ screen's journal lines (UE_LOG LogHOLUI Display).
	FString WaitLogLine(int Kind, FHOLWaitBannerView View)
	{
		if (Kind == ChangeShown())
			return "Wait banner: shown, " + View.Text;
		if (Kind == ChangeHidden())
			return "Wait banner: hidden";
		return "";
	}

	FString AbsenceLogLine(int Kind, FHOLWaitBannerView View)
	{
		if (Kind == ChangeShown())
			return "Absence banner: shown, " + View.Text.Replace("\n", " | ");
		if (Kind == ChangeHidden())
			return "Absence banner: hidden";
		return "";
	}
}

// The two panels. Built once in the script screen's OnInitialized (Build), refreshed in its Refresh (Refresh). The widget names are the C++ screen's
// (WaitPanel, WaitText, AbsencePanel, AbsenceText).
class UHOLWaitBannerView : UObject
{
	// The C++ screen's places: 16 px margin, under the top bar and the queue's two rows (36 px each); the absence banner 52 px lower.
	const float Margin = 16.0;
	const float WaitTop = 88.0;
	const float AbsenceTop = 140.0;
	// The C++ brush: the HUD panel at 80 % (FSlateRoundedBoxBrush(HUDPanel.CopyWithNewOpacity(0.8f), 4)); here the panel's tint, MakePanel's brush at 80 %.
	const float PanelOpacity = 0.8;

	UBorder WaitPanel;
	UCommonTextBlock WaitText;
	UBorder AbsencePanel;
	UCommonTextBlock AbsenceText;

	// The signature each banner is shown for ("" : hidden) and the last journal line written, for the tests (a script test cannot read the journal).
	FString ShownWait = "";
	FString ShownAbsence = "";
	FString LastWaitLine = "";
	FString LastAbsenceLine = "";
	FHOLWaitBannerView WaitNow;
	FHOLWaitBannerView AbsenceNow;

	void Build(UHOLScriptScreen Screen, UOverlay Frame)
	{
		WaitPanel = Screen.MakePanel(n"WaitPanel");
		WaitPanel.SetBrushColor(FLinearColor(1.0, 1.0, 1.0, PanelOpacity));
		WaitPanel.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot WaitSlot = Frame.AddChildToOverlay(WaitPanel);
		WaitSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		WaitSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		WaitSlot.SetPadding(FMargin(Margin, WaitTop, Margin, Margin));
		WaitText = Screen.MakeText(n"WaitText", UHOLTextStyleHUDHeading);
		WaitPanel.SetContent(WaitText);

		AbsencePanel = Screen.MakePanel(n"AbsencePanel");
		AbsencePanel.SetBrushColor(FLinearColor(1.0, 1.0, 1.0, PanelOpacity));
		AbsencePanel.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot AbsenceSlot = Frame.AddChildToOverlay(AbsencePanel);
		AbsenceSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		AbsenceSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		AbsenceSlot.SetPadding(FMargin(Margin, AbsenceTop, Margin, Margin));
		AbsenceText = Screen.MakeText(n"AbsenceText", UHOLTextStyleHUDHeading);
		AbsenceText.SetJustification(ETextJustify::Center);
		AbsencePanel.SetContent(AbsenceText);
	}

	// One read of the game state (HOLWaitBannerRead), both banners: the text follows the countdown at each call, the visibility and the journal the
	// signature (HOLWaitBanner::Change).
	void Refresh()
	{
		TArray<FString> Missing;
		int SecondsLeft = 0;
		const bool bWaiting = HOLWaitBannerRead::GetWaitState(Missing, SecondsLeft);
		WaitNow = HOLWaitBanner::WaitView(bWaiting, Missing, SecondsLeft);
		const int WaitChange = HOLWaitBanner::Change(ShownWait, WaitNow);
		if (WaitNow.bVisible)
			WaitText.SetText(FText::FromString(WaitNow.Text));
		if (WaitChange != HOLWaitBanner::ChangeNone())
		{
			ShownWait = WaitNow.bVisible ? WaitNow.Signature : "";
			WaitPanel.SetVisibility(WaitNow.bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			LastWaitLine = HOLWaitBanner::WaitLogLine(WaitChange, WaitNow);
			Log(n"LogHOLUI", LastWaitLine);
		}

		TArray<FString> Lines;
		TArray<FString> Names;
		TArray<bool> bBack;
		HOLWaitBannerRead::GetAbsenceState(Lines, Names, bBack);
		AbsenceNow = HOLWaitBanner::AbsenceView(Lines, Names, bBack);
		const int AbsenceChange = HOLWaitBanner::Change(ShownAbsence, AbsenceNow);
		if (AbsenceNow.bVisible)
			AbsenceText.SetText(FText::FromString(AbsenceNow.Text));
		if (AbsenceChange != HOLWaitBanner::ChangeNone())
		{
			ShownAbsence = AbsenceNow.bVisible ? AbsenceNow.Signature : "";
			AbsencePanel.SetVisibility(AbsenceNow.bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
			LastAbsenceLine = HOLWaitBanner::AbsenceLogLine(AbsenceChange, AbsenceNow);
			Log(n"LogHOLUI", LastAbsenceLine);
		}
	}
}
