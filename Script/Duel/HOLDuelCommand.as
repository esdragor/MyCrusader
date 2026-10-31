// The challenge command's rules (DuelInProgress.md D4; GDD 10.1, D16, D80, D403): pure, whole numbers, no world access. The match
// (UHOLDuelSubsystem) measures what only it can see (the heroes' places, who is alive and an enemy, whether they see each other,
// the main forums' places) and hands it over; the rules decide near enough, inside a forum's zone, and what each machine is shown
// of a slot's duel (the public part of the hero sheet, replicated). Every number is in HOLDuel::Preset.

// The facts the match measures about a challenge, one bit each (the order of the C++ side, UHOLDuelSubsystem::MeasureFacts).
namespace HOLDuelFact
{
	const int Enemies = 1;
	const int SeeEachOther = 2;
	const int Blocked = 4;
	const int ChallengerAlive = 8;
	const int TargetAlive = 16;
}

// What a slot's hero sheet carries of its duel (replicated to every machine): Code 0 nothing, 1 a challenge sent and waiting,
// 2 a challenge received and waiting, 3 a duel on.
struct FHOLDuelView
{
	int Code = 0;
	// The other hero's slot; -1 with no duel.
	int Opponent = -1;
	// Pending: the time left to answer; Active: the time left to the duel.
	int LeftMs = 0;
	// Active: the circle, cm.
	int CenterX = 0;
	int CenterY = 0;
	int RadiusCm = 0;
}

namespace HOLDuel
{
	// Whether the point (X, Y) is inside the zone of any listed main forum (ForumXY: X, Y, X, Y... in cm; D403: any player's,
	// ruined or not; the match lists the main forums only).
	bool InForumZone(const FHOLDuelSettings& S, int X, int Y, const TArray<int>& ForumXY)
	{
		const int64 Radius = int64(S.ForumZoneRadiusCm);
		for (int Index = 0; Index + 1 < ForumXY.Num(); Index += 2)
		{
			const int64 DX = int64(X) - int64(ForumXY[Index]);
			const int64 DY = int64(Y) - int64(ForumXY[Index + 1]);
			if (DX * DX + DY * DY <= Radius * Radius)
				return true;
		}
		return false;
	}

	// The place of a challenge from the measured facts (HOLDuelFact bits), the two heroes' places and the main forums.
	FHOLDuelPlace MeasurePlace(const FHOLDuelSettings& S, int Facts, int AX, int AY, int BX, int BY, const TArray<int>& ForumXY)
	{
		FHOLDuelPlace Place;
		Place.bEnemies = (Facts & HOLDuelFact::Enemies) != 0;
		Place.bSeeEachOther = (Facts & HOLDuelFact::SeeEachOther) != 0;
		Place.bBlockedByEffect = (Facts & HOLDuelFact::Blocked) != 0;
		Place.bNear = FitsInArena(S, AX, AY, BX, BY);
		Place.bInMainForumRadius = InForumZone(S, AX, AY, ForumXY) || InForumZone(S, BX, BY, ForumXY);
		return Place;
	}

	FHOLDuelHero HeroOfFacts(int Facts, bool bChallenger)
	{
		FHOLDuelHero Hero;
		Hero.bAlive = (Facts & (bChallenger ? HOLDuelFact::ChallengerAlive : HOLDuelFact::TargetAlive)) != 0;
		return Hero;
	}

	// Whether the conditions of the challenge (not the cooldown, not "in a duel") hold now (DU-Q2: read at the answer and while waiting).
	bool ConditionsHold(const FHOLDuelSettings& S, int Facts, int AX, int AY, int BX, int BY, const TArray<int>& ForumXY)
	{
		FHOLDuelPlace Place = MeasurePlace(S, Facts, AX, AY, BX, BY, ForumXY);
		return Place.bEnemies && Place.bNear && Place.bSeeEachOther && !Place.bInMainForumRadius && !Place.bBlockedByEffect
			&& HeroOfFacts(Facts, true).bAlive && HeroOfFacts(Facts, false).bAlive;
	}

	// The book's challenge: "" and the duel Pending, or the reason (CanChallenge's).
	FString BookStartChallenge(const FHOLDuelSettings& S, FHOLDuelBook& Book, int Challenger, int Target, int Facts, int AX, int AY, int BX, int BY, const TArray<int>& ForumXY)
	{
		return Start(S, Book, Challenger, Target, HeroOfFacts(Facts, true), HeroOfFacts(Facts, false), MeasurePlace(S, Facts, AX, AY, BX, BY, ForumXY));
	}

	// The slot's duel as the sheet shows it. A duel Over has left the book (Tick), so it shows nothing.
	FHOLDuelView BookViewOf(const FHOLDuelBook& Book, int Slot)
	{
		FHOLDuelView View;
		for (int Index = 0; Index < Book.Duels.Num(); Index++)
		{
			const FHOLDuel Duel = Book.Duels[Index];
			if (!InDuel(Duel, Slot))
				continue;
			View.Opponent = Slot == Duel.Challenger ? Duel.Target : Duel.Challenger;
			if (Duel.Phase == EHOLDuelPhase::Pending)
			{
				View.Code = Slot == Duel.Challenger ? 1 : 2;
				View.LeftMs = Duel.WindowLeftMs;
			}
			else
			{
				View.Code = 3;
				View.LeftMs = Duel.TimeLeftMs;
				if (Duel.bHasArena)
				{
					View.CenterX = Duel.Arena.CenterX;
					View.CenterY = Duel.Arena.CenterY;
					View.RadiusCm = Duel.Arena.RadiusCm;
				}
			}
			return View;
		}
		return View;
	}
}
