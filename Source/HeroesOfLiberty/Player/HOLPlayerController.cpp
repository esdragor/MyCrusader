// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLPlayerController.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/HUD.h"
#include "Engine/GameViewportClient.h"
#include "Engine/LocalPlayer.h"
#include "EngineUtils.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HAL/IConsoleManager.h"
#include "HOLCollision.h"
#include "HeroesOfLibertyLog.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Map/HOLMapBounds.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Map/HOLStartSlot.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLRTSCamera.h"
#include "Player/HOLSelectionMarkers.h"
#include "Player/HOLUnitPicking.h"
#include "Resources/HOLSourceField.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Settings/HOLGameUserSettings.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

namespace UE::HeroesOfLiberty::Player
{
	// The match HUD (D281) lives in the UI module, which a dedicated server never loads: found by its path on the
	// player's machine, whatever HUD class the server sends.
	const TCHAR* MatchHUDClassPath = TEXT("/Script/HeroesOfLibertyUI.HOLGameHUD");

	// Made by Tools/Editor/make_game_input.py.
	const TCHAR* MappingContextPath = TEXT("/Game/Input/IMC_Game.IMC_Game");
	const TCHAR* ActionFolder = TEXT("/Game/Input/");

	/** Units of one move order at most: a player's whole population (D03), a client could send more. */
	constexpr int32 MaxOrderedUnits = 200;

	/** Height (cm) of a unit's body drawn at scale 1 (the mannequin), and its drawn width over its radius: what a click aims at (D288). */
	constexpr double PickBodyHeight = 180.0;
	constexpr double PickWidthFactor = 1.3;
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
		TEXT("hol.Select.Click <X> <Y> [add]: selects as a left click at that point of the viewport (0 to 1 each way) would; 'add' as with Shift"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Click <X> <Y> [add]"));
				return;
			}
			const bool bAdd = Args.Num() >= 3 && Args[2] == TEXT("add");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectAt(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])), bAdd);
			}
		}));

	/** The same on one of the player's own units, wherever it is drawn: a script does not know where units stand on the screen. */
	FAutoConsoleCommand ClickUnitCommand(
		TEXT("hol.Select.ClickUnit"),
		TEXT("hol.Select.ClickUnit <N> [add]: hol.Select.Click at the middle of the drawn body of the player's N-th own unit known here (by identifier; negative from the last)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Wanted = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			const bool bAdd = Args.Num() >= 2 && Args[1] == TEXT("add");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				TArray<FHOLUnitView> Views;
				HOLUnitView::Gather(*Controller->GetWorld(), Views);
				Views.RemoveAll([PlayerState](const FHOLUnitView& View) { return !PlayerState || View.OwnerPlayer != PlayerState->GetPlayerIndex(); });
				Views.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
				// Negative: from the last one (-1), whatever units the steps before made.
				const int32 N = Wanted < 0 ? Views.Num() + Wanted : Wanted;
				int32 SizeX = 0, SizeY = 0;
				Controller->GetViewportSize(SizeX, SizeY);
				FVector2D Middle;
				if (!Views.IsValidIndex(N) || SizeX <= 0 || SizeY <= 0 || !Controller->ProjectWorldLocationToScreen(
					Views[N].Location + FVector(0.0, 0.0, PickBodyHeight * 0.5 * Views[N].MeshScale), Middle))
				{
					UE_LOG(LogHOLMatch, Warning, TEXT("hol.Select.ClickUnit: no own unit %d on the screen"), N);
					continue;
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Click on unit #%u at (%.3f, %.3f)"), Views[N].UnitId, Middle.X / SizeX, Middle.Y / SizeY);
				Controller->SelectAt(FVector2D(Middle.X / SizeX, Middle.Y / SizeY), bAdd);
			}
		}));

	FAutoConsoleCommand BoxCommand(
		TEXT("hol.Select.Box"),
		TEXT("hol.Select.Box <X1> <Y1> <X2> <Y2> [add]: selects as a box drawn between those points of the viewport (0 to 1 each way) would; 'add' as with Shift"),
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
		TEXT("hol.Build.Place <House|LumberCamp|MiningCamp>: placement mode, a ghost under the cursor (left click places, Shift + click places and stays, Escape or right click leave)"),
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
		TEXT("hol.Prod.Queue [UnitType=Peasant]: asks the server to produce one unit at the selected building (own), or at the player's main centre (selected first) when none is"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const FName TypeName(Args.IsEmpty() ? TEXT("Peasant") : *Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
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
	if (!IsLocalController())
	{
		return;
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
	}
	if (const UInputAction* Deselect = LoadAction(TEXT("IA_Deselect")))
	{
		Input->BindAction(Deselect, ETriggerEvent::Started, this, &AHOLPlayerController::HandleDeselect);
	}
	if (const UInputAction* NextIdle = LoadAction(TEXT("IA_NextIdle")))
	{
		Input->BindAction(NextIdle, ETriggerEvent::Started, this, &AHOLPlayerController::HandleNextIdle);
	}
	for (int32 Cell = 0; Cell < NumGridCells; ++Cell)
	{
		if (const UInputAction* GridKey = LoadAction(*FString::Printf(TEXT("IA_Grid_%02d"), Cell)))
		{
			Input->BindAction(GridKey, ETriggerEvent::Started, this, &AHOLPlayerController::HandleGridKey, Cell);
		}
	}
}

void AHOLPlayerController::PlayerTick(const float DeltaTime)
{
	// Input first (the bindings fill KeyPan), then the camera.
	Super::PlayerTick(DeltaTime);
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

void AHOLPlayerController::GatherPickCandidates(const bool bOwnOnly, TArray<FHOLPickCandidate>& OutCandidates, TArray<uint32>& OutUnitIds) const
{
	using namespace UE::HeroesOfLiberty::Player;

	OutCandidates.Reset();
	OutUnitIds.Reset();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (bOwnOnly && !HOLPlayerState)
	{
		return;
	}
	FVector CameraLocation;
	FRotator CameraRotation;
	GetPlayerViewPoint(CameraLocation, CameraRotation);
	const FVector CameraRight = FRotationMatrix(CameraRotation).GetUnitAxis(EAxis::Y);

	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	for (const FHOLUnitView& View : Views)
	{
		if (bOwnOnly && View.OwnerPlayer != HOLPlayerState->GetPlayerIndex())
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
		if (!Type)
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
	// Shift adds to the player's own units only: another player's unit is always selected alone.
	const TArray<uint32> Own = GetOwnSelectedUnits();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
	const FHOLUnitView* Unit = Views.FindByPredicate([UnitId](const FHOLUnitView& View) { return View.UnitId == UnitId; });
	const bool bOwnUnit = Unit && HOLPlayerState && Unit->OwnerPlayer == HOLPlayerState->GetPlayerIndex();
	if (!bAdd || !bOwnUnit || Own.Num() != SelectedUnits.Num())
	{
		SetUnitSelection({ UnitId });
		return;
	}
	TArray<uint32> Units = SelectedUnits;
	if (Units.Remove(UnitId) == 0)
	{
		Units.Add(UnitId);
	}
	SetUnitSelection(MoveTemp(Units));
}

void AHOLPlayerController::SelectInBox(const FVector2D& CornerA, const FVector2D& CornerB, const bool bAdd)
{
	TArray<FHOLPickCandidate> Candidates;
	TArray<uint32> UnitIds;
	GatherPickCandidates(/*bOwnOnly*/ true, Candidates, UnitIds);
	TArray<int32> Indices;
	HOLUnitPicking::PickInBox(Candidates, ToPixels(CornerA), ToPixels(CornerB), Indices);
	TArray<uint32> Boxed;
	for (const int32 Index : Indices)
	{
		Boxed.Add(UnitIds[Index]);
	}
	// Shift adds to the player's own selected units, or removes the boxed ones when all of them already are.
	if (!bAdd || SelectedUnits.IsEmpty() || GetOwnSelectedUnits().Num() != SelectedUnits.Num())
	{
		SetUnitSelection(MoveTemp(Boxed));
		return;
	}
	TArray<uint32> Units = SelectedUnits;
	const bool bAllSelected = !Boxed.IsEmpty() && !Boxed.ContainsByPredicate([&Units](const uint32 Id) { return !Units.Contains(Id); });
	for (const uint32 Id : Boxed)
	{
		if (bAllSelected)
		{
			Units.Remove(Id);
		}
		else
		{
			Units.AddUnique(Id);
		}
	}
	SetUnitSelection(MoveTemp(Units));
}

void AHOLPlayerController::OrderAt(const FVector2D& ViewportFraction, const bool bQueue)
{
	// Shift held, or a script's Shift: a site or a source at the end of the units' queue (D311, the user's trial of J6a.4).
	const bool bShift = bSelectAddHeld || bQueue;
	const FVector2D Pixels = ToPixels(ViewportFraction);
	FVector Origin, Direction, Ground;
	if (!RTSCamera || !DeprojectScreenPositionToWorld(Pixels.X, Pixels.Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, Ground))
	{
		return;
	}
	// A resource source under the cursor (J5, D298): its ground point, where the brief mark shows.
	const int32 SourceIndex = FindSourceAt(ViewportFraction);
	const UHOLSourceSubsystem* Sources = GetWorld()->GetSubsystem<UHOLSourceSubsystem>();
	const FHOLSource* Source = SourceIndex != INDEX_NONE && Sources ? &Sources->GetSources()[SourceIndex] : nullptr;
	const TArray<uint32> Own = GetOwnSelectedUnits();
	// One of the player's sites not finished under the cursor (J6a.4, D312): its selected units go and help build it.
	AHOLBuilding* Site = !Own.IsEmpty() ? FindBuildingAt(ViewportFraction) : nullptr;
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	if (Site && HOLPlayerState && Site->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex() && !Site->IsFinished())
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units build %s%s"), Own.Num(), *DescribeBuilding(Site), bShift ? TEXT(", queued") : TEXT(""));
		RequestBuildUnits(Own, Site, bShift);
		OnOrderGiven.Broadcast(Site->GetActorLocation());
		return;
	}
	if (!Own.IsEmpty())
	{
		if (Source)
		{
			UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units gather source %d (%s) at (%.0f, %.0f)%s"), Own.Num(), SourceIndex,
				*Sources->GetTypeNames()[Source->TypeIndex].ToString(), Source->Location.X, Source->Location.Y, bShift ? TEXT(", queued") : TEXT(""));
			RequestGatherUnits(Own, SourceIndex, bShift);
			OnOrderGiven.Broadcast(Source->Location);
			return;
		}
		UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units to (%.0f, %.0f)"), Own.Num(), Ground.X, Ground.Y);
		RequestMoveUnits(Own, Ground);
		OnOrderGiven.Broadcast(Ground);
	}
	else if (AHOLBuilding* Building = GetOwnSelection())
	{
		// A click on the building itself clears its rally point: the ground seen behind its roof is off its footprint.
		AHOLBuilding* Under = FindBuildingAt(ViewportFraction);
		const bool bOnBuilding = Under == Building;
		// One of the player's sites not finished (the user's trial of J6a.4): the units born here build it; the flag on its
		// middle, as on a source.
		AHOLBuilding* RallySite = Under && !bOnBuilding && HOLPlayerState && Under->GetOwnerPlayer() == HOLPlayerState->GetPlayerIndex()
			&& !Under->IsFinished() ? Under : nullptr;
		const FVector Point = bOnBuilding ? Building->GetActorLocation() : RallySite ? RallySite->GetActorLocation() : Source ? Source->Location : Ground;
		const int32 RallySource = !bOnBuilding && !RallySite && Source ? SourceIndex : INDEX_NONE;
		UE_LOG(LogHOLMatch, Display, TEXT("Order: rally point of %s at (%.0f, %.0f)%s"), *DescribeBuilding(Building), Point.X, Point.Y,
			RallySite ? *FString::Printf(TEXT(" on site %s"), *DescribeBuilding(RallySite))
			: RallySource != INDEX_NONE ? *FString::Printf(TEXT(" on source %d"), RallySource) : TEXT(""));
		RequestRallyPoint(Point, RallySource, RallySite);
		OnOrderGiven.Broadcast(Point);
	}
}

void AHOLPlayerController::SetSelection(AHOLBuilding* Building)
{
	if (Selection.Get() == Building && SelectedUnits.IsEmpty() && SelectedSource == INDEX_NONE)
	{
		return;
	}
	Selection = Building;
	SelectedUnits.Reset();
	SelectedSource = INDEX_NONE;
	BroadcastSelection();
}

void AHOLPlayerController::SetUnitSelection(TArray<uint32> UnitIds)
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
	if (!Selection.IsValid() && SelectedSource == INDEX_NONE && UnitIds == SelectedUnits)
	{
		return;
	}
	Selection = nullptr;
	SelectedSource = INDEX_NONE;
	SelectedUnits = MoveTemp(UnitIds);
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
		SetUnitSelection(MoveTemp(Units));
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

AHOLBuilding* AHOLPlayerController::GetOwnSelection() const
{
	AHOLBuilding* Building = Selection.Get();
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	return Building && HOLPlayerState && HOLPlayerState->GetPlayerIndex() == Building->GetOwnerPlayer() ? Building : nullptr;
}

FName AHOLPlayerController::GetGridCellUnitType(const int32 Cell) const
{
	// A building under construction produces nothing (D312).
	const AHOLBuilding* Building = GetOwnSelection();
	const FHOLBuildingTypeRow* Type = Building && Building->IsFinished() ? Building->GetType() : nullptr;
	return Type && Type->Produces.IsValidIndex(Cell) ? Type->Produces[Cell] : NAME_None;
}

void AHOLPlayerController::ActivateGridCell(const int32 Cell)
{
	const FName TypeName = GetGridCellUnitType(Cell);
	if (!TypeName.IsNone())
	{
		RequestProduction(TypeName);
	}
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
	if (AHOLBuilding* Building = GetOwnSelection())
	{
		ServerCancelProduction(Building, Index);
	}
}

void AHOLPlayerController::RequestMoveUnits(const TConstArrayView<uint32> UnitIds, const FVector& Target)
{
	if (UnitIds.Num() > 0)
	{
		ServerMoveUnits(TArray<uint32>(UnitIds), Target);
	}
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

bool AHOLPlayerController::CheckPlacementHere(const FName TypeName, const FBox2D& Footprint, FString& OutReason) const
{
	OutReason.Reset();
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
		return false;
	}
	if (!HOLPlayerState->GetResources().Covers(Type->Cost))
	{
		OutReason = TEXT("not enough resources");
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
	const FBox2D Footprint = UHOLConstructionSubsystem::SnapFootprint(*Type, Ground);
	FString Reason;
	const bool bValid = CheckPlacementHere(PlacementType, Footprint, Reason);
	const bool bChanged = !bGhostShown || bValid != bGhostValid || Reason != GhostReason || !(Footprint.Min == GhostFootprint.Min);
	bGhostShown = true;
	bGhostValid = bValid;
	GhostReason = Reason;
	GhostFootprint = Footprint;
	GhostGroundZ = Ground.Z;
	if (bChanged)
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Placement ghost: %s at (%.0f, %.0f) %s"), *PlacementType.ToString(), Footprint.GetCenter().X,
			Footprint.GetCenter().Y, bValid ? TEXT("valid") : *FString::Printf(TEXT("refused (%s)"), *Reason));
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
		// A red ghost places nothing (J6a.5 shows why in a short message).
		UE_LOG(LogHOLMatch, Display, TEXT("Placement: not here (%s)"), *GhostReason);
		return;
	}
	const FVector2D Centre = GhostFootprint.GetCenter();
	UE_LOG(LogHOLMatch, Display, TEXT("Placement: %s at (%.0f, %.0f)%s"), *PlacementType.ToString(), Centre.X, Centre.Y, bKeep ? TEXT(", Shift: next one") : TEXT(""));
	RequestPlaceBuilding(PlacementType, FVector(Centre, GhostGroundZ), bKeep);
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
	// The order names the player: only its own units, on its own site (D288; checked again by the builders' step).
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (Building && HOLPlayerState && Commands && UnitIds.Num() <= UE::HeroesOfLiberty::Player::MaxOrderedUnits)
	{
		FHOLBuildOrder Order;
		Order.UnitIds = UnitIds;
		Order.Building = Building;
		Order.PlayerIndex = HOLPlayerState->GetPlayerIndex();
		Order.bQueue = bQueue;
		Commands->IssueBuildOrder(Order);
	}
}

void AHOLPlayerController::RequestCancelBuilding(AHOLBuilding* Building)
{
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
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (HOLPlayerState && Commands)
	{
		FHOLConstructionCommand Command;
		Command.Kind = FHOLConstructionCommand::EKind::Place;
		Command.PlayerIndex = HOLPlayerState->GetPlayerIndex();
		Command.TypeName = TypeName;
		Command.Location = Point;
		if (UnitIds.Num() <= UE::HeroesOfLiberty::Player::MaxOrderedUnits)
		{
			Command.UnitIds = UnitIds;
		}
		Command.bQueue = bQueue;
		Commands->IssueConstructionOrder(Command);
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

void AHOLPlayerController::ServerCheatGatherSpeed_Implementation(const float Scale)
{
#if !UE_BUILD_SHIPPING
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

void AHOLPlayerController::ServerMoveUnits_Implementation(const TArray<uint32>& UnitIds, const FVector_NetQuantize Target)
{
	// The order names the player: the simulation moves only its own units (D288), a client could send any identifier.
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (HOLPlayerState && Commands && UnitIds.Num() <= UE::HeroesOfLiberty::Player::MaxOrderedUnits)
	{
		Commands->IssueMoveOrder(UnitIds, Target, HOLPlayerState->GetPlayerIndex());
	}
}

void AHOLPlayerController::ServerGatherUnits_Implementation(const TArray<uint32>& UnitIds, const int32 SourceIndex, const bool bQueue)
{
	// The order names the player: the gathering sends only its own units (D288), a client could send any identifier.
	const AHOLPlayerState* HOLPlayerState = GetHOLPlayerState();
	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (HOLPlayerState && Commands && UnitIds.Num() <= UE::HeroesOfLiberty::Player::MaxOrderedUnits)
	{
		Commands->IssueGatherOrder(UnitIds, SourceIndex, HOLPlayerState->GetPlayerIndex(), bQueue);
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
	// Several units are always the player's own.
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
	// In placement mode a left click places (J6a.3): it never selects nor starts a box.
	if (IsPlacing())
	{
		bSelectPressed = false;
		ClickPlacement(bSelectAddHeld);
		return;
	}
	bSelectPressed = GetCursorFraction(SelectPressFraction);
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
	}
	else
	{
		SelectAt(SelectPressFraction, bSelectAddHeld);
	}
}

void AHOLPlayerController::HandleSelectAdd(const bool bHeld)
{
	bSelectAddHeld = bHeld;
}

void AHOLPlayerController::HandleOrder()
{
	// A right click leaves placement mode (D313), without ordering anything.
	if (IsPlacing())
	{
		EndPlacement();
		return;
	}
	FVector2D Cursor;
	if (GetCursorFraction(Cursor))
	{
		OrderAt(Cursor);
	}
}

void AHOLPlayerController::HandleDeselect()
{
	// Escape leaves placement mode first, the selection kept (D313).
	if (IsPlacing())
	{
		EndPlacement();
		return;
	}
	SetSelection(nullptr);
}

void AHOLPlayerController::HandleNextIdle()
{
	SelectNextIdleWorker();
}

void AHOLPlayerController::HandleGridKey(const FInputActionValue& Value, const int32 Cell)
{
	ActivateGridCell(Cell);
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
