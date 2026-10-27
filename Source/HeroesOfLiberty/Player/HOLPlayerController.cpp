// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLPlayerController.h"
#include "Player/HOLAttackCursor.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Containers/Ticker.h"
#include "Dev/HOLTestHooks.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HAL/IConsoleManager.h"
#include "HOLCollision.h"
#include "HeroesOfLibertyLog.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Map/HOLMapBounds.h"
#include "Net/HOLCheatRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchStatsSubsystem.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLGameMenuRules.h"
#include "Player/HOLRTSCamera.h"
#include "Player/HOLSelectionMarkers.h"
#include "Player/HOLUnitPicking.h"
#include "Match/HOLVictoryRules.h"
#include "Resources/HOLSourceField.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Settings/HOLGameUserSettings.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
#include "Simulation/HOLUnitView.h"

namespace UE::HeroesOfLiberty::Player
{
	// The match HUD (D281) lives in the UI module, which a dedicated server never loads: found by its path on the
	// player's machine, whatever HUD class the server sends.
	const TCHAR* MatchHUDClassPath = TEXT("/Script/HeroesOfLibertyUI.HOLGameHUD");

	// Made by Tools/Editor/make_game_input.py.
	const TCHAR* MappingContextPath = TEXT("/Game/Input/IMC_Game.IMC_Game");
	const TCHAR* ActionFolder = TEXT("/Game/Input/");

	/** Units of one order at most (D03): the rule of every slot's orders (IA0.2), kept here for the selection and the groups. */
	constexpr int32 MaxOrderedUnits = HOLPlayerOrders::MaxOrderedUnits;

	/** Height (cm) of a unit's body drawn at scale 1 (the mannequin), and its drawn width over its radius: what a click aims at (D288). */
	constexpr double PickBodyHeight = 180.0;
	constexpr double PickWidthFactor = 1.3;
	/** Margin of a right click's target around an enemy's drawn body (J7c, retour d'essai 8, loop 3; HOLUnitPicking::PickNear). */
	float TargetMargin = static_cast<float>(HOLUnitPicking::TargetMarginAt1080);
	FAutoConsoleVariableRef CVarTargetMargin(TEXT("hol.Select.TargetMargin"), TargetMargin,
		TEXT("Pixels (on a viewport 1080 high, scaled with its height) around another player's unit within which a right click still attacks it, the nearest one; a click on a body always wins; 0: on the body only"));
	/** Seconds between two checks of the selected units still known here. */
	constexpr float SelectionPurgeInterval = 0.25f;

	/** Width of the screen border that scrolls the camera, as a fraction of the viewport. */
	constexpr double EdgeMargin = 0.01;
	/** Length of the selection trace from the camera (cm): well past the farthest zoom (D254). */
	constexpr double SelectionTraceLength = 1000000.0;

	/** Two « +10 » this close (cm) and this young (s) at once: the newer one a line higher (Peasants unloading side by side). */
	constexpr double DepositPopupStackDistance = 250.0;
	constexpr float DepositPopupStackSeconds = 0.5f;
	/** Lines a stack of « +10 » goes up to before starting again at the bottom. */
	constexpr int32 MaxDepositPopupStack = 4;

#if !UE_BUILD_SHIPPING
	int32 LogDepositPopups = 0;
	FAutoConsoleVariableRef CVarLogDepositPopups(TEXT("hol.Gather.LogPopups"), LogDepositPopups,
		TEXT("1: every \"+10\" shown on this machine at an unload of its player is logged, \"Deposit popup: +10 Wood at (x, y)\" (J5c.3, scripted checks)"));
#endif

	TAutoConsoleVariable<FString> CVarTestCursor(
		TEXT("hol.Camera.TestCursor"),
		TEXT(""),
		TEXT("Cursor position seen by the RTS camera's edge scrolling, \"X Y\" in 0..1 of the viewport (empty or off: the real cursor). For scripted checks (D257), never the desktop's cursor."));

	const UInputAction* LoadAction(const TCHAR* Name)
	{
		return LoadObject<UInputAction>(nullptr, *FString::Printf(TEXT("%s%s.%s"), ActionFolder, Name, Name));
	}

	EHOLCameraSpeed GetSpeedSetting()
	{
		const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
		return Settings ? Settings->GetCameraSpeed() : EHOLCameraSpeed::Normal;
	}

	/** Logs where the local players' cameras look, in every game world. */
	void ReportCameras()
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (!World || !World->IsGameWorld())
			{
				continue;
			}
			for (TActorIterator<AHOLPlayerController> It(World); It; ++It)
			{
				if (const AHOLRTSCamera* Camera = It->IsLocalController() ? It->GetRTSCamera() : nullptr)
				{
					const FVector Focus = Camera->GetFocus();
					UE_LOG(LogHOLMatch, Display, TEXT("Camera: focus (%.0f, %.0f, %.0f), height %.0f"), Focus.X, Focus.Y, Focus.Z,
						Camera->GetHeight());
				}
			}
		}
	}

	/** The local players' controllers in every game world. */
	TArray<AHOLPlayerController*> GetLocalControllers()
	{
		TArray<AHOLPlayerController*> Controllers;
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (World && World->IsGameWorld())
			{
				for (TActorIterator<AHOLPlayerController> It(World); It; ++It)
				{
					if (It->IsLocalController())
					{
						Controllers.Add(*It);
					}
				}
			}
		}
		return Controllers;
	}

	/** Scripted checks (D264, D289): a left click at a point of the viewport, without the desktop's mouse. */
	FAutoConsoleCommand ClickCommand(
		TEXT("hol.Select.Click"),
		TEXT("hol.Select.Click <X> <Y> [add] [double]: selects as a left click at that point of the viewport (0 to 1 each way) would; 'add' as with Shift; 'double' as a double click (A.1)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Click <X> <Y> [add]"));
				return;
			}
			const bool bAdd = Args.Contains(TEXT("add"));
			const bool bDouble = Args.Contains(TEXT("double"));
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const FVector2D Fraction(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]));
				bDouble ? Controller->SelectSameTypeAt(Fraction, bAdd) : Controller->SelectAt(Fraction, bAdd);
			}
		}));

	/** The same on one of the player's own units, wherever it is drawn: a script does not know where units stand on the screen. */
	FAutoConsoleCommand ClickUnitCommand(
		TEXT("hol.Select.ClickUnit"),
		TEXT("hol.Select.ClickUnit <N> [add] [double] [order] [enemy|ally|slot=<S>]: hol.Select.Click (order: hol.Select.Order, a right click) at the middle of the drawn body of the player's N-th own unit known here (by identifier; negative from the last); 'enemy': of the other players' units instead, 'ally': of the players' of its team (D.3), 'slot=<S>': of slot S's (D.2)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Wanted = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			const bool bAdd = Args.Contains(TEXT("add"));
			const bool bDouble = Args.Contains(TEXT("double"));
			const bool bOrder = Args.Contains(TEXT("order"));
			const bool bEnemy = Args.Contains(TEXT("enemy"));
			const bool bAlly = Args.Contains(TEXT("ally"));
			int32 Slot = INDEX_NONE;
			for (const FString& Arg : Args)
			{
				if (Arg.StartsWith(TEXT("slot=")))
				{
					Slot = FCString::Atoi(*Arg.RightChop(5));
				}
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				TArray<FHOLUnitView> Views;
				HOLUnitView::Gather(*Controller->GetWorld(), Views);
				const AHOLGameState* GameState = Controller->GetWorld()->GetGameState<AHOLGameState>();
				Views.RemoveAll([PlayerState, GameState, bEnemy, bAlly, Slot](const FHOLUnitView& View)
				{
					if (Slot != INDEX_NONE)
					{
						return View.OwnerPlayer != Slot;
					}
					if (bAlly)
					{
						return !PlayerState || !GameState
							|| HOLUnitPicking::GetSide(GameState->GetLaunchConfig(), View.OwnerPlayer, PlayerState->GetPlayerIndex()) != EHOLSelectionSide::Ally;
					}
					const bool bOwn = PlayerState && View.OwnerPlayer == PlayerState->GetPlayerIndex();
					return bEnemy ? bOwn || !PlayerState : !bOwn;
				});
				Views.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
				// Negative: from the last one (-1), whatever units the steps before made.
				const int32 N = Wanted < 0 ? Views.Num() + Wanted : Wanted;
				int32 SizeX = 0, SizeY = 0;
				Controller->GetViewportSize(SizeX, SizeY);
				FVector2D Middle;
				if (!Views.IsValidIndex(N) || SizeX <= 0 || SizeY <= 0 || !Controller->ProjectWorldLocationToScreen(
					Views[N].Location + FVector(0.0, 0.0, PickBodyHeight * 0.5 * Views[N].MeshScale), Middle))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.ClickUnit: no %s unit %d on the screen"), Slot != INDEX_NONE ? TEXT("such") : bAlly ? TEXT("ally") : bEnemy ? TEXT("enemy") : TEXT("own"), N);
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("%s on unit #%u at (%.3f, %.3f)"), bOrder ? TEXT("Right click") : TEXT("Click"), Views[N].UnitId, Middle.X / SizeX, Middle.Y / SizeY);
				const FVector2D Fraction(Middle.X / SizeX, Middle.Y / SizeY);
				if (bOrder)
				{
					Controller->OrderAt(Fraction, false);
					continue;
				}
				bDouble ? Controller->SelectSameTypeAt(Fraction, bAdd) : Controller->SelectAt(Fraction, bAdd);
			}
		}));

	FAutoConsoleCommand BoxCommand(
		TEXT("hol.Select.Box"),
		TEXT("hol.Select.Box <X1> <Y1> <X2> <Y2> [add]: selects as a box drawn between those points of the viewport (0 to 1 each way) would (any side's units, the first side present kept: own > ally > enemy > neutral); 'add' as with Shift"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 4)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Box <X1> <Y1> <X2> <Y2> [add]"));
				return;
			}
			const bool bAdd = Args.Num() >= 5 && Args[4] == TEXT("add");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectInBox(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])),
					FVector2D(FCString::Atod(*Args[2]), FCString::Atod(*Args[3])), bAdd);
			}
		}));

	/** A word among the arguments after the four numbers of hol.Select.OrderLine / OrderLineGround ("ctrl", "shift"). */
	bool HasLineFlag(const TArray<FString>& Args, const TCHAR* Flag)
	{
		for (int32 Index = 4; Index < Args.Num(); ++Index)
		{
			if (Args[Index].Equals(Flag, ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	FAutoConsoleCommand OrderLineCommand(
		TEXT("hol.Select.OrderLine"),
		TEXT("hol.Select.OrderLine <X1> <Y1> <X2> <Y2> [ctrl] [shift]: a right-button drag between those points of the viewport (0 to 1 each way): an arrow, the group's front row centred on its base, facing along it; 'ctrl' its length is the front's width, 'shift' queued"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 4)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.OrderLine <X1> <Y1> <X2> <Y2>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->OrderLineAt(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])),
					FVector2D(FCString::Atod(*Args[2]), FCString::Atod(*Args[3])), HasLineFlag(Args, TEXT("ctrl")), HasLineFlag(Args, TEXT("shift")));
			}
		}));

	FAutoConsoleCommand OrderCommand(
		TEXT("hol.Select.Order"),
		TEXT("hol.Select.Order <X> <Y> [shift]: orders as a right click at that point of the viewport (0 to 1 each way) would; 'shift' as with Shift held"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Order <X> <Y> [shift]"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->OrderAt(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])), Args.Num() >= 3 && Args[2] == TEXT("shift"));
			}
		}));

	/** Where a ground point (cm) is drawn, as a fraction of the viewport; false when it is not on the screen. */
	bool ProjectGroundPoint(const AHOLPlayerController& Controller, const double X, const double Y, FVector2D& OutFraction)
	{
		int32 SizeX = 0, SizeY = 0;
		Controller.GetViewportSize(SizeX, SizeY);
		FVector2D Pixel;
		if (SizeX <= 0 || SizeY <= 0 || !Controller.ProjectWorldLocationToScreen(FVector(X, Y, 0.), Pixel))
		{
			return false;
		}
		OutFraction = FVector2D(Pixel.X / SizeX, Pixel.Y / SizeY);
		return OutFraction.X >= 0. && OutFraction.X <= 1. && OutFraction.Y >= 0. && OutFraction.Y <= 1.;
	}

	/** The box and the right click on ground points (J4.7, D289): a script knows the map, not where it is drawn. */
	FAutoConsoleCommand BoxGroundCommand(
		TEXT("hol.Select.BoxGround"),
		TEXT("hol.Select.BoxGround <X1> <Y1> <X2> <Y2> [add]: hol.Select.Box around where that ground rectangle (cm) is drawn"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 4)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.BoxGround <X1> <Y1> <X2> <Y2> [add]"));
				return;
			}
			const double X1 = FCString::Atod(*Args[0]), Y1 = FCString::Atod(*Args[1]), X2 = FCString::Atod(*Args[2]), Y2 = FCString::Atod(*Args[3]);
			const bool bAdd = Args.Num() >= 5 && Args[4] == TEXT("add");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FBox2D Box(ForceInit);
				bool bOnScreen = true;
				for (const FVector2D& Corner : { FVector2D(X1, Y1), FVector2D(X1, Y2), FVector2D(X2, Y1), FVector2D(X2, Y2) })
				{
					FVector2D Fraction;
					bOnScreen &= ProjectGroundPoint(*Controller, Corner.X, Corner.Y, Fraction);
					Box += Fraction;
				}
				if (!bOnScreen)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.BoxGround: the rectangle is not wholly on the screen"));
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Box on the ground from (%.3f, %.3f) to (%.3f, %.3f)"), Box.Min.X, Box.Min.Y, Box.Max.X, Box.Max.Y);
				Controller->SelectInBox(Box.Min, Box.Max, bAdd);
			}
		}));

	FAutoConsoleCommand OrderGroundCommand(
		TEXT("hol.Select.OrderGround"),
		TEXT("hol.Select.OrderGround <X> <Y> [shift]: hol.Select.Order where that ground point (cm) is drawn"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.OrderGround <X> <Y> [shift]"));
				return;
			}
			const bool bShift = Args.Num() >= 3 && Args[2] == TEXT("shift");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector2D Fraction;
				if (!ProjectGroundPoint(*Controller, FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), Fraction))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.OrderGround: the point is not on the screen"));
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Right click on the ground at (%.3f, %.3f)%s"), Fraction.X, Fraction.Y, bShift ? TEXT(", Shift") : TEXT(""));
				Controller->OrderAt(Fraction, bShift);
			}
		}));

	/** The right-button drag on ground points (J7a.4, quick_check "formation"): hol.Select.OrderLine where they are drawn. */
	FAutoConsoleCommand OrderLineGroundCommand(
		TEXT("hol.Select.OrderLineGround"),
		TEXT("hol.Select.OrderLineGround <X1> <Y1> <X2> <Y2> [ctrl] [shift]: hol.Select.OrderLine between where those ground points (cm) are drawn"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 4)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.OrderLineGround <X1> <Y1> <X2> <Y2>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector2D From, To;
				if (!ProjectGroundPoint(*Controller, FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), From)
					|| !ProjectGroundPoint(*Controller, FCString::Atod(*Args[2]), FCString::Atod(*Args[3]), To))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.OrderLineGround: a point is not on the screen"));
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Right drag on the ground from (%.3f, %.3f) to (%.3f, %.3f)"), From.X, From.Y, To.X, To.Y);
				Controller->OrderLineAt(From, To, HasLineFlag(Args, TEXT("ctrl")), HasLineFlag(Args, TEXT("shift")));
			}
		}));

	/**
	 * Where the standing source nearest a ground point is drawn (a fraction of the viewport), aimed at the middle of its
	 * trunk: inside its drawn body, in front of the ground behind it. INDEX_NONE (logged for Command) when there is none
	 * or it is not on the screen.
	 */
	int32 AimAtSource(const AHOLPlayerController& Controller, const double X, const double Y, const TCHAR* Command, FVector2D& OutFraction)
	{
		const UHOLSourceSubsystem* Sources = Controller.GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
		const int32 Index = Sources ? Sources->FindNearestStanding(FVector(X, Y, 0.)) : INDEX_NONE;
		if (Index == INDEX_NONE)
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("%s: no standing source"), Command);
			return INDEX_NONE;
		}
		const FHOLSource& Source = Sources->GetSources()[Index];
		const FHOLSourceTypeRow* Type = Sources->GetType(Source.TypeIndex);
		int32 SizeX = 0, SizeY = 0;
		Controller.GetViewportSize(SizeX, SizeY);
		FVector2D Pixel;
		if (!Type || SizeX <= 0 || SizeY <= 0 || !Controller.ProjectWorldLocationToScreen(Source.Location + FVector(0., 0., Type->Height * 0.3), Pixel))
		{
			UE_LOG(LogHOLMatch, Warning, TEXT("%s: the source is not on the screen"), Command);
			return INDEX_NONE;
		}
		OutFraction = FVector2D(Pixel.X / SizeX, Pixel.Y / SizeY);
		return Index;
	}

	/** A right click on the standing source nearest a ground point (J5, D299), where it is drawn: through the picking. */
	FAutoConsoleCommand OrderSourceCommand(
		TEXT("hol.Select.OrderSource"),
		TEXT("hol.Select.OrderSource <X> <Y> [shift]: a right click where the standing resource source nearest that ground point (cm) is drawn; 'shift' as with Shift held"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.OrderSource <X> <Y> [shift]"));
				return;
			}
			const bool bShift = Args.Num() >= 3 && Args[2] == TEXT("shift");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector2D Fraction;
				const int32 Index = AimAtSource(*Controller, FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), TEXT("hol.Select.OrderSource"), Fraction);
				if (Index != INDEX_NONE)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Right click on source %d at (%.3f, %.3f)%s"), Index, Fraction.X, Fraction.Y, bShift ? TEXT(", Shift") : TEXT(""));
					Controller->OrderAt(Fraction, bShift);
				}
			}
		}));

	/** A left click on the standing source nearest a ground point (J5b, D298): its block in the HUD. */
	FAutoConsoleCommand ClickSourceCommand(
		TEXT("hol.Select.ClickSource"),
		TEXT("hol.Select.ClickSource <X> <Y>: a left click where the standing resource source nearest that ground point (cm) is drawn"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.ClickSource <X> <Y>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector2D Fraction;
				const int32 Index = AimAtSource(*Controller, FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), TEXT("hol.Select.ClickSource"), Fraction);
				if (Index != INDEX_NONE)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Click on source %d at (%.3f, %.3f)"), Index, Fraction.X, Fraction.Y);
					Controller->SelectAt(Fraction);
				}
			}
		}));

	/** The combat orders (J7b.3), without the mouse: the selected own units attack a unit (its identifier). */
	FAutoConsoleCommand AttackCommand(
		TEXT("hol.Select.Attack"),
		TEXT("hol.Select.Attack <UnitId|slot=<S>> [shift]: the local player's selected units attack that unit (slot=<S>: slot S's first unit known here), without the right click's checks (the server checks it is an enemy alive)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 1)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Attack <UnitId> [shift]"));
				return;
			}
			const bool bQueue = Args.Num() > 1 && Args[1].Equals(TEXT("shift"), ESearchCase::IgnoreCase);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				uint32 TargetId = static_cast<uint32>(FCString::Atoi64(*Args[0]));
				if (Args[0].StartsWith(TEXT("slot=")))
				{
					// A script does not know the identifiers: slot S's first unit (allied combat, quick_check "sides").
					const int32 Slot = FCString::Atoi(*Args[0].RightChop(5));
					TArray<FHOLUnitView> Views;
					HOLUnitView::Gather(*Controller->GetWorld(), Views);
					TargetId = 0;
					for (const FHOLUnitView& View : Views)
					{
						TargetId = View.OwnerPlayer == Slot && (TargetId == 0 || View.UnitId < TargetId) ? View.UnitId : TargetId;
					}
				}
				const TArray<uint32> Own = Controller->GetOwnSelectedUnits();
				UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units attack #%u%s"), Own.Num(), TargetId, bQueue ? TEXT(", queued") : TEXT(""));
				Controller->RequestAttackUnit(Own, TargetId, bQueue);
				FHOLUnitView View;
				if (HOLUnitView::Find(*Controller->GetWorld(), TargetId, View))
				{
					Controller->OnAttackOrderGiven.Broadcast(View.Location);
				}
			}
		}));

	FAutoConsoleCommand AttackMoveCommand(
		TEXT("hol.Select.AttackMove"),
		TEXT("hol.Select.AttackMove <X> <Y> [shift]: the grid's attack-move (Q) then a left click where that ground point (cm) is drawn"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.AttackMove <X> <Y> [shift]"));
				return;
			}
			const bool bQueue = Args.Num() > 2 && Args[2].Equals(TEXT("shift"), ESearchCase::IgnoreCase);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector2D Fraction;
				if (!ProjectGroundPoint(*Controller, FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), Fraction))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.AttackMove: the point is not on the screen"));
					continue;
				}
				Controller->AttackMoveAt(Fraction, bQueue);
			}
		}));

	FAutoConsoleCommand StopCommand(
		TEXT("hol.Select.Stop"),
		TEXT("hol.Select.Stop: the local player's selected units stop, as the orders' S"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->ActivateOrderCell(HOLCommandGrid::StopCell);
			}
		}));

	FAutoConsoleCommand HoldCommand(
		TEXT("hol.Select.Hold"),
		TEXT("hol.Select.Hold: the local player's selected units hold their position, as the orders' V"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->ActivateOrderCell(HOLCommandGrid::HoldCell);
			}
		}));

	/** The "." key (D30, J5b), without the keyboard. */
	FAutoConsoleCommand NextIdleCommand(
		TEXT("hol.Select.NextIdle"),
		TEXT("hol.Select.NextIdle: selects the local player's next idle worker and centres the camera on it, as the '.' key"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectNextIdleWorker();
			}
		}));

	/** The control groups' keys (D30, J7a.5), without the keyboard. */
	FAutoConsoleCommand GroupCommand(
		TEXT("hol.Select.Group"),
		TEXT("hol.Select.Group <Select|Add|Remove|Double> <0-9>: as the digit, Shift + digit (creates the group when empty), Ctrl + Shift + digit; Double, the digit pressed twice (the camera to the group)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Group = Args.Num() >= 2 && Args[1].IsNumeric() ? FCString::Atoi(*Args[1]) : INDEX_NONE;
			if (Group < 0 || Group >= AHOLPlayerController::NumControlGroups)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Group <Select|Add|Remove|Double> <0-9>"));
				return;
			}
			const FString& Mode = Args[0];
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				if (Mode == TEXT("Add"))
				{
					Controller->AddToControlGroup(Group);
				}
				else if (Mode == TEXT("Remove"))
				{
					Controller->RemoveFromControlGroup(Group);
				}
				else if (Mode == TEXT("Select"))
				{
					Controller->SelectControlGroup(Group);
				}
				else if (Mode == TEXT("Double"))
				{
					Controller->SelectControlGroup(Group);
					Controller->SelectControlGroup(Group);
				}
				else
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Group <Select|Add|Remove|Double> <0-9>"));
				}
			}
		}));

	/** Ctrl + A (D30, J7a.5), without the keyboard. */
	FAutoConsoleCommand ArmyCommand(
		TEXT("hol.Select.Army"),
		TEXT("hol.Select.Army: selects all the local player's soldiers (every unit but the Peasants), as Ctrl + A"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectArmy();
			}
		}));

	/** The F1, F2, F3 keys (D30, the user's trial of J6a.5), without the keyboard. */
	FAutoConsoleCommand CategoryCommand(
		TEXT("hol.Select.Category"),
		TEXT("hol.Select.Category <Military|Economic|Technological>: selects all the local player's buildings of the category, as F1, F2, F3; again, the camera goes to the next one"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int64 Value = Args.IsEmpty() ? INDEX_NONE : StaticEnum<EHOLBuildingCategory>()->GetValueByNameString(Args[0]);
			if (Value == INDEX_NONE)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Category <Military|Economic|Technological>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectBuildingCategory(static_cast<EHOLBuildingCategory>(Value));
			}
		}));

	/** The F4 key (D30, J8a-Q4), without the keyboard. */
	FAutoConsoleCommand HeroCommand(
		TEXT("hol.Select.Hero"),
		TEXT("hol.Select.Hero: selects the local player's hero alone, as F4; again (he alone selected), the camera on him"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectHero();
			}
		}));

	/** A cell of the command grid (J6a.5), as its key or a click on the HUD: 0 to 11, A Z E R / Q S D F / W X C V. */
	FAutoConsoleCommand GridCommand(
		TEXT("hol.Select.Grid"),
		TEXT("hol.Select.Grid <Cell>: presses the key of a cell (0 = A ... 11 = V), as the keyboard: the grid on the right (production), an age chosen (A Z E R, none chosen) then its buildings' ghosts (every key: 0 then 0 the house), or the orders on the left"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Grid <Cell>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->PressGridKey(FCString::Atoi(*Args[0]));
			}
		}));

	/** The orders' panel (the user's choice of 2026-10-21), as a click on its cell. */
	FAutoConsoleCommand OrderCellCommand(
		TEXT("hol.Select.OrderCell"),
		TEXT("hol.Select.OrderCell <Cell>: activates a cell of the orders' panel (4 = Q attack-move, 5 = S stop, 11 = V hold), as a click"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.OrderCell <Cell>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->ActivateOrderCell(FCString::Atoi(*Args[0]));
			}
		}));

	/** Tab (the user's 2nd trial of J6a.5), without the keyboard: the next subgroup of a group of buildings. */
	FAutoConsoleCommand SubgroupCommand(
		TEXT("hol.Select.Subgroup"),
		TEXT("hol.Select.Subgroup: the next subgroup of the selected group of buildings (All, then each type, each main centre alone), or of the selected units of several types, as Tab"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->CycleSubgroup();
			}
		}));

	/** The same from the player's own start (a script does not know which slot a client holds, J5a.5). */
	FAutoConsoleCommand OrderSourceNearCommand(
		TEXT("hol.Select.OrderSourceNear"),
		TEXT("hol.Select.OrderSourceNear [Dx Dy]: hol.Select.OrderSource at the player's start (its main centre's middle) plus (Dx, Dy) (cm)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				const AHOLStartSlot* Start = PlayerState ? AHOLStartSlot::Find(Controller->GetWorld(), PlayerState->GetPlayerIndex()) : nullptr;
				if (Start)
				{
					const FVector Point = Start->GetMainCentreLocation() + (Args.Num() >= 2 ? FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.) : FVector::ZeroVector);
					IConsoleManager::Get().ProcessUserConsoleInput(*FString::Printf(TEXT("hol.Select.OrderSource %.0f %.0f"), Point.X, Point.Y),
						*GLog, Controller->GetWorld());
				}
			}
		}));

	FAutoConsoleCommand SelectionReportCommand(
		TEXT("hol.Select.Report"),
		TEXT("hol.Select.Report: logs the local player's selection"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Selection report: selected %s"), *Controller->DescribeSelection());
				if (const AHOLSelectionMarkers* Markers = Controller->GetSelectionMarkers())
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Selection markers: %s"), *Markers->Describe());
				}
			}
		}));

	FAutoConsoleCommand UnitsBarsCommand(
		TEXT("hol.Units.Bars"),
		TEXT("hol.Units.Bars: logs the health bars the local player sees (J7b.5)"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				if (const AHOLSelectionMarkers* Markers = Controller->GetSelectionMarkers())
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Health bars: %s"), *Markers->DescribeHealthBars());
				}
			}
		}));

	FAutoConsoleCommand FocusSlotCommand(
		TEXT("hol.Camera.FocusSlot"),
		TEXT("hol.Camera.FocusSlot <Slot|own>: puts the local player's camera above a start slot's main centre, or its own (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const bool bOwn = !Args.IsEmpty() && Args[0] == TEXT("own");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				const int32 Slot = !bOwn ? (Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0])) : PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
				const AHOLStartSlot* Start = AHOLStartSlot::Find(Controller->GetWorld(), Slot);
				if (Start && Controller->GetRTSCamera())
				{
					Controller->GetRTSCamera()->SetFocus(Start->GetMainCentreLocation());
					UE_LOG(LogHOLMatch, Display, TEXT("Camera above the start of slot %d"), Slot);
				}
			}
		}));

	FAutoConsoleCommand FocusCommand(
		TEXT("hol.Camera.Focus"),
		TEXT("hol.Camera.Focus <X> <Y>: puts the local player's camera above a ground point (cm) (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				if (Controller->GetRTSCamera())
				{
					Controller->GetRTSCamera()->SetFocus(FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.));
				}
			}
		}));

	FAutoConsoleCommand ZoomCommand(
		TEXT("hol.Camera.Zoom"),
		TEXT("hol.Camera.Zoom <Steps>: zooms the local player's camera by mouse wheel steps, positive closer (scripted checks: close-ups of the animations, J5c.1)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.IsEmpty())
			{
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				if (Controller->GetRTSCamera())
				{
					Controller->GetRTSCamera()->Zoom(FCString::Atof(*Args[0]));
					UE_LOG(LogHOLMatch, Display, TEXT("Camera height %.0f cm"), Controller->GetRTSCamera()->GetHeight());
				}
			}
		}));

	/** The player's own building of a type (its main centre for the scripted checks), or null. */
	AHOLBuilding* FindOwnBuilding(const AHOLPlayerController& Controller, const FName TypeName)
	{
		const AHOLPlayerState* PlayerState = Controller.GetHOLPlayerState();
		for (TActorIterator<AHOLBuilding> It(Controller.GetWorld()); PlayerState && It; ++It)
		{
			if (It->GetOwnerPlayer() == PlayerState->GetPlayerIndex() && It->GetTypeName() == TypeName)
			{
				return *It;
			}
		}
		return nullptr;
	}

	/**
	 * Scripted checks of the orders (J4, D289), without the selection: the player's own units known on this machine
	 * (or every unit, to check that the server moves only the player's own) ordered to a ground point.
	 */
	FAutoConsoleCommand UnitsOrderCommand(
		TEXT("hol.Units.Order"),
		TEXT("hol.Units.Order X Y [all]: asks the server to move the local player's units known here to a ground point (cm); 'all' sends every unit, other players' too"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				return;
			}
			const FVector Target(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			const bool bAll = Args.Num() >= 3 && Args[2] == TEXT("all");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				TArray<FHOLUnitView> Views;
				HOLUnitView::Gather(*Controller->GetWorld(), Views);
				TArray<uint32> UnitIds;
				for (const FHOLUnitView& View : Views)
				{
					if (bAll || (PlayerState && View.OwnerPlayer == PlayerState->GetPlayerIndex()))
					{
						UnitIds.Add(View.UnitId);
					}
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units to (%.0f, %.0f)%s"), UnitIds.Num(), Target.X, Target.Y, bAll ? TEXT(", other players' too") : TEXT(""));
				Controller->RequestMoveUnits(UnitIds, Target);
			}
		}));

	/** Scripted checks of the building (J6a.4): the player's units selected wherever they are, on the screen or not. */
	FAutoConsoleCommand SelectOwnCommand(
		TEXT("hol.Select.Own"),
		TEXT("hol.Select.Own: selects every unit of the local player known here, on the screen or not"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				TArray<FHOLUnitView> Views;
				HOLUnitView::Gather(*Controller->GetWorld(), Views);
				TArray<uint32> UnitIds;
				for (const FHOLUnitView& View : Views)
				{
					if (PlayerState && View.OwnerPlayer == PlayerState->GetPlayerIndex())
					{
						UnitIds.Add(View.UnitId);
					}
				}
				UnitIds.Sort();
				// Logged even when they already were (the selection then logs nothing).
				UE_LOG(LogHOLMatch, Display, TEXT("Select own: %d units"), UnitIds.Num());
				Controller->SetUnitSelection(MoveTemp(UnitIds));
			}
		}));

	/** The same, to a point given from the player's own start (a script does not know which slot a client holds). */
	FAutoConsoleCommand UnitsOrderNearCommand(
		TEXT("hol.Units.OrderNear"),
		TEXT("hol.Units.OrderNear Dx Dy [all]: hol.Units.Order to the player's start (its main centre's middle) plus (Dx, Dy) (cm)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				return;
			}
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				const AHOLStartSlot* Start = PlayerState ? AHOLStartSlot::Find(Controller->GetWorld(), PlayerState->GetPlayerIndex()) : nullptr;
				if (Start)
				{
					const FVector Target = Start->GetMainCentreLocation() + FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
					IConsoleManager::Get().ProcessUserConsoleInput(*FString::Printf(TEXT("hol.Units.Order %.0f %.0f %s"), Target.X, Target.Y,
						Args.Num() >= 3 ? *Args[2] : TEXT("")), *GLog, Controller->GetWorld());
				}
			}
		}));

	/** A left click on a ground point (cm), where it is drawn: a foundation (J6a.3) or anything else under it. */
	FAutoConsoleCommand ClickGroundCommand(
		TEXT("hol.Select.ClickGround"),
		TEXT("hol.Select.ClickGround <X> <Y> [add]: hol.Select.Click where that ground point (cm) is drawn"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.ClickGround <X> <Y> [add]"));
				return;
			}
			const bool bAdd = Args.Num() >= 3 && Args[2] == TEXT("add");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector2D Fraction;
				if (!ProjectGroundPoint(*Controller, FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), Fraction))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.ClickGround: the point is not on the screen"));
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Click on the ground at (%.3f, %.3f)"), Fraction.X, Fraction.Y);
				Controller->SelectAt(Fraction, bAdd);
			}
		}));

	/** The main centre of the player (or, with bEnemy, of the first other player known here), or null. */
	const AHOLBuilding* FindMainCentre(const AHOLPlayerController& Controller, const bool bEnemy)
	{
		const AHOLPlayerState* PlayerState = Controller.GetHOLPlayerState();
		for (TActorIterator<AHOLBuilding> It(Controller.GetWorld()); PlayerState && It; ++It)
		{
			if (It->GetTypeName() == HOLBuildingTypeNames::MainCentre && (It->GetOwnerPlayer() == PlayerState->GetPlayerIndex()) != bEnemy)
			{
				return *It;
			}
		}
		return nullptr;
	}

	/** Development and scripted checks of the placement (J6a.3, D311), until the Peasant's grid of J6a.5. */
	FAutoConsoleCommand BuildPlaceCommand(
		TEXT("hol.Build.Place"),
		TEXT("hol.Build.Place <House|LumberCamp|MiningCamp|Mill|Barracks|ArcheryRange>: placement mode, a ghost under the cursor (left click places, Shift + click places and stays, Escape or right click leave)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const FName TypeName(Args.IsEmpty() ? TEXT("House") : *Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->BeginPlacement(TypeName);
			}
		}));

	FAutoConsoleCommand BuildClickCommand(
		TEXT("hol.Build.Click"),
		TEXT("hol.Build.Click [shift]: a left click in placement mode, where the ghost is (with Shift: stays in the mode)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const bool bKeep = !Args.IsEmpty() && Args[0] == TEXT("shift");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->ClickPlacement(bKeep);
			}
		}));

	/** Logs what this machine thinks of a placement, then asks the server (which checks everything again). */
	void RequestPlacementAt(AHOLPlayerController& Controller, const FName TypeName, const FVector& Point)
	{
		const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
		if (!Type)
		{
			return;
		}
		const FBox2D Footprint = UHOLConstructionSubsystem::SnapFootprint(*Type, Point);
		FString Reason;
		const bool bValid = Controller.CheckPlacementHere(TypeName, Footprint, Reason);
		UE_LOG(LogHOLMatch, Display, TEXT("Build request: %s at (%.0f, %.0f), here %s"), *TypeName.ToString(), Footprint.GetCenter().X,
			Footprint.GetCenter().Y, bValid ? TEXT("valid") : *FString::Printf(TEXT("refused (%s)"), *Reason));
		Controller.RequestPlaceBuilding(TypeName, Point);
	}

	FAutoConsoleCommand BuildPlaceNearCommand(
		TEXT("hol.Build.PlaceNear"),
		TEXT("hol.Build.PlaceNear <Type> <Dx> <Dy> [enemy]: asks the server for a foundation at the player's main centre (or an enemy's) plus (Dx, Dy) cm in its frame (X toward the map's middle), whatever this machine thinks of it"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 3)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Build.PlaceNear <Type> <Dx> <Dy> [enemy]"));
				return;
			}
			const bool bEnemy = Args.Num() >= 4 && Args[3] == TEXT("enemy");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLBuilding* Centre = FindMainCentre(*Controller, bEnemy);
				if (!Centre)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Build.PlaceNear: no %s main centre known here"), bEnemy ? TEXT("enemy") : TEXT("own"));
					continue;
				}
				const FVector Offset = Centre->GetActorRotation().RotateVector(FVector(FCString::Atod(*Args[1]), FCString::Atod(*Args[2]), 0.));
				RequestPlacementAt(*Controller, FName(*Args[0]), Centre->GetActorLocation() + Offset);
			}
		}));

	/** J7a.6 (quick_check "net"): a ground point given in the frame of the player's main centre (or an enemy's), as hol.Build.PlaceNear. */
	bool GetPointNear(const AHOLPlayerController& Controller, const TArray<FString>& Args, FVector& OutPoint)
	{
		const bool bEnemy = Args.Num() >= 3 && Args[2] == TEXT("enemy");
		const AHOLBuilding* Centre = Args.Num() >= 2 ? FindMainCentre(Controller, bEnemy) : nullptr;
		if (!Centre)
		{
			return false;
		}
		OutPoint = Centre->GetActorLocation() + Centre->GetActorRotation().RotateVector(FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.));
		OutPoint.Z = 0.;
		return true;
	}

	FAutoConsoleCommand CameraFocusNearCommand(
		TEXT("hol.Camera.FocusNear"),
		TEXT("hol.Camera.FocusNear <Dx> <Dy> [enemy]: puts the camera above the player's main centre (or an enemy's) plus (Dx, Dy) cm in its frame (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector Point;
				if (GetPointNear(*Controller, Args, Point) && Controller->GetRTSCamera())
				{
					Controller->GetRTSCamera()->SetFocus(Point);
					UE_LOG(LogHOLMatch, Display, TEXT("Camera near: (%.0f, %.0f)"), Point.X, Point.Y);
				}
			}
		}));

	FAutoConsoleCommand OrderNearCommand(
		TEXT("hol.Select.OrderNear"),
		TEXT("hol.Select.OrderNear <Dx> <Dy> [enemy]: a right click (hol.Select.OrderGround) at the player's main centre (or an enemy's) plus (Dx, Dy) cm in its frame"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector Point;
				FVector2D Fraction;
				if (!GetPointNear(*Controller, Args, Point) || !ProjectGroundPoint(*Controller, Point.X, Point.Y, Fraction))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.OrderNear: the point is not on the screen"));
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Right click near the centre at (%.0f, %.0f)"), Point.X, Point.Y);
				Controller->OrderAt(Fraction, false);
			}
		}));

	FAutoConsoleCommand BuildPlaceAtSourceCommand(
		TEXT("hol.Build.PlaceAtSource"),
		TEXT("hol.Build.PlaceAtSource <Type> <X> <Y>: asks the server for a foundation on the standing resource source nearest that ground point (cm), whatever this machine thinks of it"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 3)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Build.PlaceAtSource <Type> <X> <Y>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const UHOLSourceSubsystem* Sources = Controller->GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
				const int32 Index = Sources ? Sources->FindNearestStanding(FVector(FCString::Atod(*Args[1]), FCString::Atod(*Args[2]), 0.)) : INDEX_NONE;
				if (Index == INDEX_NONE)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Build.PlaceAtSource: no standing source"));
					continue;
				}
				const FHOLSource& Source = Sources->GetSources()[Index];
				UE_LOG(LogHOLMatch, Display, TEXT("Build on source %d at (%.0f, %.0f)"), Index, Source.Location.X, Source.Location.Y);
				RequestPlacementAt(*Controller, FName(*Args[0]), Source.Location);
			}
		}));

	FAutoConsoleCommand BuildCancelSelectedCommand(
		TEXT("hol.Build.CancelSelected"),
		TEXT("hol.Build.CancelSelected: asks the server to cancel the selected building of the player not yet finished (refunded, D311)"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCancelBuilding(Controller->GetOwnSelection());
			}
		}));

	const TCHAR* DescribeState(const EHOLConstructionState State)
	{
		switch (State)
		{
		case EHOLConstructionState::Foundation: return TEXT("foundation");
		case EHOLConstructionState::UnderConstruction: return TEXT("under construction");
		default: return TEXT("finished");
		}
	}

	FAutoConsoleCommand BuildReportCommand(
		TEXT("hol.Build.Report"),
		TEXT("hol.Build.Report: logs the local player's stock and every building known here, its state and place"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				TArray<FString> Buildings;
				for (TActorIterator<AHOLBuilding> It(Controller->GetWorld()); It; ++It)
				{
					const bool bOwn = PlayerState && It->GetOwnerPlayer() == PlayerState->GetPlayerIndex();
					// A site's progress (J6a.4), as this machine knows it (its work replicated).
					const FString Progress = It->GetConstructionState() == EHOLConstructionState::UnderConstruction
						? FString::Printf(TEXT(" %d%%"), FMath::FloorToInt32(It->GetProgress() * 100.f)) : FString();
					Buildings.Add(FString::Printf(TEXT("%s of slot %d (%s) %s%s at (%.0f, %.0f)"), *It->GetTypeName().ToString(), It->GetOwnerPlayer(),
						bOwn ? TEXT("own") : TEXT("other"), DescribeState(It->GetConstructionState()), *Progress, It->GetActorLocation().X, It->GetActorLocation().Y));
				}
				Buildings.Sort();
				UE_LOG(LogHOLMatch, Display, TEXT("Build report: %s; %d buildings [%s]"), PlayerState ? *PlayerState->DescribeEconomy() : TEXT("no player state"),
					Buildings.Num(), *FString::Join(Buildings, TEXT("; ")));
			}
		}));

	FAutoConsoleCommand ProductionRallyCommand(
		TEXT("hol.Prod.Rally"),
		TEXT("hol.Prod.Rally X Y: asks the server to put the rally point of the selected building (own), or of the player's main centre (selected first), at a ground point (cm); on the building, clears it"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				if (!Controller->GetOwnSelection())
				{
					Controller->SetSelection(FindOwnBuilding(*Controller, HOLBuildingTypeNames::MainCentre));
				}
				Controller->RequestRallyPoint(FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.));
			}
		}));

	/** Scripted checks of the production (D284), without the HUD: what a cell of the grid does. */
	FAutoConsoleCommand ProductionQueueCommand(
		TEXT("hol.Prod.Queue"),
		TEXT("hol.Prod.Queue [UnitType=Peasant] [Building]: asks the server to produce one unit at the selected building (own), or at the player's main centre (selected first) when none is"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const FName TypeName(Args.IsEmpty() ? TEXT("Peasant") : *Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				// J7a.6: [Building], its own building of that type selected first (the net part's barracks).
				if (Args.Num() >= 2)
				{
					Controller->SetSelection(FindOwnBuilding(*Controller, FName(*Args[1])));
				}
				if (!Controller->GetOwnSelection())
				{
					Controller->SetSelection(FindOwnBuilding(*Controller, HOLBuildingTypeNames::MainCentre));
				}
				Controller->RequestProduction(TypeName);
			}
		}));

	FAutoConsoleCommand ProductionCancelCommand(
		TEXT("hol.Prod.Cancel"),
		TEXT("hol.Prod.Cancel <Index>: asks the server to cancel an item of the selected building's queue (own)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Index = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCancelProduction(Index);
			}
		}));

	FAutoConsoleCommand ProductionReportCommand(
		TEXT("hol.Prod.Report"),
		TEXT("hol.Prod.Report: logs the local player's economy and the queue of its selected building"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				const AHOLBuilding* Building = Controller->GetOwnSelection();
				TArray<FString> Items;
				if (Building)
				{
					const UHOLUnitSubsystem* Units = Controller->GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
					const double Step = UHOLProductionSubsystem::GetDisplayStep(Controller->GetWorld());
					const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
					for (const FHOLProductionItem& Item : Building->GetProductionQueue())
					{
						const FHOLUnitTypeRow* Row = Units ? Units->FindTypeRow(Item.TypeName) : nullptr;
						Items.Add(Item.IsStarted() && Row
							? FString::Printf(TEXT("%s %.2f"), *Item.TypeName.ToString(),
								HOLProduction::GetProgress(Item, Step, UHOLProductionSubsystem::GetBuildSteps(*Row, StepSeconds)))
							: FString::Printf(TEXT("%s waiting"), *Item.TypeName.ToString()));
					}
				}
				// The other players' economy as this machine sees it: a client is sent its own only (D282).
				TArray<FString> Others;
				if (const AGameStateBase* GameState = Controller->GetWorld()->GetGameState())
				{
					for (const APlayerState* Other : GameState->PlayerArray)
					{
						const AHOLPlayerState* OtherState = Cast<AHOLPlayerState>(Other);
						if (OtherState && OtherState != PlayerState)
						{
							Others.Add(FString::Printf(TEXT("slot %d %s"), OtherState->GetPlayerIndex(), *OtherState->DescribeEconomy()));
						}
					}
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Production report: %s, selected %s, queue %d [%s]; others [%s]"),
					PlayerState ? *PlayerState->DescribeEconomy() : TEXT("no player state"),
					*Controller->DescribeBuilding(Controller->GetSelection()), Items.Num(), *FString::Join(Items, TEXT(", ")),
					*FString::Join(Others, TEXT("; ")));
			}
		}));

	/** D356, until the in-game menu of V1.3: the local players give up (the server checks it). */
	FAutoConsoleCommand VictorySurrenderCommand(
		TEXT("hol.Victory.Surrender"),
		TEXT("hol.Victory.Surrender: the local player gives up, an immediate defeat (D356)"),
		FConsoleCommandDelegate::CreateLambda([]
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestSurrender();
			}
		}));

#if !UE_BUILD_SHIPPING
	/**
	 * Development cheats (the user's trials of J4): console commands, sent to the server by the local players'
	 * controllers. None of them exists in a Shipping build.
	 */
	void CheatResources(const TArray<FString>& Args, const FHOLResources& PerUnit)
	{
		const int32 Amount = Args.IsEmpty() ? 1000 : FCString::Atoi(*Args[0]);
		for (AHOLPlayerController* Controller : GetLocalControllers())
		{
			Controller->RequestCheatResources(FHOLResources(PerUnit.Food * Amount, PerUnit.Wood * Amount, PerUnit.Stone * Amount, PerUnit.Gold * Amount));
		}
	}

	FAutoConsoleCommand CheatResourcesCommand(
		TEXT("hol.Cheat.Resources"),
		TEXT("hol.Cheat.Resources [N=1000]: adds N of each resource to the local player's stock (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { CheatResources(Args, FHOLResources(1, 1, 1, 1)); }));

	FAutoConsoleCommand CheatFoodCommand(
		TEXT("hol.Cheat.Food"),
		TEXT("hol.Cheat.Food [N=1000]: adds N food to the local player's stock (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { CheatResources(Args, FHOLResources(1, 0, 0, 0)); }));

	FAutoConsoleCommand CheatWoodCommand(
		TEXT("hol.Cheat.Wood"),
		TEXT("hol.Cheat.Wood [N=1000]: adds N wood to the local player's stock (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { CheatResources(Args, FHOLResources(0, 1, 0, 0)); }));

	FAutoConsoleCommand CheatStoneCommand(
		TEXT("hol.Cheat.Stone"),
		TEXT("hol.Cheat.Stone [N=1000]: adds N stone to the local player's stock (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { CheatResources(Args, FHOLResources(0, 0, 1, 0)); }));

	FAutoConsoleCommand CheatGoldCommand(
		TEXT("hol.Cheat.Gold"),
		TEXT("hol.Cheat.Gold [N=1000]: adds N gold to the local player's stock (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args) { CheatResources(Args, FHOLResources(0, 0, 0, 1)); }));

	FAutoConsoleCommand CheatFastBuildCommand(
		TEXT("hol.Cheat.FastBuild"),
		TEXT("hol.Cheat.FastBuild [1|0]: production in one step and every site finished at its first blow (a builder still has to reach it) for every player, or back to the table's times (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const bool bOn = Args.IsEmpty() || FCString::Atoi(*Args[0]) != 0;
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatFastBuild(bOn);
			}
		}));

	FAutoConsoleCommand CheatPopulationCommand(
		TEXT("hol.Cheat.Population"),
		TEXT("hol.Cheat.Population [N=165]: adds N places to the local player's population cap, 175 at most (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Places = Args.IsEmpty() ? 165 : FCString::Atoi(*Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatPopulation(Places);
			}
		}));

	FAutoConsoleCommand CheatUnitSpeedCommand(
		TEXT("hol.Cheat.UnitSpeed"),
		TEXT("hol.Cheat.UnitSpeed [Scale=1]: every unit of every player walks Scale times as fast (0.1 to 10; 1: the table's speeds; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const float Scale = Args.IsEmpty() ? 1.f : FCString::Atof(*Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatUnitSpeed(Scale);
			}
		}));

	/** The attack type named by a cheat's argument (melee by default: full damage on a building, D345). */
	uint8 ParseAttackType(const TArray<FString>& Args, const int32 Index)
	{
		const FString Name = Args.IsValidIndex(Index) ? Args[Index].ToLower() : FString();
		const EHOLAttackType Type = Name == TEXT("ranged") ? EHOLAttackType::Ranged : Name == TEXT("siege") ? EHOLAttackType::Siege
			: Name == TEXT("hero") ? EHOLAttackType::Hero : EHOLAttackType::Melee;
		return static_cast<uint8>(Type);
	}

	FAutoConsoleCommand CheatDamageNearCommand(
		TEXT("hol.Cheat.DamageNear"),
		TEXT("hol.Cheat.DamageNear <Amount> <Dx> <Dy> [enemy|own] [melee|ranged|siege|hero]: one attack of Amount on the building nearest the player's main centre (or an enemy's) plus (Dx, Dy) cm in its frame, its armour taken off (D345.2a; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 3)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Cheat.DamageNear <Amount> <Dx> <Dy> [enemy|own] [melee|ranged|siege|hero]"));
				return;
			}
			const TArray<FString> PointArgs(&Args[1], FMath::Min(Args.Num() - 1, 3));
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector Point;
				if (GetPointNear(*Controller, PointArgs, Point))
				{
					Controller->RequestCheatDamageBuilding(Point, FCString::Atoi(*Args[0]), ParseAttackType(Args, 4));
				}
			}
		}));

	FAutoConsoleCommand CheatSpawnBuildingCommand(
		TEXT("hol.Cheat.SpawnBuilding"),
		TEXT("hol.Cheat.SpawnBuilding <Type> <Slot> <Dx> <Dy> [enemy]: a finished building of that type for that slot, free, at the player's main centre (or an enemy's) plus (Dx, Dy) cm in its frame, on the tile grid (D345.2b scenarios; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 4)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Cheat.SpawnBuilding <Type> <Slot> <Dx> <Dy> [enemy]"));
				return;
			}
			const TArray<FString> PointArgs(&Args[2], FMath::Min(Args.Num() - 2, 3));
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				FVector Point;
				if (GetPointNear(*Controller, PointArgs, Point))
				{
					Controller->RequestCheatSpawnBuilding(FName(*Args[0]), FCString::Atoi(*Args[1]), Point);
				}
			}
		}));

	FAutoConsoleCommand CheatDamageSelectedCommand(
		TEXT("hol.Cheat.DamageSelected"),
		TEXT("hol.Cheat.DamageSelected [Amount=100] [melee|ranged|siege|hero]: one attack of Amount on the selected building, its armour taken off (D345.2a; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Amount = Args.IsEmpty() ? 100 : FCString::Atoi(*Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				if (const AHOLBuilding* Building = Controller->GetSelection())
				{
					Controller->RequestCheatDamageBuilding(Building->GetActorLocation(), Amount, ParseAttackType(Args, 1));
				}
			}
		}));

	FAutoConsoleCommand VictoryDefeatCommand(
		TEXT("hol.Victory.Defeat"),
		TEXT("hol.Victory.Defeat <Slot>: that slot loses at once, an AI's too, reason Cheat (V1.1; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.IsEmpty() || !Args[0].IsNumeric())
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Victory.Defeat <Slot>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatDefeat(FCString::Atoi(*Args[0]));
			}
		}));

	FAutoConsoleCommand VictoryFallForumCommand(
		TEXT("hol.Victory.FallForum"),
		TEXT("hol.Victory.FallForum <Slot>: that slot's main forum at 0 health by the path of damage, a fall and a life less (V2.1; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.IsEmpty() || !Args[0].IsNumeric())
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Victory.FallForum <Slot>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatFallForum(FCString::Atoi(*Args[0]));
			}
		}));

	FAutoConsoleCommand VictorySkipRuinCommand(
		TEXT("hol.Victory.SkipRuin"),
		TEXT("hol.Victory.SkipRuin <Slot>: that slot's main forum leaves its phase at once, ruin -> rebuilding, rebuilding -> rebuilt (V2.1; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.IsEmpty() || !Args[0].IsNumeric())
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Victory.SkipRuin <Slot>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatSkipForum(FCString::Atoi(*Args[0]));
			}
		}));

	FAutoConsoleCommand VictoryHoldPresenceCommand(
		TEXT("hol.Victory.HoldPresence"),
		TEXT("hol.Victory.HoldPresence <Slot> <Attackers 0|1> <Defenders 0|1> [AttackerSlot] | <Slot> off: fixes who stands in the zone of that slot's forum down, until off (V3.1; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const bool bOff = Args.Num() >= 2 && Args[1].Equals(TEXT("off"), ESearchCase::IgnoreCase);
			if (Args.IsEmpty() || !Args[0].IsNumeric() || (!bOff && (Args.Num() < 3 || !Args[1].IsNumeric() || !Args[2].IsNumeric())))
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Victory.HoldPresence <Slot> <Attackers 0|1> <Defenders 0|1> [AttackerSlot] | <Slot> off"));
				return;
			}
			const int32 Slot = FCString::Atoi(*Args[0]);
			const bool bAttackers = !bOff && FCString::Atoi(*Args[1]) != 0;
			const bool bDefenders = !bOff && FCString::Atoi(*Args[2]) != 0;
			const int32 Attacker = !bOff && Args.Num() >= 4 && Args[3].IsNumeric() ? FCString::Atoi(*Args[3]) : INDEX_NONE;
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatHoldPresence(Slot, bOff, bAttackers, bDefenders, Attacker);
			}
		}));

	FAutoConsoleCommand CheatGatherSpeedCommand(
		TEXT("hol.Cheat.GatherSpeed"),
		TEXT("hol.Cheat.GatherSpeed [Scale=1]: every gatherer of every player gathers Scale times as fast (0.1 to 10; 1: the table's rates; development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const float Scale = Args.IsEmpty() ? 1.f : FCString::Atof(*Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->RequestCheatGatherSpeed(Scale);
			}
		}));

	/** For the user's trials of the combat (J7b): enemies where the camera looks, without typing a ground point. */
	FAutoConsoleCommand SpawnHereCommand(
		TEXT("hol.Units.SpawnHere"),
		TEXT("hol.Units.SpawnHere [Count=5] [Type=Spearman] [Slot=1] [Radius=600]: makes units of a slot around the local camera's focus, server or solo; free and over the population cap (development only)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLRTSCamera* Camera = Controller->GetRTSCamera();
				UWorld* World = Controller->GetWorld();
				UHOLUnitSubsystem* Units = World && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				if (!Camera || !Units)
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Units.SpawnHere: on the server or in solo, once the match runs"));
					continue;
				}
				FHOLUnitSpawnRequest Request;
				Request.Count = Args.Num() >= 1 ? FCString::Atoi(*Args[0]) : 5;
				Request.TypeName = FName(Args.Num() >= 2 ? *Args[1] : TEXT("Spearman"));
				Request.OwnerPlayer = static_cast<uint8>(Args.Num() >= 3 ? FCString::Atoi(*Args[2]) : 1);
				Request.Radius = Args.Num() >= 4 ? FCString::Atof(*Args[3]) : 600.f;
				const FVector Focus = Camera->GetFocus();
				Request.Center = FVector(Focus.X, Focus.Y, 0.);
				Units->RequestUnits(Request);
				UE_LOG(LogHOLMatch, Display, TEXT("SpawnHere: %d %s of slot %d around (%.0f, %.0f)"), Request.Count,
					*Request.TypeName.ToString(), Request.OwnerPlayer, Focus.X, Focus.Y);
			}
		}));
#endif

	/** For the scripted checks (D257): the delay lets a command given at start-up report once the match runs. */
	FAutoConsoleCommand ReportCommand(
		TEXT("hol.Camera.Report"),
		TEXT("hol.Camera.Report [DelaySeconds=0]: logs the focus and height of the local player's RTS camera"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const float Delay = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 0.f;
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float)
			{
				ReportCameras();
				return false;
			}), Delay);
		}));
}

void AHOLPlayerController::ClientSetHUD_Implementation(const TSubclassOf<AHUD> NewHUDClass)
{
	UClass* MatchHUDClass = StaticLoadClass(AHUD::StaticClass(), nullptr, UE::HeroesOfLiberty::Player::MatchHUDClassPath,
		nullptr, LOAD_Quiet | LOAD_NoWarn);
	Super::ClientSetHUD_Implementation(MatchHUDClass ? MatchHUDClass : NewHUDClass.Get());
}

AHOLPlayerController::AHOLPlayerController()
{
	bShowMouseCursor = true;
	// No pawn to follow: the view stays on the RTS camera.
	bAutoManageActiveCameraTarget = false;
}

void AHOLPlayerController::BeginPlay()
{
	using namespace UE::HeroesOfLiberty::Player;

	Super::BeginPlay();
	// A change of selection closes the chosen age (the user's 1st trial of G.1).
	OnSelectionChanged.AddWeakLambda(this, [this]()
	{
		SetChosenAge(HOLCommandGrid::GetChosenAgeAfter(ChosenAge, EHOLAgeEvent::SelectionChanged));
		// No soldier left to give the attack-move to (A.2): its mode ends, and its red cursor with it.
		if (bAttackMoveArmed && GetOwnSelectedSoldiers().IsEmpty())
		{
			bAttackMoveArmed = false;
		}
	});
	if (!IsLocalController())
	{
		return;
	}
	// The red attack cursor of the attack-move mode (A.2), a Slate widget: no asset.
	if (UGameViewportClient* Viewport = GetWorld()->GetGameViewport())
	{
		Viewport->SetSoftwareCursorWidget(EMouseCursor::Crosshairs, HOLAttackCursor::Make());
	}

	if (UEnhancedInputLocalPlayerSubsystem* Input = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (UInputMappingContext* Context = LoadObject<UInputMappingContext>(nullptr, MappingContextPath))
		{
			Input->AddMappingContext(Context, 0);
		}
		else
		{
			UE_LOG(LogHOLMatch, Error, TEXT("Game keys missing (%s): run Tools/Editor/make_game_input.py"), MappingContextPath);
		}
	}

	// Local only, never replicated (D255).
	FActorSpawnParameters Params;
	Params.Owner = this;
	RTSCamera = GetWorld()->SpawnActor<AHOLRTSCamera>(Params);
	if (const AHOLMapBounds* Bounds = AHOLMapBounds::Find(GetWorld()))
	{
		RTSCamera->SetArea(Bounds->GetArea());
	}
	else
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("No map bounds (AHOLMapBounds): the camera is not bounded"));
	}
	SetViewTarget(RTSCamera);

	SelectionMarkers = GetWorld()->SpawnActor<AHOLSelectionMarkers>(Params);
	SelectionMarkers->Init(this);

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	TryPlaceCamera();
}

void AHOLPlayerController::SetupInputComponent()
{
	using namespace UE::HeroesOfLiberty::Player;

	Super::SetupInputComponent();
	UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent);
	if (!Input)
	{
		return;
	}
	const TPair<const TCHAR*, FVector2D> PanActions[] = {
		{ TEXT("IA_Camera_Left"), FVector2D(-1.0, 0.0) },
		{ TEXT("IA_Camera_Right"), FVector2D(1.0, 0.0) },
		{ TEXT("IA_Camera_Up"), FVector2D(0.0, 1.0) },
		{ TEXT("IA_Camera_Down"), FVector2D(0.0, -1.0) },
	};
	for (const TPair<const TCHAR*, FVector2D>& Pan : PanActions)
	{
		if (const UInputAction* Action = LoadAction(Pan.Key))
		{
			Input->BindAction(Action, ETriggerEvent::Triggered, this, &AHOLPlayerController::HandlePan, Pan.Value);
		}
	}
	if (const UInputAction* Zoom = LoadAction(TEXT("IA_Camera_Zoom")))
	{
		Input->BindAction(Zoom, ETriggerEvent::Triggered, this, &AHOLPlayerController::HandleZoom);
	}
	if (const UInputAction* Drag = LoadAction(TEXT("IA_Camera_Drag")))
	{
		Input->BindAction(Drag, ETriggerEvent::Started, this, &AHOLPlayerController::HandleDragStarted);
		Input->BindAction(Drag, ETriggerEvent::Completed, this, &AHOLPlayerController::HandleDragCompleted);
	}
	if (const UInputAction* Select = LoadAction(TEXT("IA_Select")))
	{
		Input->BindAction(Select, ETriggerEvent::Started, this, &AHOLPlayerController::HandleSelectStarted);
		Input->BindAction(Select, ETriggerEvent::Completed, this, &AHOLPlayerController::HandleSelectCompleted);
	}
	if (const UInputAction* SelectAdd = LoadAction(TEXT("IA_SelectAdd")))
	{
		Input->BindAction(SelectAdd, ETriggerEvent::Started, this, &AHOLPlayerController::HandleSelectAdd, true);
		Input->BindAction(SelectAdd, ETriggerEvent::Completed, this, &AHOLPlayerController::HandleSelectAdd, false);
	}
	if (const UInputAction* Order = LoadAction(TEXT("IA_Order")))
	{
		Input->BindAction(Order, ETriggerEvent::Started, this, &AHOLPlayerController::HandleOrder);
		Input->BindAction(Order, ETriggerEvent::Completed, this, &AHOLPlayerController::HandleOrderCompleted);
	}
	if (const UInputAction* Deselect = LoadAction(TEXT("IA_Deselect")))
	{
		Input->BindAction(Deselect, ETriggerEvent::Started, this, &AHOLPlayerController::HandleDeselect);
	}
	if (const UInputAction* NextIdle = LoadAction(TEXT("IA_NextIdle")))
	{
		Input->BindAction(NextIdle, ETriggerEvent::Started, this, &AHOLPlayerController::HandleNextIdle);
	}
	// F1, F2, F3 (D30): the player's military, economic, technological buildings; F4: his hero (J8a-Q4).
	const TPair<const TCHAR*, EHOLBuildingCategory> CategoryKeys[] = { { TEXT("IA_MilitaryBuildings"), EHOLBuildingCategory::Military },
		{ TEXT("IA_EconomicBuildings"), EHOLBuildingCategory::Economic }, { TEXT("IA_TechnologicalBuildings"), EHOLBuildingCategory::Technological } };
	for (const TPair<const TCHAR*, EHOLBuildingCategory>& CategoryKey : CategoryKeys)
	{
		if (const UInputAction* Action = LoadAction(CategoryKey.Key))
		{
			Input->BindAction(Action, ETriggerEvent::Started, this, &AHOLPlayerController::HandleCategoryKey, CategoryKey.Value);
		}
	}
	if (const UInputAction* HeroKey = LoadAction(TEXT("IA_Hero")))
	{
		Input->BindAction(HeroKey, ETriggerEvent::Started, this, &AHOLPlayerController::HandleHeroKey);
	}
	// Tab (the user's 2nd trial of J6a.5): the next subgroup of a group of buildings.
	if (const UInputAction* NextSubgroup = LoadAction(TEXT("IA_NextSubgroup")))
	{
		Input->BindAction(NextSubgroup, ETriggerEvent::Started, this, &AHOLPlayerController::HandleSubgroupKey);
	}
	for (int32 Cell = 0; Cell < NumGridCells; ++Cell)
	{
		if (const UInputAction* GridKey = LoadAction(*FString::Printf(TEXT("IA_Grid_%02d"), Cell)))
		{
			Input->BindAction(GridKey, ETriggerEvent::Started, this, &AHOLPlayerController::HandleGridKey, Cell);
		}
	}
	for (int32 Group = 0; Group < NumControlGroups; ++Group)
	{
		if (const UInputAction* GroupKey = LoadAction(*FString::Printf(TEXT("IA_Group_%d"), Group)))
		{
			Input->BindAction(GroupKey, ETriggerEvent::Started, this, &AHOLPlayerController::HandleGroupKey, Group);
		}
	}
}

void AHOLPlayerController::PlayerTick(const float DeltaTime)
{
	// Input first (the bindings fill KeyPan), then the camera.
	Super::PlayerTick(DeltaTime);
	// Enter: a test scenario's continue key (waitkey, HOLTestSequence.cpp); free in game (IMC_UI's Enter is the menus').
	// Read from the viewport's key state, nothing bound nor consumed: the console open or a focused menu keeps it.
	if (IsLocalController() && WasInputKeyJustPressed(EKeys::Enter))
	{
		HandleContinueKey();
	}
	// The red attack cursor while Q waits for its point (A.2): every way out of the mode brings the arrow back.
	const EMouseCursor::Type Cursor = HOLCommandGrid::GetMouseCursor(bAttackMoveArmed, IsPlacing());
	if (IsLocalController() && CurrentMouseCursor != Cursor)
	{
		CurrentMouseCursor = Cursor;
		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().QueryCursor();
		}
	}
	if (!RTSCamera)
	{
		return;
	}
	if (!bCameraPlaced)
	{
		TryPlaceCamera();
	}
	if (bDragging)
	{
		UpdateDrag();
	}
	else
	{
		RTSCamera->Pan(KeyPan + GetEdgeScroll(), DeltaTime, UE::HeroesOfLiberty::Player::GetSpeedSetting());
	}
	KeyPan = FVector2D::ZeroVector;

	if (IsPlacing())
	{
		UpdatePlacementGhost();
	}

	SincePurge += DeltaTime;
	if (SincePurge >= UE::HeroesOfLiberty::Player::SelectionPurgeInterval)
	{
		SincePurge = 0.f;
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*GetWorld(), Views);
		PurgeSelection(Views);
		UpdateIdleWorkers(Views);
	}
}

void AHOLPlayerController::TryPlaceCamera()
{
	const AHOLPlayerState* HOLPlayerState = GetPlayerState<AHOLPlayerState>();
	const int32 Slot = HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE;
	if (!RTSCamera || Slot == INDEX_NONE)
	{
		return;
	}
	bCameraPlaced = true;
	const AHOLStartSlot* Start = AHOLStartSlot::Find(GetWorld(), Slot);
	if (!Start)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("No start for slot %d in this map: the camera stays where it is"), Slot);
		return;
	}
	const FVector Ground = Start->GetMainCentreLocation();
	RTSCamera->SetFocus(Ground);
	UE_LOG(LogHOLMatch, Display, TEXT("Camera above the start of slot %d (%.0f, %.0f)"), Slot, Ground.X, Ground.Y);
}

bool AHOLPlayerController::GetCursorFraction(FVector2D& OutFraction) const
{
	const FString TestCursor = UE::HeroesOfLiberty::Player::CVarTestCursor.GetValueOnGameThread();
	FString X, Y;
	if (TestCursor.Split(TEXT(" "), &X, &Y))
	{
		OutFraction = FVector2D(FCString::Atod(*X), FCString::Atod(*Y));
		return true;
	}
	// The real cursor scrolls only while the game window is the one in front, and only from inside it.
	const ULocalPlayer* LocalPlayer = GetLocalPlayer();
	const FViewport* Viewport = LocalPlayer && LocalPlayer->ViewportClient ? LocalPlayer->ViewportClient->Viewport : nullptr;
	int32 SizeX = 0, SizeY = 0;
	float MouseX = 0.f, MouseY = 0.f;
	GetViewportSize(SizeX, SizeY);
	if (!Viewport || !Viewport->IsForegroundWindow() || SizeX <= 0 || SizeY <= 0 || !GetMousePosition(MouseX, MouseY))
	{
		return false;
	}
	OutFraction = FVector2D(MouseX / SizeX, MouseY / SizeY);
	return OutFraction.X >= 0.0 && OutFraction.X <= 1.0 && OutFraction.Y >= 0.0 && OutFraction.Y <= 1.0;
}

FVector2D AHOLPlayerController::GetEdgeScroll() const
{
	using namespace UE::HeroesOfLiberty::Player;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	FVector2D Cursor;
	if ((Settings && !Settings->IsEdgeScrollEnabled()) || !GetCursorFraction(Cursor))
	{
		return FVector2D::ZeroVector;
	}
	// Screen Y grows downwards, the pan's Y upwards.
	return FVector2D(Cursor.X <= EdgeMargin ? -1.0 : Cursor.X >= 1.0 - EdgeMargin ? 1.0 : 0.0,
		Cursor.Y <= EdgeMargin ? 1.0 : Cursor.Y >= 1.0 - EdgeMargin ? -1.0 : 0.0);
}

void AHOLPlayerController::HandlePan(const FInputActionValue& Value, const FVector2D Direction)
{
	KeyPan += Direction;
}

void AHOLPlayerController::HandleZoom(const FInputActionValue& Value)
{
	if (RTSCamera)
	{
		RTSCamera->Zoom(Value.Get<float>());
	}
}

void AHOLPlayerController::HandleDragStarted()
{
	FVector Origin, Direction;
	bDragging = RTSCamera && DeprojectMousePositionToWorld(Origin, Direction) && RTSCamera->GetGroundPoint(Origin, Direction, DragAnchor);
}

void AHOLPlayerController::HandleDragCompleted()
{
	bDragging = false;
}

AHOLBuilding* AHOLPlayerController::FindBuildingAt(const FVector2D& ViewportFraction) const
{
	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	FVector Origin, Direction;
	if (SizeX <= 0 || SizeY <= 0
		|| !DeprojectScreenPositionToWorld(ViewportFraction.X * SizeX, ViewportFraction.Y * SizeY, Origin, Direction))
	{
		return nullptr;
	}
	// The selection channel: only the pieces of buildings block it (D262).
	FHitResult Hit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(HOLSelection), /*bTraceComplex*/ false);
	GetWorld()->LineTraceSingleByChannel(Hit, Origin, Origin + Direction * UE::HeroesOfLiberty::Player::SelectionTraceLength,
		HOLCollision::Selection, Params);
	return Cast<AHOLBuilding>(Hit.GetActor());
}

FVector2D AHOLPlayerController::ToPixels(const FVector2D& ViewportFraction) const
{
	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	return FVector2D(ViewportFraction.X * SizeX, ViewportFraction.Y * SizeY);
}

void AHOLPlayerController::GatherPickCandidates(const bool bOwnOnly, TArray<FHOLPickCandidate>& OutCandidates, TArray<uint32>& OutUnitIds,
	const bool bHostileOnly) const
{
	using namespace UE::HeroesOfLiberty::Player;

	OutCandidates.Reset();
	OutUnitIds.Reset();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if ((bOwnOnly || bHostileOnly) && !HOLPlayerState)
	{
		return;
	}
	FVector CameraLocation;
	FRotator CameraRotation;
	GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector CameraRight = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
	// Allied combat: a target is a unit of another camp (an ally's is not, as my own).
	const FHOLCampTable Camps = bHostileOnly ? FHOLCampTable::FromWorld(GetWorld()) : FHOLCampTable();

	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	for (const FHOLUnitView& View : Views)
	{
		if ((bOwnOnly && View.OwnerPlayer != HOLPlayerState->GetPlayerIndex())
			|| (bHostileOnly && !Camps.AreHostile(View.OwnerPlayer, HOLPlayerState->GetPlayerIndex())))
		{
			continue;
		}
		// Feet, top of the head and one side of the body, as drawn: a unit behind the camera is not on the screen.
		FHOLPickCandidate Candidate;
		FVector2D Side;
		const FVector Head = View.Location + FVector(0.0, 0.0, PickBodyHeight * View.MeshScale);
		if (!ProjectWorldLocationToScreen(View.Location, Candidate.Foot) || !ProjectWorldLocationToScreen(Head, Candidate.Head)
			|| !ProjectWorldLocationToScreen(View.Location + CameraRight * (View.Radius * PickWidthFactor), Side))
		{
			continue;
		}
		Candidate.HalfWidth = FVector2D::Distance(Candidate.Foot, Side);
		Candidate.Depth = FVector::Distance(CameraLocation, View.Location);
		OutCandidates.Add(Candidate);
		OutUnitIds.Add(View.UnitId);
	}
}

bool AHOLPlayerController::IsPickableSource(const FHOLSourceTypeRow& Type)
{
	// IA1.2 F7: a linked source (a farm field) has no shape of its own: its building is clicked.
	return !Type.Linked;
}

int32 AHOLPlayerController::FindSourceAt(const FVector2D& ViewportFraction) const
{
	const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
	if (!Sources)
	{
		return INDEX_NONE;
	}
	FVector CameraLocation;
	FRotator CameraRotation;
	GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector CameraRight = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);
	// Each source as drawn, a body from the ground to its top (the picking of the units, D288).
	TArray<FHOLPickCandidate> Candidates;
	TArray<int32> Indices;
	const TArray<FHOLSource>& All = Sources->GetSources();
	for (int32 Index = 0; Index < All.Num(); ++Index)
	{
		const FHOLSource& Source = All[Index];
		const FHOLSourceTypeRow* Type = Source.IsStanding() ? Sources->GetType(Source.TypeIndex) : nullptr;
		// IA1.2 F7: a farm field is never clicked, its farm is.
		if (!Type || !IsPickableSource(*Type))
		{
			continue;
		}
		FHOLPickCandidate Candidate;
		FVector2D Side;
		if (!ProjectWorldLocationToScreen(Source.Location, Candidate.Foot)
			|| !ProjectWorldLocationToScreen(Source.Location + FVector(0., 0., Type->Height), Candidate.Head)
			|| !ProjectWorldLocationToScreen(Source.Location + CameraRight * AHOLSourceField::GetDrawnRadius(*Type), Side))
		{
			continue;
		}
		Candidate.HalfWidth = FVector2D::Distance(Candidate.Foot, Side);
		Candidate.Depth = FVector::Distance(CameraLocation, Source.Location);
		Candidates.Add(Candidate);
		Indices.Add(Index);
	}
	const int32 Picked = HOLUnitPicking::PickAt(Candidates, ToPixels(ViewportFraction));
	return Indices.IsValidIndex(Picked) ? Indices[Picked] : INDEX_NONE;
}

uint32 AHOLPlayerController::FindUnitAt(const FVector2D& ViewportFraction) const
{
	TArray<FHOLPickCandidate> Candidates;
	TArray<uint32> UnitIds;
	GatherPickCandidates(/*bOwnOnly*/ false, Candidates, UnitIds);
	const int32 Index = HOLUnitPicking::PickAt(Candidates, ToPixels(ViewportFraction));
	return UnitIds.IsValidIndex(Index) ? UnitIds[Index] : 0;
}

uint32 AHOLPlayerController::FindTargetAt(const FVector2D& ViewportFraction, bool& bOutDirect) const
{
	using namespace UE::HeroesOfLiberty::Player;

	bOutDirect = false;
	TArray<FHOLPickCandidate> Candidates;
	TArray<uint32> UnitIds;
	GatherPickCandidates(/*bOwnOnly*/ false, Candidates, UnitIds, /*bHostileOnly*/ true);
	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	const double Margin = HOLUnitPicking::GetScaledMargin(FMath::Max(static_cast<double>(TargetMargin), 0.), SizeY);
	const int32 Index = HOLUnitPicking::PickNear(Candidates, ToPixels(ViewportFraction), Margin, bOutDirect);
	return UnitIds.IsValidIndex(Index) ? UnitIds[Index] : 0;
}

TArray<uint32> AHOLPlayerController::GetOwnSelectedUnits() const
{
	TArray<uint32> Own;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (SelectedUnits.IsEmpty() || !HOLPlayerState)
	{
		return Own;
	}
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	for (const FHOLUnitView& View : Views)
	{
		if (View.OwnerPlayer == HOLPlayerState->GetPlayerIndex() && SelectedUnits.Contains(View.UnitId))
		{
			Own.Add(View.UnitId);
		}
	}
	return Own;
}

void AHOLPlayerController::SelectAt(const FVector2D& ViewportFraction, const bool bAdd)
{
	// A unit before a building (D288): it stands in front of the building's base.
	const uint32 UnitId = FindUnitAt(ViewportFraction);
	if (UnitId == 0)
	{
		// Shift on nothing keeps the units taken so far.
		if (!bAdd || SelectedUnits.IsEmpty())
		{
			AHOLBuilding* Building = FindBuildingAt(ViewportFraction);
			const int32 SourceIndex = Building ? INDEX_NONE : FindSourceAt(ViewportFraction);
			if (SourceIndex != INDEX_NONE)
			{
				SetSourceSelection(SourceIndex);
			}
			else
			{
				SetSelection(Building);
			}
		}
		return;
	}
	// Shift (D.2): the first side of the selection and the unit wins (own > ally > enemy > neutral): an enemy is never
	// added to the player's own units; of the selection's side, it is added, or removed when it already is selected.
	TMap<uint32, EHOLSelectionSide> SideOf;
	TArray<uint32> Selected;
	EHOLSelectionSide SelectedSide = EHOLSelectionSide::Neutral;
	GatherSelectionSides(SideOf, Selected, SelectedSide);
	const EHOLSelectionSide* Side = SideOf.Find(UnitId);
	TArray<uint32> Units;
	HOLUnitPicking::CombineClick(Selected, SelectedSide, UnitId, Side ? *Side : EHOLSelectionSide::Enemy, bAdd, Units);
	SetUnitSelection(MoveTemp(Units));
}

void AHOLPlayerController::GatherSelectionSides(TMap<uint32, EHOLSelectionSide>& OutSideOf, TArray<uint32>& OutSelected,
	EHOLSelectionSide& OutSelectedSide) const
{
	OutSideOf.Reset();
	OutSelected.Reset();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	const int32 LocalPlayer = HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE;
	// D.3: the camps of the match's configuration (the game state's, replicated): a slot of the player's team is an ally.
	const AHOLGameState* GameState = GetWorld()->GetGameState<AHOLGameState>();
	const FHOLMatchConfig Config = GameState ? GameState->GetLaunchConfig() : FHOLMatchConfig::MakeDefault();
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	for (const FHOLUnitView& View : Views)
	{
		OutSideOf.Add(View.UnitId, HOLUnitPicking::GetSide(Config, View.OwnerPlayer, LocalPlayer));
	}
	TArray<EHOLSelectionSide> SelectedSides;
	for (const uint32 Id : SelectedUnits)
	{
		if (const EHOLSelectionSide* Side = OutSideOf.Find(Id))
		{
			OutSelected.Add(Id);
			SelectedSides.Add(*Side);
		}
	}
	HOLUnitPicking::GetTopSide(SelectedSides, OutSelectedSide);
}

void AHOLPlayerController::SelectSameTypeAt(const FVector2D& ViewportFraction, const bool bAdd)
{
	TArray<FHOLPickCandidate> Candidates;
	TArray<uint32> UnitIds;
	GatherPickCandidates(/*bOwnOnly*/ false, Candidates, UnitIds);
	const int32 Clicked = HOLUnitPicking::PickAt(Candidates, ToPixels(ViewportFraction));
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	TMap<uint32, const FHOLUnitView*> ById;
	for (const FHOLUnitView& View : Views)
	{
		ById.Add(View.UnitId, &View);
	}
	TArray<FHOLTypedPickCandidate> Typed;
	for (int32 Index = 0; Index < Candidates.Num(); ++Index)
	{
		const FHOLUnitView* const* View = ById.Find(UnitIds[Index]);
		FHOLTypedPickCandidate& Candidate = Typed.AddDefaulted_GetRef();
		Candidate.Pick = Candidates[Index];
		Candidate.TypeName = View ? (*View)->TypeName : NAME_None;
		Candidate.bOwn = View && HOLPlayerState && (*View)->OwnerPlayer == HOLPlayerState->GetPlayerIndex();
		Candidate.OwnerPlayer = View ? (*View)->OwnerPlayer : INDEX_NONE;
	}
	// Not on a unit known here (a building, nothing): a single click.
	if (!Typed.IsValidIndex(Clicked) || !ById.Contains(UnitIds[Clicked]))
	{
		SelectAt(ViewportFraction, bAdd);
		return;
	}
	int32 SizeX = 0, SizeY = 0;
	GetViewportSize(SizeX, SizeY);
	TArray<int32> Indices;
	HOLUnitPicking::PickSameTypeOnScreen(Typed, Clicked, FVector2D(SizeX, SizeY), Indices);
	// The player's own units of its type, or (D.2) its owner's, another player's. Shift adds them to the selection of
	// their side (as before, never removing them), the first side of the whole kept: enemies never join own units.
	TMap<uint32, EHOLSelectionSide> SideOf;
	TArray<uint32> Selected;
	EHOLSelectionSide SelectedSide = EHOLSelectionSide::Neutral;
	GatherSelectionSides(SideOf, Selected, SelectedSide);
	TArray<uint32> Picked;
	TArray<EHOLSelectionSide> PickedSides;
	for (const int32 Index : Indices)
	{
		Picked.Add(UnitIds[Index]);
		PickedSides.Add(SideOf.FindRef(UnitIds[Index], EHOLSelectionSide::Enemy));
	}
	TArray<uint32> Units;
	HOLUnitPicking::CombineBox(Selected, SelectedSide, Picked, PickedSides, bAdd, Units, /*bRemoveWhenAll*/ false);
	UE_LOG(LogHOLMatch, Display, TEXT("Select: %d units of type %s of slot %d on the screen"), Indices.Num(), *Typed[Clicked].TypeName.ToString(),
		Typed[Clicked].OwnerPlayer);
	SetUnitSelection(MoveTemp(Units));
}

void AHOLPlayerController::SelectInBox(const FVector2D& CornerA, const FVector2D& CornerB, const bool bAdd)
{
	// Every unit in the box (D.1): only the first side present is kept, Own > Ally > Enemy > Neutral (an enemy group
	// selected shows its panel and its bars, without commands).
	TArray<FHOLPickCandidate> Candidates;
	TArray<uint32> UnitIds;
	GatherPickCandidates(/*bOwnOnly*/ false, Candidates, UnitIds);
	TArray<int32> Indices;
	HOLUnitPicking::PickInBox(Candidates, ToPixels(CornerA), ToPixels(CornerB), Indices);
	// Shift (D.1): the first side of the selection and the box together wins; of the same side, the boxed ones are added,
	// or removed when all of them already are. The selection's side: its first one present (a single side since D.1).
	TMap<uint32, EHOLSelectionSide> SideOf;
	TArray<uint32> Selected;
	EHOLSelectionSide SelectedSide = EHOLSelectionSide::Neutral;
	GatherSelectionSides(SideOf, Selected, SelectedSide);
	TArray<uint32> Boxed;
	TArray<EHOLSelectionSide> BoxedSides;
	for (const int32 Index : Indices)
	{
		if (const EHOLSelectionSide* Side = SideOf.Find(UnitIds[Index]))
		{
			Boxed.Add(UnitIds[Index]);
			BoxedSides.Add(*Side);
		}
	}
	TArray<uint32> Units;
	HOLUnitPicking::CombineBox(Selected, SelectedSide, Boxed, BoxedSides, bAdd, Units);
	if (!Units.IsEmpty() && Units != SelectedUnits)
	{
		const EHOLSelectionSide* Side = SideOf.Find(Units[0]);
		UE_LOG(LogHOLMatch, Display, TEXT("Select: box, %d units (%s)"), Units.Num(),
			!Side ? TEXT("?") : *Side == EHOLSelectionSide::Own ? TEXT("own") : *Side == EHOLSelectionSide::Ally ? TEXT("ally")
			: *Side == EHOLSelectionSide::Enemy ? TEXT("enemy") : TEXT("neutral"));
	}
	SetUnitSelection(MoveTemp(Units));
}

void AHOLPlayerController::OrderAt(const FVector2D& ViewportFraction, const bool bQueue)
{
	// D357: a spectator's right click does nothing (no order bound to be refused).
	if (IsSpectator())
	{
		return;
	}
	// Shift held, or a script's Shift: a site or a source at the end of the units' queue (D311, the user's trial of J6a.4).
	const bool bShift = bSelectAddHeld || bQueue;
	const FVector2D Pixels = ToPixels(ViewportFraction);
	FVector Origin, Direction, Ground;
	if (!RTSCamera || !DeprojectScreenPositionToWorld(Pixels.X, Pixels.Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, Ground))
	{
		return;
	}
	// A resource source under the cursor (J5, D298): its ground point, where the brief mark shows.
	int32 SourceIndex = FindSourceAt(ViewportFraction);
	const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
	const FHOLSource* Source = SourceIndex != INDEX_NONE && Sources ? &Sources->GetSources()[SourceIndex] : nullptr;
	const TArray<uint32> Own = GetOwnSelectedUnits();
	// One of the player's sites not finished under the cursor (J6a.4, D312), or its main forum down (V2.2): its selected
	// units go and help build it.
	AHOLBuilding* Site = !Own.IsEmpty() ? FindBuildingAt(ViewportFraction) : nullptr;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (Site && HOLPlayerState && Site->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex() && Site->NeedsBuilders())
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units build %s%s"), Own.Num(), *DescribeBuilding(Site), bShift ? TEXT(", queued") : TEXT(""));
		RequestBuildUnits(Own, Site, bShift);
		OnOrderGiven.Broadcast(Site->GetActorLocation());
		return;
	}
	// IA1.2 F7: one of the player's farms finished under the cursor, no source drawn over it: its field is the source
	// clicked (FindSourceAt never takes it), gathered by the selected units or the rally point of the selected buildings,
	// as a source. An ally's or an enemy's farm stays a building (a move, an attack).
	if (!Source && Sources && HOLPlayerState && (!Own.IsEmpty() || GetOwnSelection() || HasBuildingGroup()))
	{
		AHOLBuilding* Farm = !Own.IsEmpty() ? Site : FindBuildingAt(ViewportFraction);
		const int32 Field = Farm && Farm->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex() ? Farm->FindLinkedSourceHere() : INDEX_NONE;
		if (Sources->GetSources().IsValidIndex(Field))
		{
			SourceIndex = Field;
			Source = &Sources->GetSources()[Field];
			UE_LOG(LogHOLMatch, Display, TEXT("Order: on farm %s: its field, source %d"), *DescribeBuilding(Farm), Field);
		}
	}
	if (!Own.IsEmpty())
	{
		// An enemy unit under the cursor (J7b.3): attacked, its place marked in red. A unit before a source (D288). Near
		// the cursor within the target margin (J7c, retour d'essai 8, loop 3: FindTargetAt), unless a source is drawn
		// under it: a click on a body always wins.
		// D345.2b: an enemy building under the cursor (finished, or a site this machine shows: a bare foundation of the enemy
		// is never drawn here), struck by the selected units; a unit's body drawn over it still wins.
		// Allied combat: an ally's unit or building is taken as my own (a move there, never an attack).
		const FHOLCampTable Camps = FHOLCampTable::FromWorld(GetWorld());
		AHOLBuilding* EnemyBuilding = Site && HOLPlayerState && Camps.AreHostile(Site->GetOwnerPlayer(), HOLPlayerState->GetPlayerIndex()) && Site->IsSeenHere()
			&& !Site->IsFoundation() ? Site : nullptr;
		bool bDirect = false;
		const uint32 UnderId = FindTargetAt(ViewportFraction, bDirect);
		FHOLUnitView Under;
		if (UnderId != 0 && (bDirect || (!Source && !EnemyBuilding)) && HOLPlayerState && HOLUnitView::Find(*GetWorld(), UnderId, Under)
			&& Camps.AreHostile(Under.OwnerPlayer, HOLPlayerState->GetPlayerIndex()))
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units attack #%u %s of slot %d%s"), Own.Num(), UnderId, *Under.TypeName.ToString(), Under.OwnerPlayer,
				bShift ? TEXT(", queued") : TEXT(""));
			RequestAttackUnit(Own, UnderId, bShift);
			OnAttackOrderGiven.Broadcast(Under.Location);
			return;
		}
		if (EnemyBuilding)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units attack %s%s"), Own.Num(), *DescribeBuilding(EnemyBuilding), bShift ? TEXT(", queued") : TEXT(""));
			RequestAttackBuilding(Own, EnemyBuilding, bShift);
			OnAttackOrderGiven.Broadcast(EnemyBuilding->GetActorLocation());
			return;
		}
		// D485 (EtapeDefini L): one of the player's finished buildings taking deliveries (its sites went above): the units
		// carrying what it takes unload there, the others walk to the ground point; the server sorts them (it knows the loads).
		const FHOLBuildingTypeRow* SiteType = Site ? Site->GetType() : nullptr;
		if (Site && SiteType && HOLPlayerState && Site->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex() && !Site->NeedsBuilders()
			&& Site->TakesDeliveries() && !SiteType->Accepts.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units drop off at %s%s"), Own.Num(), *DescribeBuilding(Site), bShift ? TEXT(", queued") : TEXT(""));
			RequestDropOffUnits(Own, Site, Ground, bShift);
			OnOrderGiven.Broadcast(Site->GetActorLocation());
			return;
		}
		if (Source)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units gather source %d (%s) at (%.0f, %.0f)%s"), Own.Num(), SourceIndex,
				*Sources->GetTypeNames()[Source->TypeIndex].ToString(), Source->Location.X, Source->Location.Y, bShift ? TEXT(", queued") : TEXT(""));
			RequestGatherUnits(Own, SourceIndex, bShift);
			OnOrderGiven.Broadcast(Source->Location);
			return;
		}
		UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units to (%.0f, %.0f)%s"), Own.Num(), Ground.X, Ground.Y, bShift ? TEXT(", queued") : TEXT(""));
		RequestMoveUnits(Own, Ground, bShift);
		OnOrderGiven.Broadcast(Ground);
	}
	else if (AHOLBuilding* Building = GetOwnSelection())
	{
		// A click on the building itself clears its rally point: the ground seen behind its roof is off its footprint.
		AHOLBuilding* Under = FindBuildingAt(ViewportFraction);
		const bool bOnBuilding = Under == Building;
		// One of the player's sites not finished (the user's trial of J6a.4): the units born here build it; or one of its
		// camps, finished or not: once it is, they gather the nearest source it accepts (D312, J6b.1); the flag on its
		// middle, as on a source.
		AHOLBuilding* RallySite = Under && !bOnBuilding && HOLPlayerState && Under->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex()
			&& (Under->NeedsBuilders() || Under->IsCamp()) ? Under : nullptr;
		const FVector Point = bOnBuilding ? Building->GetActorLocation() : RallySite ? RallySite->GetActorLocation() : Source ? Source->Location : Ground;
		const int32 RallySource = !bOnBuilding && !RallySite && Source ? SourceIndex : INDEX_NONE;
		UE_LOG(LogHOLMatch, Display, TEXT("Order: rally point of %s at (%.0f, %.0f)%s"), *DescribeBuilding(Building), Point.X, Point.Y,
			RallySite ? *FString::Printf(TEXT(" on site %s"), *DescribeBuilding(RallySite))
			: RallySource != INDEX_NONE ? *FString::Printf(TEXT(" on source %d"), RallySource) : TEXT(""));
		RequestRallyPoint(Point, RallySource, RallySite);
		OnOrderGiven.Broadcast(Point);
	}
	else if (HasBuildingGroup())
	{
		// A group (an F key, D30): the rally point of each of its buildings that produce, as for one; a click on one of
		// them clears nothing (each its own).
		TArray<AHOLBuilding*> Group;
		GetSelectedBuildings(Group);
		AHOLBuilding* Under = FindBuildingAt(ViewportFraction);
		if (Group.Contains(Under))
		{
			return;
		}
		AHOLBuilding* RallySite = Under && HOLPlayerState && Under->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex()
			&& (Under->NeedsBuilders() || Under->IsCamp()) ? Under : nullptr;
		const FVector Point = RallySite ? RallySite->GetActorLocation() : Source ? Source->Location : Ground;
		const int32 RallySource = !RallySite && Source ? SourceIndex : INDEX_NONE;
		int32 Rallied = 0;
		for (AHOLBuilding* Member : Group)
		{
			const FHOLBuildingTypeRow* Type = Member->GetType();
			// Finished or not: one that will produce takes its rally point from its ghost on (the user's request of 2026-10-19).
			if (Type && !Type->Produces.IsEmpty())
			{
				ServerSetRallyPoint(Member, Point, RallySource, RallySite);
				++Rallied;
			}
		}
		if (Rallied > 0)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order: rally point of %d buildings of the group at (%.0f, %.0f)"), Rallied, Point.X, Point.Y);
			OnOrderGiven.Broadcast(Point);
		}
	}
}

void AHOLPlayerController::SetSelection(AHOLBuilding* Building)
{
	if (Selection.Get() == Building && SelectedUnits.IsEmpty() && SelectedSource == INDEX_NONE && SelectedBuildings.IsEmpty())
	{
		return;
	}
	Selection = Building;
	SelectedBuildings.Reset();
	SelectedUnits.Reset();
	SelectedSource = INDEX_NONE;
	BroadcastSelection();
}

void AHOLPlayerController::SetBuildingSelection(TArray<AHOLBuilding*> Buildings)
{
	Buildings.Remove(nullptr);
	if (Buildings.Num() <= 1)
	{
		SetSelection(Buildings.IsEmpty() ? nullptr : Buildings[0]);
		return;
	}
	const bool bSame = !Selection.IsValid() && SelectedUnits.IsEmpty() && SelectedSource == INDEX_NONE && SelectedBuildings.Num() == Buildings.Num()
		&& !Buildings.ContainsByPredicate([this](const AHOLBuilding* Building) { return !SelectedBuildings.Contains(Building); });
	if (bSame)
	{
		return;
	}
	Selection = nullptr;
	SelectedUnits.Reset();
	SelectedSource = INDEX_NONE;
	SelectedBuildings.Reset();
	for (AHOLBuilding* Building : Buildings)
	{
		SelectedBuildings.Add(Building);
	}
	// A new group starts on « Tous » (the user's 2nd trial of J6a.5).
	ActiveSubgroup = 0;
	BroadcastSelection();
}

void AHOLPlayerController::GetGroupMembers(const TConstArrayView<AHOLBuilding*> Group, TArray<FHOLGroupMember>& OutMembers) const
{
	OutMembers.Reset();
	for (const AHOLBuilding* Building : Group)
	{
		OutMembers.Add({ Building->GetTypeName(), HOLCommandGrid::IsAloneInGroup(Building->GetTypeName()) });
	}
}

void AHOLPlayerController::GetSubgroups(TArray<FHOLSubgroup>& OutSubgroups) const
{
	OutSubgroups.Reset();
	if (!HasBuildingGroup())
	{
		return;
	}
	TArray<AHOLBuilding*> Group;
	GetSelectedBuildings(Group);
	TArray<FHOLGroupMember> Members;
	GetGroupMembers(Group, Members);
	OutSubgroups = HOLCommandGrid::SplitSubgroups(Members);
}

bool AHOLPlayerController::GetActiveSubgroup(FHOLSubgroup& OutSubgroup) const
{
	TArray<FHOLSubgroup> Subgroups;
	GetSubgroups(Subgroups);
	if (Subgroups.IsEmpty())
	{
		return false;
	}
	OutSubgroup = Subgroups[Subgroups.IsValidIndex(ActiveSubgroup) ? ActiveSubgroup : 0];
	return true;
}

void AHOLPlayerController::CycleSubgroup()
{
	TArray<FHOLSubgroup> Subgroups;
	GetSubgroups(Subgroups);
	if (Subgroups.IsEmpty())
	{
		// Units of several types (the user's trial of J7b): « Tous », then each type, by the type kept.
		GetUnitSubgroups(Subgroups);
		if (Subgroups.IsEmpty())
		{
			return;
		}
		const int32 Active = FMath::Max(0, Subgroups.IndexOfByPredicate([this](const FHOLSubgroup& Subgroup) { return !Subgroup.bAll && Subgroup.TypeName == ActiveUnitType; }));
		const int32 Next = (Active + 1) % Subgroups.Num();
		ActiveUnitType = Subgroups[Next].bAll ? NAME_None : Subgroups[Next].TypeName;
		bAttackMoveArmed = false;
		UE_LOG(LogHOLMatch, Display, TEXT("Subgroup: %s (%d of %d)"), *DescribeSubgroup(), Next + 1, Subgroups.Num());
		OnSelectionChanged.Broadcast();
		return;
	}
	ActiveSubgroup = (ActiveSubgroup + 1) % Subgroups.Num();
	UE_LOG(LogHOLMatch, Display, TEXT("Subgroup: %s (%d of %d)"), *DescribeSubgroup(), ActiveSubgroup + 1, Subgroups.Num());
	// The same selection, other cells: the HUD and the frames follow.
	OnSelectionChanged.Broadcast();
}

void AHOLPlayerController::RestoreSubgroup(const uint32 FirstKey)
{
	TArray<FHOLSubgroup> Subgroups;
	GetSubgroups(Subgroups);
	TArray<AHOLBuilding*> Group;
	GetSelectedBuildings(Group);
	const int32 Previous = ActiveSubgroup;
	ActiveSubgroup = 0;
	for (int32 Index = 1; Index < Subgroups.Num(); ++Index)
	{
		if (Subgroups[Index].Members.ContainsByPredicate([&Group, FirstKey](const int32 Member) { return Group[Member]->GetUniqueID() == FirstKey; }))
		{
			ActiveSubgroup = Index;
			break;
		}
	}
	if (ActiveSubgroup != Previous)
	{
		OnSelectionChanged.Broadcast();
	}
}

bool AHOLPlayerController::IsInActiveSubgroup(const AHOLBuilding* Building) const
{
	FHOLSubgroup Subgroup;
	if (!GetActiveSubgroup(Subgroup) || Subgroup.bAll)
	{
		return true;
	}
	TArray<AHOLBuilding*> Group;
	GetSelectedBuildings(Group);
	return Subgroup.Members.ContainsByPredicate([&Group, Building](const int32 Member) { return Group[Member] == Building; });
}

void AHOLPlayerController::GetUnitSubgroups(TArray<FHOLSubgroup>& OutSubgroups, TArray<uint32>* OutUnits) const
{
	OutSubgroups.Reset();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (SelectedUnits.Num() < 2 || !HOLPlayerState || HasBuildingGroup() || Selection.IsValid())
	{
		return;
	}
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	Views.RemoveAll([this, HOLPlayerState](const FHOLUnitView& View)
	{
		return View.OwnerPlayer != HOLPlayerState->GetPlayerIndex() || !SelectedUnits.Contains(View.UnitId);
	});
	// The HUD's order (J7a.5): the soldiers first, the workers (the Peasants) last, each part by name.
	Views.StableSort([](const FHOLUnitView& A, const FHOLUnitView& B)
	{
		const bool bWorkerA = A.CarryCapacity > 0;
		const bool bWorkerB = B.CarryCapacity > 0;
		return bWorkerA != bWorkerB ? !bWorkerA : A.TypeName != B.TypeName && A.TypeName.LexicalLess(B.TypeName);
	});
	if (Views.IsEmpty() || !Views.ContainsByPredicate([&Views](const FHOLUnitView& View) { return View.TypeName != Views[0].TypeName; }))
	{
		return;
	}
	TArray<FHOLGroupMember> Members;
	for (const FHOLUnitView& View : Views)
	{
		Members.Add({ View.TypeName, false });
		if (OutUnits)
		{
			OutUnits->Add(View.UnitId);
		}
	}
	OutSubgroups = HOLCommandGrid::SplitSubgroups(Members);
}

bool AHOLPlayerController::GetActiveUnitSubgroup(FHOLSubgroup& OutSubgroup) const
{
	TArray<FHOLSubgroup> Subgroups;
	GetUnitSubgroups(Subgroups);
	if (Subgroups.IsEmpty())
	{
		return false;
	}
	// The type kept, or « Tous » when none of its units is left.
	const FHOLSubgroup* Active = Subgroups.FindByPredicate([this](const FHOLSubgroup& Subgroup) { return !Subgroup.bAll && Subgroup.TypeName == ActiveUnitType; });
	OutSubgroup = Active ? *Active : Subgroups[0];
	return true;
}

TArray<uint32> AHOLPlayerController::GetCommandUnits() const
{
	TArray<FHOLSubgroup> Subgroups;
	TArray<uint32> Units;
	GetUnitSubgroups(Subgroups, &Units);
	const FHOLSubgroup* Active = Subgroups.FindByPredicate([this](const FHOLSubgroup& Subgroup) { return !Subgroup.bAll && Subgroup.TypeName == ActiveUnitType; });
	if (!Active)
	{
		return GetOwnSelectedUnits();
	}
	TArray<uint32> Result;
	for (const int32 Member : Active->Members)
	{
		Result.Add(Units[Member]);
	}
	return Result;
}

FString AHOLPlayerController::DescribeSubgroup() const
{
	FHOLSubgroup Subgroup;
	if (!GetActiveSubgroup(Subgroup) && !GetActiveUnitSubgroup(Subgroup))
	{
		return FString();
	}
	if (Subgroup.bAll)
	{
		return FString::Printf(TEXT("All (%d)"), Subgroup.Members.Num());
	}
	return HOLCommandGrid::IsAloneInGroup(Subgroup.TypeName) ? Subgroup.TypeName.ToString()
		: FString::Printf(TEXT("%s x%d"), *Subgroup.TypeName.ToString(), Subgroup.Members.Num());
}

void AHOLPlayerController::GetCommandBuildings(TArray<AHOLBuilding*>& OutBuildings) const
{
	OutBuildings.Reset();
	if (AHOLBuilding* Own = GetOwnSelection())
	{
		OutBuildings.Add(Own);
		return;
	}
	FHOLSubgroup Subgroup;
	if (!GetActiveSubgroup(Subgroup))
	{
		return;
	}
	TArray<AHOLBuilding*> Group;
	GetSelectedBuildings(Group);
	for (const int32 Member : Subgroup.Members)
	{
		OutBuildings.Add(Group[Member]);
	}
}

void AHOLPlayerController::GetSelectedBuildings(TArray<AHOLBuilding*>& OutBuildings) const
{
	OutBuildings.Reset();
	if (AHOLBuilding* Building = Selection.Get())
	{
		OutBuildings.Add(Building);
		return;
	}
	for (const TWeakObjectPtr<AHOLBuilding>& Building : SelectedBuildings)
	{
		if (Building.IsValid())
		{
			OutBuildings.Add(Building.Get());
		}
	}
}

void AHOLPlayerController::SetUnitSelection(TArray<uint32> UnitIds, const FName ActiveType)
{
	if (UnitIds.IsEmpty())
	{
		SetSelection(nullptr);
		return;
	}
	if (UnitIds.Num() > UE::HeroesOfLiberty::Player::MaxOrderedUnits)
	{
		UnitIds.SetNum(UE::HeroesOfLiberty::Player::MaxOrderedUnits);
	}
	if (!Selection.IsValid() && SelectedSource == INDEX_NONE && SelectedBuildings.IsEmpty() && UnitIds == SelectedUnits)
	{
		return;
	}
	Selection = nullptr;
	SelectedBuildings.Reset();
	SelectedSource = INDEX_NONE;
	SelectedUnits = MoveTemp(UnitIds);
	// A new selection starts on « Tous » (PurgeSelection keeps the type of the units left, set before the broadcast so the
	// grid shows that subgroup's commands: the review of J7b).
	ActiveUnitType = ActiveType;
	BroadcastSelection();
}

void AHOLPlayerController::SetSourceSelection(const int32 SourceIndex)
{
	const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
	if (SourceIndex == INDEX_NONE || !Sources || !Sources->GetSources().IsValidIndex(SourceIndex) || !Sources->GetSources()[SourceIndex].IsStanding())
	{
		SetSelection(nullptr);
		return;
	}
	if (SelectedSource == SourceIndex)
	{
		return;
	}
	Selection = nullptr;
	SelectedBuildings.Reset();
	SelectedUnits.Reset();
	SelectedSource = SourceIndex;
	BroadcastSelection();
}

void AHOLPlayerController::UpdateIdleWorkers(const TConstArrayView<FHOLUnitView> Views)
{
	IdleWorkers.Reset();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (!HOLPlayerState)
	{
		return;
	}
	// A gatherer on its round trip is never idle (D307): only a worker standing with nothing to do.
	for (const FHOLUnitView& View : Views)
	{
		if (View.OwnerPlayer == HOLPlayerState->GetPlayerIndex() && View.CarryCapacity > 0 && View.Activity == EHOLUnitActivity::Idle)
		{
			IdleWorkers.Add(View.UnitId);
		}
	}
	IdleWorkers.Sort();
}

void AHOLPlayerController::SelectNextIdleWorker()
{
	// Read again now: the list may be a quarter of a second old.
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	UpdateIdleWorkers(Views);
	const uint32 UnitId = HOLUnitPicking::NextInCycle(IdleWorkers, LastIdleWorker);
	const FHOLUnitView* Unit = UnitId != 0 ? Views.FindByPredicate([UnitId](const FHOLUnitView& View) { return View.UnitId == UnitId; }) : nullptr;
	if (!Unit)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Next idle worker: none"));
		return;
	}
	LastIdleWorker = UnitId;
	UE_LOG(LogHOLMatch, Display, TEXT("Next idle worker: #%u (%d idle) at (%.0f, %.0f)"), UnitId, IdleWorkers.Num(), Unit->Location.X, Unit->Location.Y);
	SetUnitSelection({ UnitId });
	if (RTSCamera)
	{
		RTSCamera->SetFocus(Unit->Location);
	}
}

void AHOLPlayerController::SelectBuildingCategory(const EHOLBuildingCategory Category)
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (!HOLPlayerState)
	{
		return;
	}
	const FString CategoryName = StaticEnum<EHOLBuildingCategory>()->GetNameStringByValue(static_cast<int64>(Category));
	// The player's buildings of the category seen here: finished, under construction and its ghosts (the user's 2nd
	// trial of J6a.5), in the order of their cycle (the centre, then by type and key): the group's order, its subgroups'.
	TArray<AHOLBuilding*> Buildings;
	TArray<FHOLCycleBuilding> Entries;
	TArray<uint32> Keys;
	for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
	{
		const FHOLBuildingTypeRow* Type = It->GetType();
		if (Type && It->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex() && It->IsSeenHere() && HOLCommandGrid::IsSelectedByCategory(*Type, Category))
		{
			Buildings.Add(*It);
		}
	}
	Buildings.Sort([](const AHOLBuilding& A, const AHOLBuilding& B)
	{
		const int32 RankA = HOLCommandGrid::GetCycleRank(*A.GetType());
		const int32 RankB = HOLCommandGrid::GetCycleRank(*B.GetType());
		return RankA != RankB ? RankA < RankB : A.GetUniqueID() < B.GetUniqueID();
	});
	for (const AHOLBuilding* Building : Buildings)
	{
		Entries.Add({ HOLCommandGrid::GetCycleRank(*Building->GetType()), Building->GetUniqueID() });
		Keys.Add(Building->GetUniqueID());
	}
	if (Buildings.IsEmpty())
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Category %s: no building"), *CategoryName);
		LastCategory.Reset();
		FHOLCommandFeedback Feedback;
		Feedback.Kind = EHOLCommandFeedback::NoBuilding;
		Feedback.Category = Category;
		NotifyFeedback(Feedback);
		return;
	}
	// Pressed again while its selection stays: the selection kept (a building finished or begun since joins it), the
	// camera to the next one.
	TArray<AHOLBuilding*> Selected;
	GetSelectedBuildings(Selected);
	TArray<uint32> SelectedKeys;
	for (const AHOLBuilding* Building : Selected)
	{
		SelectedKeys.Add(Building->GetUniqueID());
	}
	const bool bRepeat = HOLCommandGrid::IsRepeatPress(SelectedKeys, LastCategorySelection, LastCategory.IsSet() && LastCategory.GetValue() == Category);
	if (!bRepeat)
	{
		LastCategoryBuilding = 0;
	}
	LastCategory = Category;
	LastCategorySelection = Keys;
	// Pressed again: the active subgroup kept, even when a building placed since joins the group.
	FHOLSubgroup Active;
	const bool bActive = bRepeat && GetActiveSubgroup(Active);
	const uint32 ActiveKey = bActive && !Active.bAll && Selected.IsValidIndex(Active.Members[0]) ? Selected[Active.Members[0]]->GetUniqueID() : 0;
	SetBuildingSelection(Buildings);
	if (!bRepeat)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Category %s: %d buildings selected"), *CategoryName, Buildings.Num());
		return;
	}
	if (ActiveKey != 0)
	{
		RestoreSubgroup(ActiveKey);
	}
	const int32 Index = HOLCommandGrid::NextInCycle(Entries, LastCategoryBuilding);
	AHOLBuilding* Next = Buildings[Index];
	LastCategoryBuilding = Entries[Index].Key;
	const FVector Location = Next->GetActorLocation();
	UE_LOG(LogHOLMatch, Display, TEXT("Category %s: next building %s at (%.0f, %.0f) (%d selected)"), *CategoryName, *DescribeBuilding(Next),
		Location.X, Location.Y, Buildings.Num());
	if (RTSCamera)
	{
		RTSCamera->SetFocus(Location);
	}
}

void AHOLPlayerController::BroadcastSelection()
{
	UE_LOG(LogHOLMatch, Display, TEXT("Selection: %s"), *DescribeSelection());
	UpdateWatchedUnit();
	OnSelectionChanged.Broadcast();
}

void AHOLPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(AHOLPlayerController, WatchedLoad, COND_OwnerOnly);
}

void AHOLPlayerController::UpdateWatchedUnit()
{
	// Solo and the server simulate: the selection block reads the unit itself.
	if (GetNetMode() != NM_Client)
	{
		return;
	}
	uint32 UnitId = 0;
	FHOLUnitView View;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (SelectedUnits.Num() == 1 && HOLPlayerState && HOLUnitView::Find(*GetWorld(), SelectedUnits[0], View)
		&& View.OwnerPlayer == HOLPlayerState->GetPlayerIndex() && View.CarryCapacity > 0)
	{
		UnitId = View.UnitId;
	}
	if (UnitId != SentWatchedUnitId)
	{
		SentWatchedUnitId = UnitId;
		ServerWatchUnit(UnitId);
	}
}

void AHOLPlayerController::ServerWatchUnit_Implementation(const uint32 UnitId)
{
	WatchedUnitId = UnitId;
	SendWatchedLoad();
}

void AHOLPlayerController::SendWatchedLoad()
{
	FHOLWatchedLoad Load;
	FHOLUnitView View;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	const UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
	// Only the player's own unit: another player's load is never sent to it.
	if (WatchedUnitId != 0 && HOLPlayerState && Units && Units->GetUnitView(WatchedUnitId, View)
		&& View.OwnerPlayer == HOLPlayerState->GetPlayerIndex())
	{
		Load.UnitId = WatchedUnitId;
		Load.Resource = View.LoadResource;
		Load.Amount = static_cast<uint8>(FMath::Clamp(View.LoadAmount, 0, 255));
	}
	if (!(Load == WatchedLoad))
	{
		WatchedLoad = Load;
		ForceNetUpdate();
	}
}

void AHOLPlayerController::OnRep_WatchedLoad()
{
	if (UHOLUnitReceptionSubsystem* Reception = GetWorld()->GetSubsystem<UHOLUnitReceptionSubsystem>())
	{
		Reception->SetWatchedLoad(WatchedLoad.UnitId, WatchedLoad.Resource, WatchedLoad.Amount);
	}
}

void AHOLPlayerController::ClientShowDepositPopups_Implementation(const TArray<FHOLDepositPopup>& Popups)
{
	using namespace UE::HeroesOfLiberty::Player;
	const UWorld* World = GetWorld();
	if (!World || !IsLocalController())
	{
		return;
	}
	const float Now = World->GetTimeSeconds();
	DepositPopups.RemoveAll([Now](const FShownDepositPopup& Shown) { return Now - Shown.BornSeconds >= DepositPopupSeconds; });
	for (const FHOLDepositPopup& Popup : Popups)
	{
		if (Popup.Amount == 0)
		{
			continue;
		}
		if (DepositPopups.Num() >= MaxDepositPopups)
		{
			DepositPopups.RemoveAt(0, EAllowShrinking::No);
		}
		// Over a young one at the same place, a line higher: two Peasants unloading side by side never show one number.
		int32 Stack = 0;
		for (const FShownDepositPopup& Other : DepositPopups)
		{
			if (Now - Other.BornSeconds < DepositPopupStackSeconds
				&& FVector::DistSquared2D(Other.Popup.Location, Popup.Location) < FMath::Square(DepositPopupStackDistance))
			{
				Stack = FMath::Max(Stack, Other.Stack + 1);
			}
		}
		DepositPopups.Add(FShownDepositPopup{ Popup, Now, Stack % MaxDepositPopupStack });
#if !UE_BUILD_SHIPPING
		if (LogDepositPopups != 0)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Deposit popup: +%d %s at (%.0f, %.0f)"), Popup.Amount,
				*StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Popup.Resource)), Popup.Location.X, Popup.Location.Y);
		}
#endif
	}
}

void AHOLPlayerController::Tick(const float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// On a server, for a remote player's controller: its watched unit's load, sent when it changed (D303).
	if (WatchedUnitId != 0 && GetNetMode() < NM_Client && !IsLocalController())
	{
		SendWatchedLoad();
	}
}

void AHOLPlayerController::PurgeSelection(const TConstArrayView<FHOLUnitView> Views)
{
	if (Selection.IsStale())
	{
		// A building gone (a foundation cancelled, J6a.3) leaves the selection.
		Selection.Reset();
		BroadcastSelection();
		return;
	}
	if (SelectedBuildings.ContainsByPredicate([](const TWeakObjectPtr<AHOLBuilding>& Building) { return !Building.IsValid(); }))
	{
		// The same in a group: the others stay, the active subgroup too while one of its buildings does.
		TArray<AHOLBuilding*> Buildings;
		GetSelectedBuildings(Buildings);
		FHOLSubgroup Active;
		uint32 ActiveKey = 0;
		if (GetActiveSubgroup(Active) && !Active.bAll)
		{
			ActiveKey = Buildings[Active.Members[0]]->GetUniqueID();
		}
		SelectedBuildings.Reset();
		SetBuildingSelection(MoveTemp(Buildings));
		if (ActiveKey != 0 && HasBuildingGroup())
		{
			RestoreSubgroup(ActiveKey);
		}
		return;
	}
	if (SelectedSource != INDEX_NONE)
	{
		// A used-up source leaves the map, and the selection.
		const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
		if (!Sources || !Sources->GetSources().IsValidIndex(SelectedSource) || !Sources->GetSources()[SelectedSource].IsStanding())
		{
			SetSelection(nullptr);
		}
		return;
	}
	if (SelectedUnits.IsEmpty())
	{
		return;
	}
	TSet<uint32> Known;
	for (const FHOLUnitView& View : Views)
	{
		Known.Add(View.UnitId);
	}
	TArray<uint32> Units = SelectedUnits;
	if (Units.RemoveAll([&Known](const uint32 Id) { return !Known.Contains(Id); }) > 0)
	{
		// The units' active subgroup stays while units of its type do (GetActiveUnitSubgroup).
		SetUnitSelection(MoveTemp(Units), ActiveUnitType);
	}
}

bool AHOLPlayerController::GetSelectionBox(FVector2D& OutMin, FVector2D& OutMax) const
{
	FVector2D Cursor;
	if (!bSelectPressed || !GetCursorFraction(Cursor) || !HOLUnitPicking::IsBox(ToPixels(SelectPressFraction), ToPixels(Cursor)))
	{
		return false;
	}
	OutMin = FVector2D(FMath::Min(SelectPressFraction.X, Cursor.X), FMath::Min(SelectPressFraction.Y, Cursor.Y));
	OutMax = FVector2D(FMath::Max(SelectPressFraction.X, Cursor.X), FMath::Max(SelectPressFraction.Y, Cursor.Y));
	return true;
}

AHOLPlayerState* AHOLPlayerController::GetHOLPlayerState() const
{
	return GetPlayerState<AHOLPlayerState>();
}

bool AHOLPlayerController::IsSpectator() const
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (HOLPlayerState && HOLPlayerState->IsOnlyASpectator())
	{
		return true;
	}
	const UWorld* World = GetWorld();
	const AHOLGameState* HOLGameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	return HOLPlayerState && HOLGameState && HOLGameState->IsSlotDefeated(HOLPlayerState->GetPlayerIndex());
}

AHOLBuilding* AHOLPlayerController::GetOwnSelection() const
{
	AHOLBuilding* Building = Selection.Get();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	return Building && HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Building->GetOwnerPlayer() ? Building : nullptr;
}

FName AHOLPlayerController::GetGridCellUnitType(const int32 Cell) const
{
	// A building under construction produces nothing (D312), nor a main forum down (V2.3a).
	const AHOLBuilding* Building = GetOwnSelection();
	const FHOLBuildingTypeRow* Type = Building && Building->IsFinished() && !Building->IsForumDown() ? Building->GetType() : nullptr;
	return Type && Type->Produces.IsValidIndex(Cell) ? Type->Produces[Cell] : NAME_None;
}

void AHOLPlayerController::GetGridCommands(TArray<FHOLGridCommand>& OutCommands) const
{
	// D357: a spectator's grid is empty (the HUD hides it).
	if (IsSpectator())
	{
		OutCommands.Init(FHOLGridCommand(), NumGridCells);
		return;
	}
	FHOLGridContext Context;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (const AHOLBuilding* Building = GetOwnSelection())
	{
		const FHOLBuildingTypeRow* Type = Building->GetType();
		Context.bOwnBuilding = Type != nullptr;
		Context.bBuildingFinished = Building->IsFinished();
		Context.bForumDown = Building->IsForumDown();
		if (Type)
		{
			Context.Produces = Type->Produces;
		}
	}
	else if (HasBuildingGroup())
	{
		// A group (an F key, D30), always the player's own: the cells of its active subgroup's buildings, merged (the
		// user's 2nd trial of J6a.5): in « Tous », a key two of them use goes to the main centre, then to the group's order.
		// A subgroup of one building alone is read as that building selected (a site's « Annuler »).
		FHOLSubgroup Subgroup;
		TArray<AHOLBuilding*> Group;
		GetSelectedBuildings(Group);
		if (!GetActiveSubgroup(Subgroup))
		{
			OutCommands.Init(FHOLGridCommand(), NumGridCells);
			return;
		}
		const bool bSeveral = Subgroup.bAll || Subgroup.Members.Num() > 1;
		TArray<FHOLGroupMember> Members;
		GetGroupMembers(Group, Members);
		TArray<TArray<FHOLGridCommand>> PerBuilding;
		PerBuilding.SetNum(Group.Num());
		for (const int32 Member : Subgroup.Members)
		{
			const FHOLBuildingTypeRow* Type = Group[Member]->GetType();
			FHOLGridContext One;
			One.bOwnBuilding = Type != nullptr;
			One.bBuildingGroup = bSeveral;
			One.bBuildingFinished = Group[Member]->IsFinished();
			One.bForumDown = Group[Member]->IsForumDown();
			if (Type)
			{
				One.Produces = Type->Produces;
			}
			for (int32 Cell = 0; Cell < NumGridCells; ++Cell)
			{
				PerBuilding[Member].Add(HOLCommandGrid::GetCommand(One, Cell));
			}
		}
		OutCommands = HOLCommandGrid::MergeCommands(PerBuilding, HOLCommandGrid::GetCommandPriority(Members));
		return;
	}
	else
	{
		GetUnitsContext(Context);
	}
	OutCommands.SetNum(NumGridCells);
	for (int32 Cell = 0; Cell < NumGridCells; ++Cell)
	{
		OutCommands[Cell] = HOLCommandGrid::GetCommand(Context, Cell);
	}
}

void AHOLPlayerController::GetUnitsContext(FHOLGridContext& Context) const
{
	Context.ReachedAge = HOLCommandGrid::GetReachedAge();
	if (GetHOLPlayerState() && !SelectedUnits.IsEmpty())
	{
		// The units of the active subgroup (Tab, the user's trial of J7b; all of them for « Tous »). Builders: those that
		// gather (what the server checks for a build order, J6a.4); soldiers: the others. The units' commands (J7b.3): stop
		// and hold for any own units, attack-move with soldiers among them, which hide the builders' grid (« Tous » of a
		// mixed selection: the Peasants' subgroup has it).
		const TArray<uint32> Units = GetCommandUnits();
		bool bAllKings = true;
		bool bAnyKnown = false;
		for (const uint32 UnitId : Units)
		{
			FHOLUnitView View;
			if (HOLUnitView::Find(*GetWorld(), UnitId, View))
			{
				Context.bBuilders |= View.CarryCapacity > 0;
				Context.bSoldiers |= View.CarryCapacity == 0;
				bAllKings &= View.bKing;
				bAnyKnown = true;
			}
		}
		// V5.3: Kings alone have neither attack-move nor hold (HOLCommandGrid::GetOrder).
		Context.bOnlyKings = bAnyKnown && bAllKings;
		if (Context.bBuilders)
		{
			UHOLBuildingSettings::GetBuildableTypes(Context.Buildable);
		}
		Context.bOwnUnits = !Units.IsEmpty();
	}
}

void AHOLPlayerController::GetOrderCommands(TArray<FHOLGridCommand>& OutCommands) const
{
	if (IsSpectator())
	{
		OutCommands.Init(FHOLGridCommand(), NumGridCells);
		return;
	}
	// A building or a group of buildings: no orders, its grid alone (the user's choice of 2026-10-21).
	FHOLGridContext Context;
	if (!GetOwnSelection() && !HasBuildingGroup())
	{
		GetUnitsContext(Context);
	}
	OutCommands.SetNum(NumGridCells);
	for (int32 Cell = 0; Cell < NumGridCells; ++Cell)
	{
		OutCommands[Cell] = HOLCommandGrid::GetOrder(Context, Cell);
	}
}

void AHOLPlayerController::ActivateOrderCell(const int32 Cell)
{
	TArray<FHOLGridCommand> Commands;
	GetOrderCommands(Commands);
	if (Commands.IsValidIndex(Cell) && !Commands[Cell].IsNone())
	{
		ExecuteGridCommand(Commands[Cell]);
	}
}

void AHOLPlayerController::PressGridKey(const int32 Cell)
{
	if (IsSpectator())
	{
		return;
	}
	// A building or a group: its grid. Units: HOLCommandGrid::PressKey (an age, then its buildings, AoE4's way).
	if (GetOwnSelection() || HasBuildingGroup())
	{
		SetChosenAge(INDEX_NONE);
		ActivateGridCell(Cell);
		return;
	}
	FHOLGridContext Context;
	GetUnitsContext(Context);
	const FHOLGridKeyResult Result = HOLCommandGrid::PressKey(Context, ChosenAge, Cell);
	UE_CLOG(Result.Target == EHOLGridKeyTarget::None && ChosenAge == INDEX_NONE && Cell < HOLCommandGrid::NumAges, LogHOLMatch, Display, TEXT("Grid: age %d not reached"), Cell);
	ApplyAgeKey(Result, Cell);
}

void AHOLPlayerController::ClickAge(const int32 Age, const int32 Cell)
{
	if (IsSpectator())
	{
		return;
	}
	FHOLGridContext Context;
	GetUnitsContext(Context);
	UE_CLOG(HOLCommandGrid::IsBuildGrid(Context) && !HOLCommandGrid::IsAgeReached(Context, Age), LogHOLMatch, Display, TEXT("Grid: age %d not reached"), Age);
	ApplyAgeKey(HOLCommandGrid::ClickAge(Context, ChosenAge, Age, Cell), Cell);
}

void AHOLPlayerController::ApplyAgeKey(const FHOLGridKeyResult& Result, const int32 Cell)
{
	switch (Result.Target)
	{
	case EHOLGridKeyTarget::Grid:
		ActivateGridCell(Cell);
		break;
	case EHOLGridKeyTarget::Orders:
		ActivateOrderCell(Cell);
		break;
	case EHOLGridKeyTarget::Build:
	{
		// The ghost of the chosen age's building (one shown before: replaced), the age kept until it is placed or cancelled.
		FHOLGridCommand Command;
		Command.Command = EHOLGridCommand::Build;
		Command.TypeName = Result.TypeName;
		SetChosenAge(Result.ChosenAge);
		ExecuteGridCommand(Command);
		return;
	}
	default:
		break;
	}
	SetChosenAge(Result.ChosenAge);
}

void AHOLPlayerController::SetChosenAge(const int32 Age)
{
	if (Age != ChosenAge)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Grid: age %s"), Age == INDEX_NONE ? *FString::Printf(TEXT("%d closed"), ChosenAge) : *FString::Printf(TEXT("%d chosen"), Age));
		ChosenAge = Age;
	}
}

void AHOLPlayerController::ActivateGridCell(const int32 Cell)
{
	TArray<FHOLGridCommand> Commands;
	GetGridCommands(Commands);
	if (!Commands.IsValidIndex(Cell) || Commands[Cell].IsNone())
	{
		return;
	}
	ExecuteGridCommand(Commands[Cell]);
}

void AHOLPlayerController::ExecuteGridCommand(const FHOLGridCommand& Command)
{
	if (IsSpectator())
	{
		return;
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	switch (Command.Command)
	{
	case EHOLGridCommand::Produce:
	{
		// What the server would refuse is told and not sent (D282, D313); a full population is told, the unit queued (it waits).
		// A group (D30): spread evenly among the active subgroup's buildings that produce it; every queue full: told so.
		AHOLBuilding* Building = FindProductionBuilding(Command.TypeName);
		const FHOLBuildingTypeRow* Type = Building ? Building->GetType() : nullptr;
		const UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
		const FHOLUnitTypeRow* UnitType = Units ? Units->FindTypeRow(Command.TypeName) : nullptr;
		if (HOLPlayerState && UnitType)
		{
			const int32 MaxQueue = Type ? Type->MaxQueue : 1;
			const FHOLCommandFeedback Feedback = HOLCommandGrid::CheckProduction(HOLPlayerState->GetResources(), UnitType->Cost,
				Building ? Building->GetProductionQueue().Num() : MaxQueue, MaxQueue, HOLPlayerState->GetPopulationUsed(), HOLPlayerState->GetPopulationCap(),
				UnitType->Population);
			if (!Feedback.IsNone())
			{
				NotifyFeedback(Feedback);
			}
			if (HOLCommandGrid::Blocks(Feedback))
			{
				return;
			}
		}
		if (Building)
		{
			UE_CLOG(HasBuildingGroup(), LogHOLMatch, Display, TEXT("Grid: %s queued at %s (#%u), subgroup %s"), *Command.TypeName.ToString(),
				*DescribeBuilding(Building), Building->GetUniqueID(), *DescribeSubgroup());
			LastProductionBuilding.Add(Command.TypeName, Building->GetUniqueID());
			ServerQueueProduction(Building, Command.TypeName);
		}
		return;
	}
	case EHOLGridCommand::Build:
	{
		// Not enough resources: a short message, no ghost (D313).
		const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(Command.TypeName);
		if (HOLPlayerState && Type)
		{
			const FHOLCommandFeedback Feedback = HOLCommandGrid::CheckBuild(HOLPlayerState->GetResources(), Type->Cost);
			if (HOLCommandGrid::Blocks(Feedback))
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Grid: %s not placed"), *Command.TypeName.ToString());
				NotifyFeedback(Feedback);
				return;
			}
		}
		BeginPlacement(Command.TypeName);
		return;
	}
	case EHOLGridCommand::AttackMove:
		// AoE4's way: the key, then a left click on the ground (AttackMoveAt).
		EndPlacement();
		bAttackMoveArmed = true;
		UE_LOG(LogHOLMatch, Display, TEXT("Grid: attack-move, click on the ground"));
		return;
	case EHOLGridCommand::Stop:
	case EHOLGridCommand::Hold:
	{
		// The active subgroup's units (Tab), all of them for « Tous ».
		const TArray<uint32> Own = GetCommandUnits();
		UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units %s"), Own.Num(), Command.Command == EHOLGridCommand::Hold ? TEXT("hold position") : TEXT("stop"));
		bAttackMoveArmed = false;
		RequestStopUnits(Own, Command.Command == EHOLGridCommand::Hold);
		return;
	}
	case EHOLGridCommand::CancelConstruction:
	{
		// The site selected alone, or a subgroup's only building.
		TArray<AHOLBuilding*> Buildings;
		GetCommandBuildings(Buildings);
		if (Buildings.Num() == 1)
		{
			RequestCancelBuilding(Buildings[0]);
		}
		return;
	}
	default:
		return;
	}
}

AHOLBuilding* AHOLPlayerController::FindProductionBuilding(const FName TypeName, bool* bOutAny) const
{
	TArray<AHOLBuilding*> Buildings;
	GetCommandBuildings(Buildings);
	TArray<AHOLBuilding*> Producers;
	TArray<FHOLQueueChoice> Choices;
	for (AHOLBuilding* Building : Buildings)
	{
		// A building under construction produces nothing (D312), nor a main forum down (V2.3a).
		const FHOLBuildingTypeRow* Type = Building->IsFinished() && !Building->IsForumDown() ? Building->GetType() : nullptr;
		if (Type && Type->Produces.Contains(TypeName))
		{
			Producers.Add(Building);
			Choices.Add({ Building->GetProductionQueue().Num(), Type->MaxQueue, Building->GetUniqueID() });
		}
	}
	if (bOutAny)
	{
		*bOutAny = !Producers.IsEmpty();
	}
	const uint32* LastKey = LastProductionBuilding.Find(TypeName);
	const int32 Index = HOLCommandGrid::PickEvenly(Choices, LastKey ? *LastKey : 0);
	return Index == INDEX_NONE ? nullptr : Producers[Index];
}

void AHOLPlayerController::NotifyFeedback(const FHOLCommandFeedback& Feedback)
{
	UE_LOG(LogHOLMatch, Display, TEXT("Command feedback: %s"), *HOLCommandGrid::Describe(Feedback));
	OnCommandFeedback.Broadcast(Feedback);
}

void AHOLPlayerController::RequestProduction(const FName TypeName)
{
	if (AHOLBuilding* Building = GetOwnSelection())
	{
		ServerQueueProduction(Building, TypeName);
	}
}

void AHOLPlayerController::RequestCancelProduction(const int32 Index)
{
	if (IsSpectator())
	{
		return;
	}
	if (AHOLBuilding* Building = GetOwnSelection())
	{
		ServerCancelProduction(Building, Index);
	}
}

void AHOLPlayerController::RequestMoveUnits(const TConstArrayView<uint32> UnitIds, const FVector& Target, const bool bQueue)
{
	if (UnitIds.Num() > 0)
	{
		ServerMoveUnits(TArray<uint32>(UnitIds), Target, bQueue);
	}
}

void AHOLPlayerController::RequestAttackUnit(const TConstArrayView<uint32> UnitIds, const uint32 TargetUnitId, const bool bQueue)
{
	if (UnitIds.Num() > 0 && TargetUnitId != 0)
	{
		ServerAttackUnit(TArray<uint32>(UnitIds), TargetUnitId, bQueue);
	}
}

void AHOLPlayerController::RequestAttackMove(const TConstArrayView<uint32> UnitIds, const FVector& Target, const bool bQueue)
{
	if (UnitIds.Num() > 0)
	{
		ServerAttackMove(TArray<uint32>(UnitIds), Target, bQueue);
	}
}

void AHOLPlayerController::RequestStopUnits(const TConstArrayView<uint32> UnitIds, const bool bHold)
{
	if (UnitIds.Num() > 0)
	{
		ServerStopUnits(TArray<uint32>(UnitIds), bHold);
	}
}

TArray<uint32> AHOLPlayerController::GetOwnSelectedSoldiers() const
{
	TArray<uint32> Soldiers;
	for (const uint32 UnitId : GetOwnSelectedUnits())
	{
		FHOLUnitView View;
		if (HOLUnitView::Find(*GetWorld(), UnitId, View) && View.CarryCapacity == 0)
		{
			Soldiers.Add(UnitId);
		}
	}
	return Soldiers;
}

void AHOLPlayerController::AttackMoveAt(const FVector2D& ViewportFraction, const bool bQueue)
{
	bAttackMoveArmed = false;
	const FVector2D Pixels = ToPixels(ViewportFraction);
	FVector Origin, Direction, Ground;
	// The soldiers of the active subgroup (Tab), all of them for « Tous ».
	TArray<uint32> Soldiers = GetOwnSelectedSoldiers();
	const TArray<uint32> CommandUnits = GetCommandUnits();
	Soldiers.RemoveAll([&CommandUnits](const uint32 UnitId) { return !CommandUnits.Contains(UnitId); });
	if (Soldiers.IsEmpty() || !RTSCamera || !DeprojectScreenPositionToWorld(Pixels.X, Pixels.Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, Ground))
	{
		return;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units attack-move to (%.0f, %.0f)%s"), Soldiers.Num(), Ground.X, Ground.Y, bQueue ? TEXT(", queued") : TEXT(""));
	RequestAttackMove(Soldiers, Ground, bQueue);
	OnAttackOrderGiven.Broadcast(Ground);
}

void AHOLPlayerController::RequestGatherUnits(const TConstArrayView<uint32> UnitIds, const int32 SourceIndex, const bool bQueue)
{
	if (UnitIds.Num() > 0 && SourceIndex != INDEX_NONE)
	{
		ServerGatherUnits(TArray<uint32>(UnitIds), SourceIndex, bQueue);
	}
}

void AHOLPlayerController::RequestRallyPoint(const FVector& Point, const int32 SourceIndex, AHOLBuilding* Site)
{
	if (AHOLBuilding* Building = GetOwnSelection())
	{
		ServerSetRallyPoint(Building, Point, SourceIndex, Site);
	}
}

void AHOLPlayerController::BeginPlacement(const FName TypeName)
{
	if (IsSpectator())
	{
		return;
	}
	const FHOLBuildingTypeRow* Type = TypeName.IsNone() ? nullptr : UHOLBuildingSettings::FindType(TypeName);
	if (!Type || !Type->IsBuildable())
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Placement: %s is not built by builders"), *TypeName.ToString());
		return;
	}
	PlacementType = TypeName;
	bGhostShown = false;
	GhostReason.Reset();
	UE_LOG(LogHOLMatch, Display, TEXT("Placement: %s (cost %s)"), *TypeName.ToString(), *Type->Cost.ToString());
	UpdatePlacementGhost();
}

void AHOLPlayerController::EndPlacement()
{
	if (!IsPlacing())
	{
		return;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Placement: ended"));
	PlacementType = NAME_None;
	bGhostShown = false;
}

bool AHOLPlayerController::GetPlacementGhost(FName& OutType, FBox2D& OutFootprint, double& OutGroundZ, bool& bOutValid) const
{
	if (!IsPlacing() || !bGhostShown)
	{
		return false;
	}
	OutType = PlacementType;
	OutFootprint = GhostFootprint;
	OutGroundZ = GhostGroundZ;
	bOutValid = bGhostValid;
	return true;
}

bool AHOLPlayerController::CheckPlacementHere(const FName TypeName, const FBox2D& Footprint, FString& OutReason, FHOLCommandFeedback* OutFeedback) const
{
	OutReason.Reset();
	if (OutFeedback)
	{
		*OutFeedback = FHOLCommandFeedback();
	}
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (!Type || !HOLPlayerState)
	{
		OutReason = TEXT("unknown");
		return false;
	}
	const EHOLPlacementRefusal Refusal = UHOLConstructionSubsystem::CheckPlacement(*GetWorld(), Footprint, HOLPlayerState->GetPlayerIndex());
	if (Refusal != EHOLPlacementRefusal::None)
	{
		OutReason = UHOLConstructionSubsystem::DescribeRefusal(Refusal);
		if (OutFeedback)
		{
			OutFeedback->Kind = EHOLCommandFeedback::PlacementRefused;
			OutFeedback->Refusal = Refusal;
		}
		return false;
	}
	const FHOLCommandFeedback Cost = HOLCommandGrid::CheckBuild(HOLPlayerState->GetResources(), Type->Cost);
	if (!Cost.IsNone())
	{
		OutReason = TEXT("not enough resources");
		if (OutFeedback)
		{
			*OutFeedback = Cost;
		}
		return false;
	}
	return true;
}

void AHOLPlayerController::UpdatePlacementGhost()
{
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(PlacementType);
	FVector2D Cursor;
	FVector Origin, Direction, Ground;
	const bool bCursor = GetCursorFraction(Cursor);
	const FVector2D Pixels = ToPixels(Cursor);
	if (!Type || !RTSCamera || !bCursor || !DeprojectScreenPositionToWorld(Pixels.X, Pixels.Y, Origin, Direction)
		|| !RTSCamera->GetGroundPoint(Origin, Direction, Ground))
	{
		bGhostShown = false;
		return;
	}
	// Checked again every frame: the stock, the buildings and the sources change under a ghost that stays still.
	FBox2D Footprint = UHOLConstructionSubsystem::SnapFootprint(*Type, Ground);
	FString Reason;
	bool bValid = CheckPlacementHere(PlacementType, Footprint, Reason, &GhostFeedback);
	// On a building or a source, the ghost goes to the nearest free spot beside it, up to 4 tiles away (the user's trial
	// of J7b, feedback 3): a click places it there (the server checks it again). Without the resources, or refused for
	// another reason (out of the map, an enemy centre, the navmesh), it stays red under the cursor.
	FString Moved;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (!bValid && HOLPlayerState && GhostFeedback.Kind == EHOLCommandFeedback::PlacementRefused
		&& (GhostFeedback.Refusal == EHOLPlacementRefusal::Building || GhostFeedback.Refusal == EHOLPlacementRefusal::Source)
		&& HOLCommandGrid::CheckBuild(HOLPlayerState->GetResources(), Type->Cost).IsNone())
	{
		FBox2D Free;
		int32 Ring = 0;
		EHOLPlacementRefusal Obstacle = EHOLPlacementRefusal::None;
		if (UHOLConstructionSubsystem::FindFreeFootprint(*GetWorld(), Footprint, FVector2D(Ground), HOLPlayerState->GetPlayerIndex(), Free, Ring, Obstacle))
		{
			Footprint = Free;
			bValid = true;
			Reason.Reset();
			GhostFeedback = FHOLCommandFeedback();
			Moved = FString::Printf(TEXT(", moved beside the %s (ring %d)"), Obstacle == EHOLPlacementRefusal::Source ? TEXT("source") : TEXT("building"), Ring);
		}
	}
	const bool bChanged = !bGhostShown || bValid != bGhostValid || Reason != GhostReason || Moved != GhostMoved || !(Footprint.Min == GhostFootprint.Min);
	bGhostShown = true;
	bGhostValid = bValid;
	GhostReason = Reason;
	GhostMoved = Moved;
	GhostFootprint = Footprint;
	GhostGroundZ = Ground.Z;
	if (bChanged)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Placement ghost: %s at (%.0f, %.0f) %s%s"), *PlacementType.ToString(), Footprint.GetCenter().X,
			Footprint.GetCenter().Y, bValid ? TEXT("valid") : *FString::Printf(TEXT("refused (%s)"), *Reason), *Moved);
	}
}

void AHOLPlayerController::ClickPlacement(const bool bKeep)
{
	if (!IsPlacing())
	{
		return;
	}
	UpdatePlacementGhost();
	if (!bGhostShown)
	{
		return;
	}
	if (!bGhostValid)
	{
		// A red ghost places nothing, and the player is told why in a short message (D313).
		UE_LOG(LogHOLMatch, Display, TEXT("Placement: not here (%s)"), *GhostReason);
		NotifyFeedback(GhostFeedback);
		return;
	}
	const FVector2D Centre = GhostFootprint.GetCenter();
	UE_LOG(LogHOLMatch, Display, TEXT("Placement: %s at (%.0f, %.0f)%s"), *PlacementType.ToString(), Centre.X, Centre.Y, bKeep ? TEXT(", Shift: next one") : TEXT(""));
	RequestPlaceBuilding(PlacementType, FVector(Centre, GhostGroundZ), bKeep);
	// Placed: the chosen age closed, kept with Shift (the next one, the user's 1st trial of G.1).
	SetChosenAge(HOLCommandGrid::GetChosenAgeAfter(ChosenAge, bKeep ? EHOLAgeEvent::PlacedKept : EHOLAgeEvent::Placed));
	if (!bKeep)
	{
		EndPlacement();
	}
}

void AHOLPlayerController::RequestPlaceBuilding(const FName TypeName, const FVector& Point, const bool bQueue)
{
	if (!TypeName.IsNone())
	{
		// The selected Peasants build it (J6a.4); the server keeps only the player's own that build.
		const TArray<uint32> Builders = GetOwnSelectedUnits();
		if (!Builders.IsEmpty())
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Placement: %d units sent to build it%s"), Builders.Num(), bQueue ? TEXT(", queued") : TEXT(""));
			// The brief mark of an order (J4.6) on the foundation they go to (D313).
			OnOrderGiven.Broadcast(Point);
		}
		ServerPlaceBuilding(TypeName, Point, Builders, bQueue);
	}
}

void AHOLPlayerController::RequestBuildUnits(const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const bool bQueue)
{
	if (UnitIds.Num() > 0 && Building)
	{
		ServerBuildUnits(TArray<uint32>(UnitIds), Building, bQueue);
	}
}

void AHOLPlayerController::ServerBuildUnits_Implementation(const TArray<uint32>& UnitIds, AHOLBuilding* Building, const bool bQueue)
{
	// The order names the player: only its own units, on its own site (D288; checked again by the builders' step); the
	// rule of every slot's orders (IA0.2): the cap, the soldiers walking to the site.
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Build(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, Building, bQueue);
	}
}

void AHOLPlayerController::RequestDropOffUnits(const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const FVector& Point, const bool bQueue)
{
	if (UnitIds.Num() > 0 && Building)
	{
		ServerDropOffUnits(TArray<uint32>(UnitIds), Building, Point, bQueue);
	}
}

void AHOLPlayerController::ServerDropOffUnits_Implementation(const TArray<uint32>& UnitIds, AHOLBuilding* Building, const FVector_NetQuantize Point,
	const bool bQueue)
{
	// The order names the player: only its own units, at its own building (D288, D485; checked again by the gatherers' step).
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::DropOff(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, Building, Point, bQueue);
	}
}

void AHOLPlayerController::RequestCancelBuilding(AHOLBuilding* Building)
{
	if (IsSpectator())
	{
		return;
	}
	if (Building)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Cancel request: %s"), *DescribeBuilding(Building));
		ServerCancelBuilding(Building);
	}
}

void AHOLPlayerController::ServerPlaceBuilding_Implementation(const FName TypeName, const FVector_NetQuantize Point, const TArray<uint32>& UnitIds,
	const bool bQueue)
{
	// The order names the player: it pays and owns the foundation (D288); the server checks everything again.
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Place(*GetWorld(), HOLPlayerState->GetPlayerIndex(), TypeName, Point, UnitIds, bQueue);
	}
}

void AHOLPlayerController::ServerCancelBuilding_Implementation(AHOLBuilding* Building)
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (Building && HOLPlayerState && Commands)
	{
		FHOLConstructionCommand Command;
		Command.Kind = FHOLConstructionCommand::EKind::Cancel;
		Command.PlayerIndex = HOLPlayerState->GetPlayerIndex();
		Command.Building = Building;
		Commands->IssueConstructionOrder(Command);
	}
}

void AHOLPlayerController::ServerExecRPC_Implementation(const FString& Msg)
{
	if (!IsCheatRefused(*FString::Printf(TEXT("exec '%s'"), *Msg)))
	{
		Super::ServerExecRPC_Implementation(Msg);
	}
}

bool AHOLPlayerController::IsCheatRefused(const TCHAR* What) const
{
	const UWorld* World = GetWorld();
	if (World && FHOLCheatRules::AreCheatsAllowed(*World))
	{
		return false;
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat refused: %s from %s (slot %d)"), What, HOLPlayerState ? *HOLPlayerState->GetPlayerName() : TEXT("?"),
		HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE);
	return true;
}

void AHOLPlayerController::RequestCheatResources(const FHOLResources& Amount)
{
	ServerCheatResources(Amount);
}

void AHOLPlayerController::RequestCheatFastBuild(const bool bOn)
{
	ServerCheatFastBuild(bOn);
}

void AHOLPlayerController::RequestCheatPopulation(const int32 Places)
{
	ServerCheatPopulation(Places);
}

void AHOLPlayerController::RequestCheatUnitSpeed(const float Scale)
{
	ServerCheatUnitSpeed(Scale);
}

void AHOLPlayerController::RequestCheatGatherSpeed(const float Scale)
{
	ServerCheatGatherSpeed(Scale);
}

void AHOLPlayerController::ServerCheatResources_Implementation(const FHOLResources Amount)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("resources")))
	{
		return;
	}
	AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (!HOLPlayerState)
	{
		return;
	}
	FHOLResources Stock = HOLPlayerState->GetResources();
	Stock += Amount;
	Stock = FHOLResources(FMath::Max(0, Stock.Food), FMath::Max(0, Stock.Wood), FMath::Max(0, Stock.Stone), FMath::Max(0, Stock.Gold));
	HOLPlayerState->SetResources(Stock);
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat: slot %d resources, now %s"), HOLPlayerState->GetPlayerIndex(), *HOLPlayerState->DescribeEconomy());
#endif
}

void AHOLPlayerController::ServerCheatFastBuild_Implementation(const bool bOn)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("fast build")))
	{
		return;
	}
	// One step for every unit (hol.Prod.BuildTime, on this server).
	if (IConsoleVariable* BuildTime = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Prod.BuildTime")))
	{
		BuildTime->Set(bOn ? 0.01f : 0.f, ECVF_SetByConsole);
	}
	// Every site finished at its first blow, a builder still has to reach it (hol.Build.Instant; the user's choice at the
	// trial of J6a.4, as the aegis of AoE II). hol.Build.Speed stays a knob of its own.
	if (IConsoleVariable* BuildInstant = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Build.Instant")))
	{
		BuildInstant->Set(bOn ? 1 : 0, ECVF_SetByConsole);
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat: slot %d fast build %s"), HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE, bOn ? TEXT("on") : TEXT("off"));
#endif
}

void AHOLPlayerController::ServerCheatPopulation_Implementation(const int32 Places)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("population")))
	{
		return;
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLProductionSubsystem* Production = GetWorld()->GetSubsystem<UHOLProductionSubsystem>();
	if (HOLPlayerState && Production)
	{
		Production->AddCheatPopulation(HOLPlayerState->GetPlayerIndex(), Places);
		UE_LOG(LogHOLMatch, Display, TEXT("Cheat: slot %d population cap +%d (175 at most)"), HOLPlayerState->GetPlayerIndex(), Places);
	}
#endif
}

void AHOLPlayerController::ServerCheatUnitSpeed_Implementation(const float Scale)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("unit speed")))
	{
		return;
	}
	// Every walker of every player (hol.Units.SpeedScale, on this server), at the next step.
	const float Held = FMath::Clamp(FMath::IsFinite(Scale) ? Scale : 1.f, 0.1f, 10.f);
	if (IConsoleVariable* SpeedScale = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Units.SpeedScale")))
	{
		SpeedScale->Set(Held, ECVF_SetByConsole);
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat: slot %d unit speed x%.2f"), HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE, Held);
#endif
}

void AHOLPlayerController::RequestCheatSpawnBuilding(const FName TypeName, const int32 Slot, const FVector& Point)
{
	ServerCheatSpawnBuilding(TypeName, Slot, Point);
}

void AHOLPlayerController::ServerCheatSpawnBuilding_Implementation(const FName TypeName, const int32 Slot, const FVector_NetQuantize Point)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("spawn building")))
	{
		return;
	}
	// As a player's placement (D311: on the tile grid, no rotation), finished at once and free, for any slot, unchecked.
	const FHOLBuildingTypeRow* Type = UHOLBuildingSettings::FindType(TypeName);
	UWorld* World = GetWorld();
	if (!Type || !World || Slot < 0 || Slot > 255)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Cheat: cannot spawn %s for slot %d"), *TypeName.ToString(), Slot);
		return;
	}
	const FBox2D Footprint = UHOLConstructionSubsystem::SnapFootprint(*Type, Point);
	const FVector2D Centre = Footprint.GetCenter();
	const FTransform Transform(FRotator::ZeroRotator, FVector(Centre, UHOLConstructionSubsystem::FindGroundZ(*World, Centre, Point.Z)));
	AHOLBuilding* Building = World->SpawnActorDeferred<AHOLBuilding>(AHOLBuilding::StaticClass(), Transform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Building)
	{
		return;
	}
	Building->Setup(TypeName, static_cast<uint8>(Slot), static_cast<uint8>(Slot));
	Building->FinishSpawning(Transform);
	Building->ForceNetUpdate();
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat: spawned %s of slot %d at (%.0f, %.0f)"), *TypeName.ToString(), Slot, Centre.X, Centre.Y);
#endif
}

void AHOLPlayerController::RequestCheatDefeat(const int32 Slot)
{
	ServerCheatDefeat(Slot);
}

void AHOLPlayerController::ServerCheatDefeat_Implementation(const int32 Slot)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("defeat")))
	{
		return;
	}
	AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
	if (!HOLGameState || !HOLGameState->DefeatSlot(Slot, EHOLDefeatReason::Cheat))
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Cheat: slot %d cannot lose (closed, already defeated or the match over)"), Slot);
	}
#endif
}

bool AHOLPlayerController::FocusCameraOnForum(const int32 Slot)
{
	if (!RTSCamera || !GetWorld())
	{
		return false;
	}
	if (const AHOLBuilding* Forum = AHOLGameState::FindPrimaryForum(*GetWorld(), Slot))
	{
		RTSCamera->SetFocus(Forum->GetActorLocation());
		UE_LOG(LogHOLMatch, Display, TEXT("Camera above the forum of slot %d"), Slot);
		return true;
	}
	if (const AHOLStartSlot* Start = AHOLStartSlot::Find(GetWorld(), Slot))
	{
		RTSCamera->SetFocus(Start->GetMainCentreLocation());
		UE_LOG(LogHOLMatch, Display, TEXT("Camera above the start of slot %d"), Slot);
		return true;
	}
	return false;
}

void AHOLPlayerController::RequestCheatFallForum(const int32 Slot)
{
	ServerCheatFallForum(Slot);
}

void AHOLPlayerController::ServerCheatFallForum_Implementation(const int32 Slot)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("fall forum")))
	{
		return;
	}
	// The same path as the combat's damage: melee (no armour) of exactly its health.
	AHOLBuilding* Forum = AHOLGameState::FindPrimaryForum(*GetWorld(), Slot);
	const int32 Health = Forum ? Forum->GetHealth() : 0;
	if (!Forum || Health <= 0 || UHOLConstructionSubsystem::DamageBuilding(*Forum, Health, EHOLAttackType::Melee) <= 0)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Cheat: the forum of slot %d cannot fall (none, or down already)"), Slot);
	}
#endif
}

void AHOLPlayerController::RequestCheatSkipForum(const int32 Slot)
{
	ServerCheatSkipForum(Slot);
}

void AHOLPlayerController::ServerCheatSkipForum_Implementation(const int32 Slot)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("skip forum")))
	{
		return;
	}
	AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
	if (!HOLGameState || !HOLGameState->SkipForumPhase(Slot))
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Cheat: the forum of slot %d has no phase to skip (standing, or the slot defeated)"), Slot);
	}
#endif
}

void AHOLPlayerController::RequestCheatHoldPresence(const int32 Slot, const bool bOff, const bool bAttackers, const bool bDefenders, const int32 AttackerSlot)
{
	ServerCheatHoldPresence(Slot, bOff, bAttackers, bDefenders, AttackerSlot);
}

void AHOLPlayerController::ServerCheatHoldPresence_Implementation(const int32 Slot, const bool bOff, const bool bAttackers, const bool bDefenders,
	const int32 AttackerSlot)
{
#if !UE_BUILD_SHIPPING
	AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
	if (!HOLGameState)
	{
		return;
	}
	if (bOff)
	{
		HOLGameState->ClearForumPresenceOverride(Slot);
		UE_LOG(LogHOLMatch, Display, TEXT("Victory: presence at the forum of slot %d no longer fixed"), Slot);
		return;
	}
	FHOLForumPresence Presence;
	Presence.bAttackers = bAttackers;
	Presence.bDefenders = bDefenders;
	Presence.AttackerSlot = bAttackers ? AttackerSlot : INDEX_NONE;
	HOLGameState->SetForumPresenceOverride(Slot, Presence);
#endif
}

void AHOLPlayerController::RequestSurrender()
{
	ServerSurrender();
}

void AHOLPlayerController::ServerSurrender_Implementation()
{
	// The player's own slot only (D356); DefeatSlot refuses a slot already defeated and a match over.
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
	const int32 Slot = HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE;
	if (!HOLGameState || Slot == INDEX_NONE || !HOLGameState->DefeatSlot(Slot, EHOLDefeatReason::Surrender))
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Victory: surrender of slot %d refused"), Slot);
	}
}

void AHOLPlayerController::RequestCheatDamageBuilding(const FVector& Point, const int32 Amount, const uint8 AttackType)
{
	ServerCheatDamageBuilding(Point, Amount, AttackType);
}

void AHOLPlayerController::ServerCheatDamageBuilding_Implementation(const FVector_NetQuantize Point, const int32 Amount, const uint8 AttackType)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("damage building")))
	{
		return;
	}
	// The building nearest the point on the ground, any player's (a bare foundation too: its owner's test).
	AHOLBuilding* Nearest = nullptr;
	double NearestDistance = TNumericLimits<double>::Max();
	for (TActorIterator<AHOLBuilding> It(GetWorld()); It; ++It)
	{
		const double Distance = FVector::DistSquared2D(It->GetActorLocation(), Point);
		if (It->GetType() && Distance < NearestDistance)
		{
			Nearest = *It;
			NearestDistance = Distance;
		}
	}
	const EHOLAttackType Type = static_cast<EHOLAttackType>(FMath::Min<uint8>(AttackType, static_cast<uint8>(EHOLAttackType::Hero)));
	if (!Nearest)
	{
		UE_LOG(LogHOLMatch, Warning, TEXT("Cheat: no building to damage"));
		return;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat: %d %s damage on %s of slot %d"), Amount, *UEnum::GetValueAsString(Type),
		*Nearest->GetTypeName().ToString(), Nearest->GetOwnerPlayer());
	UHOLConstructionSubsystem::DamageBuilding(*Nearest, FMath::Max(Amount, 0), Type);
#endif
}

void AHOLPlayerController::ServerCheatGatherSpeed_Implementation(const float Scale)
{
#if !UE_BUILD_SHIPPING
	if (IsCheatRefused(TEXT("gather speed")))
	{
		return;
	}
	// Every gatherer of every player (hol.Gather.SpeedScale, on this server), at the next step.
	const float Held = FMath::Clamp(FMath::IsFinite(Scale) ? Scale : 1.f, 0.1f, 10.f);
	if (IConsoleVariable* SpeedScale = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Gather.SpeedScale")))
	{
		SpeedScale->Set(Held, ECVF_SetByConsole);
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UE_LOG(LogHOLMatch, Display, TEXT("Cheat: slot %d gather speed x%.2f"), HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE, Held);
#endif
}

void AHOLPlayerController::ServerMoveUnits_Implementation(const TArray<uint32>& UnitIds, const FVector_NetQuantize Target, const bool bQueue)
{
	// The order names the player: the simulation moves only its own units (D288), a client could send any identifier; the
	// rule of every slot's orders (IA0.2, HOLPlayerOrders): its slot, the cap, the chains.
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Move(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, Target, bQueue);
	}
}

void AHOLPlayerController::ServerAttackUnit_Implementation(const TArray<uint32>& UnitIds, const uint32 TargetUnitId, const bool bQueue)
{
	// As ServerMoveUnits (D288): the player's own units alone; the order processor checks the target (an enemy unit alive).
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Attack(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, TargetUnitId, bQueue);
	}
}

void AHOLPlayerController::RequestAttackBuilding(const TConstArrayView<uint32> UnitIds, AHOLBuilding* Building, const bool bQueue)
{
	if (UnitIds.Num() > 0 && Building)
	{
		ServerAttackBuilding(TArray<uint32>(UnitIds), Building, bQueue);
	}
}

void AHOLPlayerController::ServerAttackBuilding_Implementation(const TArray<uint32>& UnitIds, AHOLBuilding* Building, const bool bQueue)
{
	// As ServerAttackUnit (D288): the player's own units alone; the order processor checks the building (an enemy's, standing).
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::AttackBuilding(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, Building, bQueue);
	}
}

void AHOLPlayerController::ServerAttackMove_Implementation(const TArray<uint32>& UnitIds, const FVector_NetQuantize Target, const bool bQueue)
{
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::AttackMove(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, Target, bQueue);
	}
}

void AHOLPlayerController::ServerStopUnits_Implementation(const TArray<uint32>& UnitIds, const bool bHold)
{
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Stop(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, bHold);
	}
}

void AHOLPlayerController::ServerMoveUnitsInLine_Implementation(const TArray<uint32>& UnitIds, const FVector_NetQuantize Start,
	const FVector_NetQuantize End, const bool bWidth, const bool bQueue)
{
	// As ServerMoveUnits (D288): the simulation moves only the player's own units. The drag is an arrow (the user's 2nd
	// trial of J7a.4): its base (Start, where the button was pressed) is the target, the middle of the front row; the
	// group faces Start -> End, and with Ctrl its length is the front's width.
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Move(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, Start, bQueue, FVector2D(End - Start), bWidth);
	}
}

void AHOLPlayerController::ServerGatherUnits_Implementation(const TArray<uint32>& UnitIds, const int32 SourceIndex, const bool bQueue)
{
	// The order names the player: the gathering sends only its own units (D288), a client could send any identifier; its
	// soldiers walk to the source (HOLPlayerOrders::SendNonWorkersTo).
	if (const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState())
	{
		HOLPlayerOrders::Gather(*GetWorld(), HOLPlayerState->GetPlayerIndex(), UnitIds, SourceIndex, bQueue);
	}
}

void AHOLPlayerController::ServerSetRallyPoint_Implementation(AHOLBuilding* Building, const FVector_NetQuantize Point, const int32 SourceIndex,
	AHOLBuilding* Site)
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (Building && HOLPlayerState && Commands)
	{
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Rally;
		Command.PlayerIndex = HOLPlayerState->GetPlayerIndex();
		Command.Building = Building;
		Command.Location = Point;
		// A source that is not standing (any more) is the ground: the rally point stays where it was clicked.
		const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
		Command.SourceIndex = Sources && Sources->GetSources().IsValidIndex(SourceIndex) && Sources->GetSources()[SourceIndex].IsStanding()
			? SourceIndex : INDEX_NONE;
		// Its own site not finished, checked by the production at its step (D288).
		Command.RallySite = Site;
		Commands->IssueProductionOrder(Command);
	}
}

void AHOLPlayerController::ServerQueueProduction_Implementation(AHOLBuilding* Building, const FName TypeName)
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (Building && HOLPlayerState && Commands)
	{
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Queue;
		Command.PlayerIndex = HOLPlayerState->GetPlayerIndex();
		Command.Building = Building;
		Command.TypeName = TypeName;
		Commands->IssueProductionOrder(Command);
	}
}

void AHOLPlayerController::ServerCancelProduction_Implementation(AHOLBuilding* Building, const int32 Index)
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (Building && HOLPlayerState && Commands)
	{
		FHOLProductionCommand Command;
		Command.Kind = FHOLProductionCommand::EKind::Cancel;
		Command.PlayerIndex = HOLPlayerState->GetPlayerIndex();
		Command.Building = Building;
		Command.Index = Index;
		Commands->IssueProductionOrder(Command);
	}
}

FString AHOLPlayerController::DescribeBuilding(const AHOLBuilding* Building) const
{
	if (!Building)
	{
		return TEXT("none");
	}
	const AHOLPlayerState* HOLPlayerState = GetPlayerState<AHOLPlayerState>();
	const bool bOwn = HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Building->GetOwnerPlayer();
	return FString::Printf(TEXT("%s of slot %d (%s)%s"), *Building->GetTypeName().ToString(), Building->GetOwnerPlayer(),
		bOwn ? TEXT("own") : TEXT("other"), Building->IsFoundation() ? TEXT(", foundation")
			: !Building->IsFinished() ? TEXT(", under construction") : TEXT(""));
}

FString AHOLPlayerController::DescribeSelection() const
{
	if (SelectedSource != INDEX_NONE)
	{
		const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
		const FHOLSource* Source = Sources && Sources->GetSources().IsValidIndex(SelectedSource) ? &Sources->GetSources()[SelectedSource] : nullptr;
		return Source && Sources->GetTypeNames().IsValidIndex(Source->TypeIndex)
			? FString::Printf(TEXT("source %d %s (%d left)"), SelectedSource, *Sources->GetTypeNames()[Source->TypeIndex].ToString(), Source->Remaining)
			: FString::Printf(TEXT("source %d"), SelectedSource);
	}
	if (!SelectedBuildings.IsEmpty())
	{
		// A group (an F key, D30), always the player's own.
		TArray<AHOLBuilding*> Group;
		GetSelectedBuildings(Group);
		TMap<FName, int32> PerType;
		for (const AHOLBuilding* Building : Group)
		{
			++PerType.FindOrAdd(Building->GetTypeName());
		}
		PerType.KeySort(FNameLexicalLess());
		TArray<FString> Types;
		for (const TPair<FName, int32>& Type : PerType)
		{
			Types.Add(FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
		}
		return FString::Printf(TEXT("%d buildings (own): %s, subgroup %s"), Group.Num(), *FString::Join(Types, TEXT(", ")), *DescribeSubgroup());
	}
	if (SelectedUnits.IsEmpty())
	{
		return DescribeBuilding(Selection.Get());
	}
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	Views.RemoveAll([this](const FHOLUnitView& View) { return !SelectedUnits.Contains(View.UnitId); });
	if (Views.IsEmpty())
	{
		return TEXT("none");
	}
	const TCHAR* const OwnText = HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Views[0].OwnerPlayer ? TEXT("own") : TEXT("other");
	if (Views.Num() == 1)
	{
		return FString::Printf(TEXT("%s #%u of slot %d (%s)"), *Views[0].TypeName.ToString(), Views[0].UnitId, Views[0].OwnerPlayer, OwnText);
	}
	// Several units are of one side (D.1: the player's own, or a box's enemies).
	TMap<FName, int32> PerType;
	for (const FHOLUnitView& View : Views)
	{
		++PerType.FindOrAdd(View.TypeName);
	}
	PerType.KeySort(FNameLexicalLess());
	TArray<FString> Types;
	for (const TPair<FName, int32>& Type : PerType)
	{
		Types.Add(FString::Printf(TEXT("%s x%d"), *Type.Key.ToString(), Type.Value));
	}
	return FString::Printf(TEXT("%d units (%s): %s"), Views.Num(), OwnText, *FString::Join(Types, TEXT(", ")));
}

void AHOLPlayerController::HandleSelectStarted()
{
	// D357: defeated with a mode armed, the mode is left and the click only selects (to watch).
	if (IsSpectator() && (IsPlacing() || bAttackMoveArmed || ChosenAge != INDEX_NONE))
	{
		if (IsPlacing())
		{
			EndPlacement();
		}
		SetChosenAge(INDEX_NONE);
		bAttackMoveArmed = false;
	}
	// In placement mode a left click places (J6a.3): it never selects nor starts a box.
	if (IsPlacing())
	{
		bSelectPressed = false;
		ClickPlacement(bSelectAddHeld);
		return;
	}
	// In attack-move mode a left click gives the order (J7b.3), never selects.
	FVector2D Cursor;
	if (bAttackMoveArmed && GetCursorFraction(Cursor))
	{
		bSelectPressed = false;
		AttackMoveAt(Cursor, bSelectAddHeld);
		return;
	}
	bSelectPressed = GetCursorFraction(SelectPressFraction);
	SelectPressTime = GetWorld()->GetRealTimeSeconds();
}

void AHOLPlayerController::HandleSelectCompleted()
{
	// Resolved at the release: a press that moved past a few pixels drew a box (D288).
	if (!bSelectPressed)
	{
		return;
	}
	bSelectPressed = false;
	FVector2D Release;
	if (GetCursorFraction(Release) && HOLUnitPicking::IsBox(ToPixels(SelectPressFraction), ToPixels(Release)))
	{
		SelectInBox(SelectPressFraction, Release, bSelectAddHeld);
		LastClickTime = -1.;
		return;
	}
	// A double click (presses close in time and place) or Ctrl + click (AoE4): the unit's type on the screen (A.1).
	const FVector2D Pixels = ToPixels(SelectPressFraction);
	if (IsCtrlHeld() || HOLUnitPicking::IsDoubleClick(LastClickTime, LastClickPixels, SelectPressTime, Pixels))
	{
		LastClickTime = -1.;
		SelectSameTypeAt(SelectPressFraction, bSelectAddHeld);
		return;
	}
	LastClickTime = SelectPressTime;
	LastClickPixels = Pixels;
	SelectAt(SelectPressFraction, bSelectAddHeld);
}

void AHOLPlayerController::HandleSelectAdd(const bool bHeld)
{
	bSelectAddHeld = bHeld;
}

void AHOLPlayerController::HandleOrder()
{
	if (IsSpectator())
	{
		bOrderPressed = false;
		if (IsPlacing())
		{
			EndPlacement();
		}
		return;
	}
	// A right click leaves placement mode (D313), the chosen age closed, or the attack-move mode (J7b.3), without ordering anything.
	if (IsPlacing())
	{
		SetChosenAge(HOLCommandGrid::GetChosenAgeAfter(ChosenAge, EHOLAgeEvent::Cancelled));
		EndPlacement();
		return;
	}
	if (bAttackMoveArmed)
	{
		bAttackMoveArmed = false;
		return;
	}
	// Resolved at the release (J7a.4): a click orders at the press, a drag along its line.
	bOrderPressed = GetCursorFraction(OrderPressFraction);
}

void AHOLPlayerController::HandleOrderCompleted()
{
	if (!bOrderPressed)
	{
		return;
	}
	bOrderPressed = false;
	FVector2D Release;
	if (GetCursorFraction(Release) && HOLUnitPicking::IsBox(ToPixels(OrderPressFraction), ToPixels(Release)))
	{
		OrderLineAt(OrderPressFraction, Release);
	}
	else
	{
		OrderAt(OrderPressFraction);
	}
}

bool AHOLPlayerController::GetOrderLine(FVector& OutStart, FVector& OutEnd) const
{
	FVector2D Cursor;
	if (!bOrderPressed || !RTSCamera || IsPlacing() || !GetCursorFraction(Cursor)
		|| !HOLUnitPicking::IsBox(ToPixels(OrderPressFraction), ToPixels(Cursor)) || GetOwnSelectedUnits().Num() < 2)
	{
		return false;
	}
	const FVector2D Ends[2] = { ToPixels(OrderPressFraction), ToPixels(Cursor) };
	FVector* Out[2] = { &OutStart, &OutEnd };
	for (int32 End = 0; End < 2; ++End)
	{
		FVector Origin, Direction;
		if (!DeprojectScreenPositionToWorld(Ends[End].X, Ends[End].Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, *Out[End]))
		{
			return false;
		}
	}
	return true;
}

bool AHOLPlayerController::IsOrderWidthHeld() const
{
	return IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);
}

void AHOLPlayerController::OrderLineAt(const FVector2D& Start, const FVector2D& End, const bool bWidth, const bool bQueue)
{
	if (IsSpectator())
	{
		return;
	}
	// Ctrl (or a script's): the front's width too; Shift (or a script's): after the current move (the user's 1st trial).
	const bool bWithWidth = bWidth || IsOrderWidthHeld();
	const bool bShift = bQueue || bSelectAddHeld;
	const TArray<uint32> Own = GetOwnSelectedUnits();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	const AHOLBuilding* Site = !Own.IsEmpty() ? FindBuildingAt(Start) : nullptr;
	const bool bOnSite = Site && HOLPlayerState && Site->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex() && Site->NeedsBuilders();
	FVector Origin, Direction, GroundStart, GroundEnd;
	const FVector2D StartPixels = ToPixels(Start);
	const FVector2D EndPixels = ToPixels(End);
	if (Own.Num() < 2 || bOnSite || FindSourceAt(Start) != INDEX_NONE || !RTSCamera
		|| !DeprojectScreenPositionToWorld(StartPixels.X, StartPixels.Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, GroundStart)
		|| !DeprojectScreenPositionToWorld(EndPixels.X, EndPixels.Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, GroundEnd))
	{
		OrderAt(Start, bQueue);
		return;
	}
	// The arrow's base is where they go (the user's 2nd trial of J7a.4).
	const FVector Middle = GroundStart;
	UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units in line from (%.0f, %.0f) to (%.0f, %.0f)%s%s"), Own.Num(), GroundStart.X, GroundStart.Y,
		GroundEnd.X, GroundEnd.Y, bWithWidth ? TEXT(", its width") : TEXT(""), bShift ? TEXT(", queued") : TEXT(""));
	if (Own.Num() <= UE::HeroesOfLiberty::Player::MaxOrderedUnits)
	{
		ServerMoveUnitsInLine(Own, GroundStart, GroundEnd, bWithWidth, bShift);
	}
	OnOrderGiven.Broadcast(Middle);
}

void AHOLPlayerController::HandleDeselect()
{
	// D356: the menu open, Escape closes it (its Back, which the menu input mode routes there first); else Escape leaves
	// placement mode first, the selection kept (D313), and closes the chosen age (the user's 1st trial of G.1: an age
	// chosen without a ghost is closed alone), then the attack-move mode (J7b.3); then it deselects; with nothing
	// selected, it opens the in-game menu.
	switch (HOLGameMenuRules::GetEscapeAction(bGameMenuOpen, IsPlacing() || ChosenAge != INDEX_NONE || bAttackMoveArmed, HasSelection()))
	{
	case EHOLEscapeAction::CloseMenu:
		return;
	case EHOLEscapeAction::CancelMode:
		if (IsPlacing())
		{
			SetChosenAge(HOLCommandGrid::GetChosenAgeAfter(ChosenAge, EHOLAgeEvent::Cancelled));
			EndPlacement();
		}
		else if (ChosenAge != INDEX_NONE)
		{
			SetChosenAge(HOLCommandGrid::GetChosenAgeAfter(ChosenAge, EHOLAgeEvent::Escape));
		}
		else
		{
			bAttackMoveArmed = false;
		}
		return;
	case EHOLEscapeAction::Deselect:
		SetSelection(nullptr);
		return;
	case EHOLEscapeAction::OpenMenu:
		OnGameMenuRequested.Broadcast();
		return;
	}
}

bool AHOLPlayerController::HasSelection() const
{
	return Selection.IsValid() || !SelectedUnits.IsEmpty() || SelectedSource != INDEX_NONE || !SelectedBuildings.IsEmpty();
}

void AHOLPlayerController::HandleContinueKey()
{
	// Alt + Enter: the window's full screen (DefaultInput.ini), not a scenario's step.
	if (!IsInputKeyDown(EKeys::LeftAlt) && !IsInputKeyDown(EKeys::RightAlt))
	{
		HOLTestHooks::PressContinueKey();
	}
}

void AHOLPlayerController::HandleNextIdle()
{
	SelectNextIdleWorker();
}

void AHOLPlayerController::HandleCategoryKey(const EHOLBuildingCategory Category)
{
	SelectBuildingCategory(Category);
}

void AHOLPlayerController::HandleHeroKey()
{
	SelectHero();
}

void AHOLPlayerController::HandleSubgroupKey()
{
	CycleSubgroup();
}

void AHOLPlayerController::HandleGridKey(const FInputActionValue& Value, const int32 Cell)
{
	// Ctrl + A (D30): the whole army; Ctrl with another key of the grid does nothing (no production by mistake).
	if (IsCtrlHeld())
	{
		if (Cell == 0)
		{
			SelectArmy();
		}
		return;
	}
	PressGridKey(Cell);
}

void AHOLPlayerController::HandleGroupKey(const FInputActionValue& Value, const int32 Group)
{
	// The modifiers are read from the keys held: on AZERTY, Shift + the key gives the digit, the key's name stays the same.
	const bool bCtrl = IsCtrlHeld();
	const bool bShift = IsShiftHeld();
	if (bCtrl && bShift)
	{
		RemoveFromControlGroup(Group);
	}
	else if (bCtrl)
	{
		// Ctrl + digit alone does nothing (the user's trial of J7a.5: no assign, Shift + digit creates the group).
	}
	else if (bShift)
	{
		AddToControlGroup(Group);
	}
	else
	{
		SelectControlGroup(Group);
	}
}

bool AHOLPlayerController::IsCtrlHeld() const
{
	return IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);
}

bool AHOLPlayerController::IsShiftHeld() const
{
	return IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
}

void AHOLPlayerController::GetOwnSelectionForGroup(TArray<uint32>& OutUnits, TArray<AHOLBuilding*>& OutBuildings) const
{
	OutUnits = GetOwnSelectedUnits();
	OutBuildings.Reset();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	TArray<AHOLBuilding*> Buildings;
	GetSelectedBuildings(Buildings);
	for (AHOLBuilding* Building : Buildings)
	{
		if (HOLPlayerState && Building->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex())
		{
			OutBuildings.Add(Building);
		}
	}
}

bool AHOLPlayerController::GetControlGroup(const int32 Group, TArray<uint32>& OutUnits, TArray<AHOLBuilding*>& OutBuildings) const
{
	OutUnits.Reset();
	OutBuildings.Reset();
	if (Group < 0 || Group >= NumControlGroups)
	{
		return false;
	}
	const FControlGroup& Members = ControlGroups[Group];
	if (!Members.Units.IsEmpty())
	{
		// Dead units (or out of a client's view) are not known here any more: they drop out.
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*GetWorld(), Views);
		TSet<uint32> Known;
		for (const FHOLUnitView& View : Views)
		{
			Known.Add(View.UnitId);
		}
		for (const uint32 UnitId : Members.Units)
		{
			if (Known.Contains(UnitId))
			{
				OutUnits.Add(UnitId);
			}
		}
	}
	for (const TWeakObjectPtr<AHOLBuilding>& Building : Members.Buildings)
	{
		if (Building.IsValid())
		{
			OutBuildings.Add(Building.Get());
		}
	}
	return !OutUnits.IsEmpty() || !OutBuildings.IsEmpty();
}

void AHOLPlayerController::AddToControlGroup(const int32 Group)
{
	if (Group < 0 || Group >= NumControlGroups)
	{
		return;
	}
	TArray<uint32> Units;
	TArray<AHOLBuilding*> Buildings;
	GetOwnSelectionForGroup(Units, Buildings);
	if (Units.IsEmpty() && Buildings.IsEmpty())
	{
		return;
	}
	TArray<uint32> OldUnits;
	TArray<AHOLBuilding*> OldBuildings;
	const bool bCreated = !GetControlGroup(Group, OldUnits, OldBuildings);
	FControlGroup& Members = ControlGroups[Group];
	Members.Units.Reset();
	Members.Buildings.Reset();
	if (!Units.IsEmpty())
	{
		// Units into a group of buildings: it becomes a group of units (one kind at a time).
		Members.Units = MoveTemp(OldUnits);
		for (const uint32 UnitId : Units)
		{
			Members.Units.AddUnique(UnitId);
		}
		if (Members.Units.Num() > UE::HeroesOfLiberty::Player::MaxOrderedUnits)
		{
			Members.Units.SetNum(UE::HeroesOfLiberty::Player::MaxOrderedUnits);
		}
	}
	else
	{
		for (AHOLBuilding* Building : OldBuildings)
		{
			Members.Buildings.Add(Building);
		}
		for (AHOLBuilding* Building : Buildings)
		{
			Members.Buildings.AddUnique(Building);
		}
	}
	LastGroupPressed = INDEX_NONE;
	UE_LOG(LogHOLMatch, Display, TEXT("Group %d: %s, %d units, %d buildings"), Group, bCreated ? TEXT("created") : TEXT("added"), Members.Units.Num(), Members.Buildings.Num());
}

void AHOLPlayerController::RemoveFromControlGroup(const int32 Group)
{
	if (Group < 0 || Group >= NumControlGroups)
	{
		return;
	}
	TArray<uint32> Units;
	TArray<AHOLBuilding*> Buildings;
	GetOwnSelectionForGroup(Units, Buildings);
	FControlGroup& Members = ControlGroups[Group];
	Members.Units.RemoveAll([&Units](const uint32 UnitId) { return Units.Contains(UnitId); });
	Members.Buildings.RemoveAll([&Buildings](const TWeakObjectPtr<AHOLBuilding>& Building) { return !Building.IsValid() || Buildings.Contains(Building.Get()); });
	LastGroupPressed = INDEX_NONE;
	UE_LOG(LogHOLMatch, Display, TEXT("Group %d: removed, %d units, %d buildings"), Group, Members.Units.Num(), Members.Buildings.Num());
}

void AHOLPlayerController::SelectControlGroup(const int32 Group)
{
	TArray<uint32> Units;
	TArray<AHOLBuilding*> Buildings;
	if (!GetControlGroup(Group, Units, Buildings))
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Group %d: empty"), Group);
		LastGroupPressed = INDEX_NONE;
		return;
	}
	const double Now = GetWorld()->GetRealTimeSeconds();
	const bool bDouble = LastGroupPressed == Group && Now - LastGroupPressTime <= ControlGroupDoublePress;
	LastGroupPressed = bDouble ? INDEX_NONE : Group;
	LastGroupPressTime = Now;
	LastSelectedGroup = Group;
	LastGroupSelectTime = Now;

	FVector Centroid = FVector::ZeroVector;
	int32 Count = 0;
	if (!Units.IsEmpty())
	{
		TArray<FHOLUnitView> Views;
		HOLUnitView::Gather(*GetWorld(), Views);
		for (const FHOLUnitView& View : Views)
		{
			if (Units.Contains(View.UnitId))
			{
				Centroid += View.Location;
				++Count;
			}
		}
	}
	else
	{
		for (const AHOLBuilding* Building : Buildings)
		{
			Centroid += Building->GetActorLocation();
			++Count;
		}
	}
	Centroid /= FMath::Max(Count, 1);
	UE_LOG(LogHOLMatch, Display, TEXT("Group %d: selected, %d units, %d buildings%s"), Group, Units.Num(), Buildings.Num(),
		bDouble ? *FString::Printf(TEXT(", camera to (%.0f, %.0f)"), Centroid.X, Centroid.Y) : TEXT(""));
	if (!Units.IsEmpty())
	{
		SetUnitSelection(MoveTemp(Units));
	}
	else
	{
		SetBuildingSelection(MoveTemp(Buildings));
	}
	if (bDouble && RTSCamera && Count > 0)
	{
		RTSCamera->SetFocus(Centroid);
	}
}

void AHOLPlayerController::SelectArmy()
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (!HOLPlayerState)
	{
		return;
	}
	static const FName PeasantType(TEXT("Peasant"));
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	TArray<uint32> UnitIds;
	for (const FHOLUnitView& View : Views)
	{
		if (View.OwnerPlayer == HOLPlayerState->GetPlayerIndex() && View.TypeName != PeasantType)
		{
			UnitIds.Add(View.UnitId);
		}
	}
	UnitIds.Sort();
	UE_LOG(LogHOLMatch, Display, TEXT("Select army: %d soldiers"), UnitIds.Num());
	if (!UnitIds.IsEmpty())
	{
		SetUnitSelection(MoveTemp(UnitIds));
	}
}

void AHOLPlayerController::SelectHero()
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	const UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
	if (!HOLPlayerState || !Units)
	{
		return;
	}
	// The units known here, a dying one left out (J8a.4: a dead hero cannot be selected). One hero a player (J8a-Q2);
	// the oldest if a test made more.
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	const FHOLUnitView* Hero = nullptr;
	for (const FHOLUnitView& View : Views)
	{
		const FHOLUnitTypeRow* Row = View.OwnerPlayer == HOLPlayerState->GetPlayerIndex() ? Units->FindTypeRow(View.TypeName) : nullptr;
		if (Row && Row->HasArmorCategory(EHOLArmorCategory::Hero) && (!Hero || View.UnitId < Hero->UnitId))
		{
			Hero = &View;
		}
	}
	if (!Hero)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Select hero: none"));
		return;
	}
	// Pressed again (he alone selected, whatever selected him): the camera on him, as a control group's double press.
	if (SelectedUnits.Num() == 1 && SelectedUnits[0] == Hero->UnitId)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Select hero: camera on #%u at (%.0f, %.0f)"), Hero->UnitId, Hero->Location.X, Hero->Location.Y);
		if (RTSCamera)
		{
			RTSCamera->SetFocus(Hero->Location);
		}
		return;
	}
	UE_LOG(LogHOLMatch, Display, TEXT("Select hero: selected #%u at (%.0f, %.0f)"), Hero->UnitId, Hero->Location.X, Hero->Location.Y);
	SetUnitSelection({ Hero->UnitId });
}

void AHOLPlayerController::UpdateDrag()
{
	// The ground point grabbed stays under the cursor: the focus moves by what separates them.
	FVector Origin, Direction, Under;
	if (DeprojectMousePositionToWorld(Origin, Direction) && RTSCamera->GetGroundPoint(Origin, Direction, Under))
	{
		RTSCamera->SetFocus(RTSCamera->GetFocus() + FVector(DragAnchor.X - Under.X, DragAnchor.Y - Under.Y, 0.0));
	}
}

void AHOLPlayerController::ServerRequestMatchStats_Implementation(const int32 FromSample, const int32 FromEvent)
{
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	const AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
	const UHOLMatchStatsSubsystem* MatchStats = GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>();
	const bool bOnlyASpectator = HOLPlayerState && HOLPlayerState->IsOnlyASpectator();
	const bool bMatchOver = HOLGameState && HOLGameState->IsMatchOver();
	const FString Name = HOLPlayerState ? HOLPlayerState->GetPlayerName() : FString(TEXT("?"));
	if (!MatchStats || !HOLMatchStats::CanReceiveStats(bOnlyASpectator, bMatchOver))
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Stats: refused to %s (a player during the match, M6-Q11)"), *Name);
		ClientMatchStatsRefused();
		return;
	}
	const FHOLMatchStatsChunk Chunk = HOLMatchStats::MakeChunk(MatchStats->GetStats(), FMath::Max(FromSample, 0), FMath::Max(FromEvent, 0),
		MatchStatsChunkSamples);
	UE_LOG(LogHOLMatch, Display, TEXT("Stats: sent to %s (%s, match %s) from sample %d event %d, %s"), *Name,
		bOnlyASpectator ? TEXT("spectator") : TEXT("player"), bMatchOver ? TEXT("over") : TEXT("on"), Chunk.FromSample, Chunk.FromEvent,
		Chunk.bComplete ? TEXT("complete") : TEXT("more to come"));
	if (Chunk.bComplete)
	{
		// The tables the client's copy now holds (its sample count whole), for the tests to compare with its hol.Stats.Dump.
		for (const FHOLPlayerMatchStats& PlayerStats : MatchStats->GetStats().Players)
		{
			if (PlayerStats.PlayerIndex != INDEX_NONE)
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Stats: sent table %s"), *HOLMatchStats::DescribePlayer(PlayerStats));
			}
		}
	}
	ClientReceiveMatchStats(Chunk);
}

void AHOLPlayerController::ClientReceiveMatchStats_Implementation(const FHOLMatchStatsChunk& Chunk)
{
	if (UHOLMatchStatsSubsystem* MatchStats = GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>())
	{
		MatchStats->ReceiveChunk(Chunk);
	}
}

void AHOLPlayerController::ClientMatchStatsRefused_Implementation()
{
	UE_LOG(LogHOLMatch, Display, TEXT("Stats: the server refused the statistics (a player during the match, M6-Q11)"));
	if (UHOLMatchStatsSubsystem* MatchStats = GetWorld()->GetSubsystem<UHOLMatchStatsSubsystem>())
	{
		MatchStats->ReceiveRefusal();
	}
}
