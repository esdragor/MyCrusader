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
#include "Settings/HOLGameUserSettings.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::Player
{
	// The match HUD (D281) lives in the UI module, which a dedicated server never loads: found by its path on the
	// player's machine, whatever HUD class the server sends.
	const TCHAR* MatchHUDClassPath = TEXT("/Script/HeroesOfLibertyUI.HOLGameHUD");

	// Made by Tools/Editor/make_game_input.py.
	const TCHAR* MappingContextPath = TEXT("/Game/Input/IMC_Game.IMC_Game");
	const TCHAR* ActionFolder = TEXT("/Game/Input/");

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

	/** Scripted checks (D264): a left click at a point of the viewport, without the desktop's mouse. */
	FAutoConsoleCommand ClickCommand(
		TEXT("hol.Select.Click"),
		TEXT("hol.Select.Click <X> <Y>: selects as a left click at that point of the viewport (0 to 1 each way) would"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Usage: hol.Select.Click <X> <Y>"));
				return;
			}
			for (AHOLPlayerController* Controller : GetLocalControllers())
			{
				Controller->SelectAt(FVector2D(FCString::Atod(*Args[0]), FCString::Atod(*Args[1])));
			}
		}));

	FAutoConsoleCommand SelectionReportCommand(
		TEXT("hol.Select.Report"),
		TEXT("hol.Select.Report: logs the local player's selection"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const AHOLPlayerController* Controller : GetLocalControllers())
			{
				UE_LOG(LogHOLMatch, Display, TEXT("Selection report: selected %s"), *Controller->DescribeBuilding(Controller->GetSelection()));
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
		Input->BindAction(Select, ETriggerEvent::Started, this, &AHOLPlayerController::HandleSelect);
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

void AHOLPlayerController::SelectAt(const FVector2D& ViewportFraction)
{
	SetSelection(FindBuildingAt(ViewportFraction));
}

void AHOLPlayerController::SetSelection(AHOLBuilding* Building)
{
	if (Selection.Get() == Building)
	{
		return;
	}
	Selection = Building;
	UE_LOG(LogHOLMatch, Display, TEXT("Selection: %s"), *DescribeBuilding(Building));
	OnSelectionChanged.Broadcast();
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

void AHOLPlayerController::HandleSelect()
{
	FVector2D Cursor;
	if (GetCursorFraction(Cursor))
	{
		SelectAt(Cursor);
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
