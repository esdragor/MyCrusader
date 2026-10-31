// The minimap's pure rules (InterfaceInProgress.md, the prototype's minimap in the manner of AoE4, decision of
// 2026-10-30): the conversion between the ground (cm) and the picture (px), what a click does, and the alerts that blink.
// No world here: UHOLGameScreenScript reads the game, calls these, and hands the result to UHOLMinimapView to paint.
// Screen up is world +X and screen right is world +Y (the RTS camera has no rotation, D254), the picture's origin at its
// upper left corner. The picture is a square of Size px; the map's area may be a rectangle (each axis keeps its ratio).
struct FHOLMinimapFrame
{
	float MinX = 0.0;
	float MinY = 0.0;
	float MaxX = 0.0;
	float MaxY = 0.0;
	// The picture's side, px.
	float Size = 0.0;
}

// One of the player's own units or buildings as the last reading saw it: where, and its health points (alerts compare two
// readings).
struct FHOLMinimapSeen
{
	int Id = 0;
	float X = 0.0;
	float Y = 0.0;
	int Health = 0;
}

// A place where something of the player's was hit, blinking for AlertSeconds.
struct FHOLMinimapAlert
{
	float X = 0.0;
	float Y = 0.0;
	float Age = 0.0;
}

namespace HOLMinimap
{
	// What a click on the picture does.
	const int ClickNothing = 0;
	const int ClickCamera = 1;
	const int ClickMove = 2;

	// An alert lives this long, then goes; lit for BlinkSeconds, dark for BlinkSeconds, and so on.
	const float AlertSeconds = 6.0;
	const float BlinkSeconds = 0.5;
	// Two wounded within this distance (cm) share one alert (it is renewed).
	const float MergeRadius = 2500.0;

	FHOLMinimapFrame MakeFrame(float MinX, float MinY, float MaxX, float MaxY, float Size)
	{
		FHOLMinimapFrame Frame;
		Frame.MinX = MinX;
		Frame.MinY = MinY;
		Frame.MaxX = MaxX;
		Frame.MaxY = MaxY;
		Frame.Size = Size;
		return Frame;
	}

	// A ground point (X, Y in cm) to its place in the picture (px, not clamped).
	FVector2D ToPicture(FHOLMinimapFrame Frame, FVector2D World)
	{
		float Width = Frame.MaxY - Frame.MinY;
		float Height = Frame.MaxX - Frame.MinX;
		if (Width <= 0.0 || Height <= 0.0)
			return FVector2D(0.0, 0.0);
		float Px = (World.Y - Frame.MinY) / Width * Frame.Size;
		float Py = (Frame.MaxX - World.X) / Height * Frame.Size;
		return FVector2D(Px, Py);
	}

	TArray<FVector2D> ToPictureAll(FHOLMinimapFrame Frame, TArray<FVector2D> World)
	{
		TArray<FVector2D> Out;
		for (int I = 0; I < World.Num(); I++)
			Out.Add(ToPicture(Frame, World[I]));
		return Out;
	}

	// A point of the picture to the ground, clamped to the map's area (a click on the edge, or a drag past it).
	FVector2D ToWorld(FHOLMinimapFrame Frame, FVector2D Picture)
	{
		if (Frame.Size <= 0.0)
			return FVector2D(0.0, 0.0);
		float U = Math::Clamp(Picture.X / Frame.Size, 0.0, 1.0);
		float V = Math::Clamp(Picture.Y / Frame.Size, 0.0, 1.0);
		float Y = Frame.MinY + U * (Frame.MaxY - Frame.MinY);
		float X = Frame.MaxX - V * (Frame.MaxX - Frame.MinX);
		return FVector2D(X, Y);
	}

	bool Contains(FHOLMinimapFrame Frame, FVector2D Picture)
	{
		return Picture.X >= 0.0 && Picture.Y >= 0.0 && Picture.X <= Frame.Size && Picture.Y <= Frame.Size;
	}

	// Left: the camera goes there. Right: the selected units go there, only for someone who has some to order.
	int ClickAction(bool bRight, bool bHasOrderable)
	{
		if (!bRight)
			return ClickCamera;
		return bHasOrderable ? ClickMove : ClickNothing;
	}

	// An alert at (X, Y): renews the one within MergeRadius, or adds a new one.
	void AddAlert(TArray<FHOLMinimapAlert>& Alerts, float X, float Y)
	{
		for (int I = 0; I < Alerts.Num(); I++)
		{
			float Dx = Alerts[I].X - X;
			float Dy = Alerts[I].Y - Y;
			if (Dx * Dx + Dy * Dy <= MergeRadius * MergeRadius)
			{
				Alerts[I].X = X;
				Alerts[I].Y = Y;
				Alerts[I].Age = 0.0;
				return;
			}
		}
		FHOLMinimapAlert Alert;
		Alert.X = X;
		Alert.Y = Y;
		Alerts.Add(Alert);
	}

	// The alerts the damage between two readings makes: every thing in both whose health fell. One that is new, healed or
	// untouched makes none.
	void Detect(TArray<FHOLMinimapSeen> Before, TArray<FHOLMinimapSeen> After, TArray<FHOLMinimapAlert>& Alerts)
	{
		for (int I = 0; I < After.Num(); I++)
		{
			for (int J = 0; J < Before.Num(); J++)
			{
				if (Before[J].Id == After[I].Id)
				{
					if (After[I].Health < Before[J].Health)
						AddAlert(Alerts, After[I].X, After[I].Y);
					break;
				}
			}
		}
	}

	// Time passes: the alerts age, those past AlertSeconds go.
	void Age(TArray<FHOLMinimapAlert>& Alerts, float Seconds)
	{
		for (int I = Alerts.Num() - 1; I >= 0; I--)
		{
			Alerts[I].Age += Seconds;
			if (Alerts[I].Age >= AlertSeconds)
				Alerts.RemoveAt(I);
		}
	}

	// Lit in the first half-period, dark in the next, lit again...
	bool IsLit(FHOLMinimapAlert Alert)
	{
		int Phase = Math::FloorToInt(Alert.Age / BlinkSeconds);
		return (Phase % 2) == 0;
	}
}
