// What the duel's challenge window shows (DuelInProgress.md D8; GDD 10.1, D117): pure rules, no widget. The screen reads one
// view per viewer: the target sees the window that asks (portrait of the challenger, countdown bar, Accept / Refuse), the
// challenger sees a "challenge sent" indicator with the same countdown, everyone else sees nothing. The window never pauses
// the game and blocks nothing: that is the widget's matter, not a rule.
enum EHOLDuelWindowKind
{
	None,
	// A challenge received: the window with the answer buttons.
	Incoming,
	// A challenge sent: the indicator, no button (the challenger cannot cancel, D80).
	Sent
}

struct FHOLDuelWindowView
{
	EHOLDuelWindowKind Kind = EHOLDuelWindowKind::None;

	// The other hero's slot (the challenger for an Incoming window, the target for a Sent one); -1 for None.
	int OpponentSlot = -1;

	// The countdown: whole seconds left (rounded up, so that the last one reads 1 until the window ends), and the bar's fill in
	// thousandths of the whole window.
	int SecondsLeft = 0;
	int FillPermille = 0;

	// The answer buttons are live (Incoming only), and the last instants flash (D117: "impossible to miss").
	bool bCanAnswer = false;
	bool bUrgent = false;
}

namespace HOLDuel
{
	// The view of one duel for one viewer.
	FHOLDuelWindowView WindowView(const FHOLDuelSettings& S, const FHOLDuel& Duel, int ViewerSlot)
	{
		FHOLDuelWindowView V;
		if (Duel.Phase != EHOLDuelPhase::Pending)
			return V;
		if (ViewerSlot == Duel.Target)
		{
			V.Kind = EHOLDuelWindowKind::Incoming;
			V.OpponentSlot = Duel.Challenger;
			V.bCanAnswer = true;
		}
		else if (ViewerSlot == Duel.Challenger)
		{
			V.Kind = EHOLDuelWindowKind::Sent;
			V.OpponentSlot = Duel.Target;
		}
		else
		{
			return V;
		}
		V.SecondsLeft = Math::IntegerDivisionTrunc(Duel.WindowLeftMs + 999, 1000);
		V.FillPermille = S.ResponseMs > 0 ? Math::Clamp(Math::IntegerDivisionTrunc(Duel.WindowLeftMs * 1000, S.ResponseMs), 0, 1000) : 0;
		V.bUrgent = Duel.WindowLeftMs <= S.WindowUrgentMs;
		return V;
	}

	// The view for a viewer across the whole book: the first challenge waiting that concerns him.
	FHOLDuelWindowView BookWindowView(const FHOLDuelSettings& S, const FHOLDuelBook& Book, int ViewerSlot)
	{
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			FHOLDuelWindowView V = WindowView(S, Book.Duels[Index], ViewerSlot);
			if (V.Kind != EHOLDuelWindowKind::None)
				return V;
		}
		return FHOLDuelWindowView();
	}
}