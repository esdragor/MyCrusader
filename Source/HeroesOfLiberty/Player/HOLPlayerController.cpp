// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLPlayerController.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
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
#include "Map/HOLStartSlot.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLRTSCamera.h"
#include "Player/HOLSelectionMarkers.h"
#include "Player/HOLUnitPicking.h"
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
		TEXT("hol.Select.ClickUnit <N> [add]: hol.Select.Click at the middle of the drawn body of the player's N-th own unit known here (by identifier)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 N = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			const bool bAdd = Args.Num() >= 2 && Args[1] == TEXT("add");
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLPlayerState* PlayerState = Controller->GetHOLPlayerState();
				TArray<FHOLUnitView> Views;
				HOLUnitView::Gather(*Controller->GetWorld(), Views);
				Views.RemoveAll([PlayerState](const FHOLUnitView& View) { return !PlayerState || View.OwnerPlayer != PlayerState->GetPlayerIndex(); });
				Views.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
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
		TEXT("hol.Select.Order <X> <Y>: orders as a right click at that point of the viewport (0 to 1 each way) would"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Order <X> <Y>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->OrderAt(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])));
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
		TEXT("hol.Camera.FocusSlot <Slot>: puts the local player's camera above a start slot (scripted checks)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 Slot = Args.IsEmpty() ? 0 : FCString::Atoi(*Args[0]);
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				const AHOLStartSlot* Start = AHOLStartSlot::Find(Controller->GetWorld(), Slot);
				if (Start && Controller->GetRTSCamera())
				{
					Controller->GetRTSCamera()->SetFocus(Start->GetGroundLocation());
					UE_LOG(LogHOLMatch, Display, TEXT("Camera above the start of slot %d"), Slot);
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

	/** The same, to a point given from the player's own start (a script does not know which slot a client holds). */
	FAutoConsoleCommand UnitsOrderNearCommand(
		TEXT("hol.Units.OrderNear"),
		TEXT("hol.Units.OrderNear Dx Dy [all]: hol.Units.Order to the player's start plus (Dx, Dy) (cm)"),
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
					const FVector Target = Start->GetGroundLocation() + FVector(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
					IConsoleManager::Get().ProcessUserConsoleInput(*FString::Printf(TEXT("hol.Units.Order %.0f %.0f %s"), Target.X, Target.Y,
						Args.Num() >= 3 ? *Args[2] : TEXT("")), *GLog, Controller->GetWorld());
				}
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

	SincePurge += DeltaTime;
	if (SincePurge >= UE::HeroesOfLiberty::Player::SelectionPurgeInterval)
	{
		SincePurge = 0.f;
		PurgeSelectedUnits();
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
	const FVector Ground = Start->GetGroundLocation();
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
			SetSelection(FindBuildingAt(ViewportFraction));
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

void AHOLPlayerController::OrderAt(const FVector2D& ViewportFraction)
{
	const FVector2D Pixels = ToPixels(ViewportFraction);
	FVector Origin, Direction, Ground;
	if (!RTSCamera || !DeprojectScreenPositionToWorld(Pixels.X, Pixels.Y, Origin, Direction) || !RTSCamera->GetGroundPoint(Origin, Direction, Ground))
	{
		return;
	}
	const TArray<uint32> Own = GetOwnSelectedUnits();
	if (!Own.IsEmpty())
	{
		UE_LOG(LogHOLMatch, Display, TEXT("Order: %d units to (%.0f, %.0f)"), Own.Num(), Ground.X, Ground.Y);
		RequestMoveUnits(Own, Ground);
		OnOrderGiven.Broadcast(Ground);
	}
	else if (AHOLBuilding* Building = GetOwnSelection())
	{
		// A click on the building itself clears its rally point: the ground seen behind its roof is off its footprint.
		const FVector Point = FindBuildingAt(ViewportFraction) == Building ? Building->GetActorLocation() : Ground;
		UE_LOG(LogHOLMatch, Display, TEXT("Order: rally point of %s at (%.0f, %.0f)"), *DescribeBuilding(Building), Point.X, Point.Y);
		RequestRallyPoint(Point);
		OnOrderGiven.Broadcast(Point);
	}
}

void AHOLPlayerController::SetSelection(AHOLBuilding* Building)
{
	if (Selection.Get() == Building && SelectedUnits.IsEmpty())
	{
		return;
	}
	Selection = Building;
	SelectedUnits.Reset();
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
	if (!Selection.IsValid() && UnitIds == SelectedUnits)
	{
		return;
	}
	Selection = nullptr;
	SelectedUnits = MoveTemp(UnitIds);
	BroadcastSelection();
}

void AHOLPlayerController::BroadcastSelection()
{
	UE_LOG(LogHOLMatch, Display, TEXT("Selection: %s"), *DescribeSelection());
	OnSelectionChanged.Broadcast();
}

void AHOLPlayerController::PurgeSelectedUnits()
{
	if (SelectedUnits.IsEmpty())
	{
		return;
	}
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*GetWorld(), Views);
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
	const AHOLBuilding* Building = GetOwnSelection();
	const FHOLBuildingTypeRow* Type = Building ? Building->GetType() : nullptr;
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

void AHOLPlayerController::RequestRallyPoint(const FVector& Point)
{
	if (AHOLBuilding* Building = GetOwnSelection())
	{
		ServerSetRallyPoint(Building, Point);
	}
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

void AHOLPlayerController::ServerSetRallyPoint_Implementation(AHOLBuilding* Building, const FVector_NetQuantize Point)
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
	return FString::Printf(TEXT("%s of slot %d (%s)"), *Building->GetTypeName().ToString(), Building->GetOwnerPlayer(),
		bOwn ? TEXT("own") : TEXT("other"));
}

FString AHOLPlayerController::DescribeSelection() const
{
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
	FVector2D Cursor;
	if (GetCursorFraction(Cursor))
	{
		OrderAt(Cursor);
	}
}

void AHOLPlayerController::HandleDeselect()
{
	SetSelection(nullptr);
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
