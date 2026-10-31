// Copyright Epic Games, Inc. All Rights Reserved.

#include "Map/HOLFogRender.h"

#include "Camera/PlayerCameraManager.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLVisionRules.h"
#include "Map/HOLVisionSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Match/HOLPlayerState.h"
#include "Misc/App.h"
#include "RenderingThread.h"
#include "UnrealClient.h"

namespace
{
	TAutoConsoleVariable<int32> CVarFogRender(
		TEXT("hol.Fog.Render"),
		1,
		TEXT("How the fog is drawn on this machine's screen (F7, F-Q3): 0 not drawn, 1 post process (D562, the default since F8: a match from the menu under a fog must show it)."),
		ECVF_Default);

	TAutoConsoleVariable<float> CVarFogOrderMask(
		TEXT("hol.Fog.OrderMask"),
		0.f,
		TEXT("How far the unfogged halo of the order lines spreads (F7.3): the share of the 3 x 3 pixels around a pixel marked by the stencil, "
			 "times this, clears the fog there (0: the hard 1-pixel mask, as before; 1.5: a pixel of halo, to try against the flicker). Read each frame."),
		ECVF_Default);

	constexpr float RefreshSeconds = 0.1f;

	const TCHAR* PostMaterialPath = TEXT("/Game/Rendering/M_FogPost");

	FAutoConsoleCommandWithWorldAndArgs FogRenderReportCommand(TEXT("hol.Fog.RenderReport"),
		TEXT("hol.Fog.RenderReport: logs how the fog is drawn on this machine (mode, texture, cells)"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* InWorld)
		{
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				const UWorld* World = WorldContext.World();
				if (const UHOLFogRenderSubsystem* Render = World && World->IsGameWorld() ? World->GetSubsystem<UHOLFogRenderSubsystem>() : nullptr)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Render->MakeReport());
				}
			}
		}));
}

bool UHOLFogRenderSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

bool UHOLFogRenderSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Something to show it on: not a dedicated server, not a commandlet, not without rendering.
	return Super::ShouldCreateSubsystem(Outer) && !IsRunningDedicatedServer() && !IsRunningCommandlet() && FApp::CanEverRender();
}

void UHOLFogRenderSubsystem::Deinitialize()
{
	Remove();
	Super::Deinitialize();
}

TStatId UHOLFogRenderSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UHOLFogRenderSubsystem, STATGROUP_Tickables);
}

int32 UHOLFogRenderSubsystem::GetViewerCamp() const
{
	const UHOLVisionSubsystem* Vision = GetWorld() ? GetWorld()->GetSubsystem<UHOLVisionSubsystem>() : nullptr;
	if (!Vision || !Vision->IsStarted() || Vision->GetMode() == EHOLFogMode::Disabled)
	{
		return INDEX_NONE;
	}
	if (Vision->IsClient())
	{
		return Vision->GetClientCamp();
	}
	const APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	const AHOLPlayerState* PlayerState = Controller ? Controller->GetPlayerState<AHOLPlayerState>() : nullptr;
	return PlayerState ? Vision->GetCampIndex(PlayerState->GetPlayerIndex()) : INDEX_NONE;
}

void UHOLFogRenderSubsystem::Tick(const float DeltaTime)
{
	// The picture is kept as long as the viewer has a camp under a fog (the minimap reads it, F8 / F9); only the post process
	// depends on hol.Fog.Render.
	const int32 Camp = GetViewerCamp();
	if (Camp == INDEX_NONE)
	{
		if (bHasPicture)
		{
			bHasPicture = false;
			Cells.Reset();
			NumVisible = 0;
			NumExplored = 0;
			++Version;
			OnPictureChanged.Broadcast();
		}
		Remove();
		return;
	}
	Seconds += DeltaTime;
	if (!Texture || Seconds >= RefreshSeconds)
	{
		Seconds = 0.f;
		RefreshTexture(Camp);
	}
	if (CVarFogRender.GetValueOnGameThread() > 0 && Texture)
	{
		if (AppliedMode != 1)
		{
			Apply();
		}
		SendCamera();
	}
	else if (AppliedMode != 0)
	{
		Remove();
	}
}

EHOLCellVision UHOLFogRenderSubsystem::GetCellVision(const FVector2D& Point) const
{
	return bHasPicture ? HOLFogPicture::Read(Grid, Cells, Point) : EHOLCellVision::Visible;
}

void UHOLFogRenderSubsystem::RefreshTexture(const int32 Camp)
{
	const UHOLVisionSubsystem* Vision = GetWorld()->GetSubsystem<UHOLVisionSubsystem>();
	const FHOLVisionMap& Map = Vision->GetMap();
	const FHOLVisionGrid& MapGrid = Map.GetGrid();
	if (MapGrid.Side <= 0)
	{
		return;
	}
	TArray<uint8> Now;
	HOLFogPicture::Fill(Map, Camp, Now, NumVisible, NumExplored);
	if (Texture && bHasPicture && Now == Cells)
	{
		return;
	}
	Grid = MapGrid;
	if (!Texture || Texture->GetSizeX() != Grid.Side)
	{
		Texture = UTexture2D::CreateTransient(Grid.Side, Grid.Side, PF_G8);
		Texture->SRGB = false;
		Texture->Filter = TF_Bilinear;
		Texture->AddressX = TA_Clamp;
		Texture->AddressY = TA_Clamp;
		Texture->MipGenSettings = TMGS_NoMipmaps;
		if (Material)
		{
			Material->SetTextureParameterValue(TEXT("FogTexture"), Texture);
		}
	}
	Cells = MoveTemp(Now);
	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, Cells.GetData(), Cells.Num());
	Mip.BulkData.Unlock();
	Texture->UpdateResource();
	bHasPicture = true;
	++Version;
	OnPictureChanged.Broadcast();
}

void UHOLFogRenderSubsystem::Apply()
{
	Remove();
	UMaterialInterface* Parent = LoadObject<UMaterialInterface>(nullptr, PostMaterialPath);
	if (!Parent || !Texture)
	{
		UE_LOG(LogHOLMatch, Error, TEXT("Fog render: material %s missing (Tools/Editor/make_fog_materials.py)"), PostMaterialPath);
		return;
	}
	const double Side = Grid.Side * Grid.CellSize;
	Material = UMaterialInstanceDynamic::Create(Parent, this);
	Material->SetTextureParameterValue(TEXT("FogTexture"), Texture);
	Material->SetVectorParameterValue(TEXT("FogRect"), FLinearColor(-0.5 * Side, -0.5 * Side, Side, 0.));

	Volume = GetWorld()->SpawnActor<APostProcessVolume>();
	Volume->bUnbound = true;
	Volume->BlendWeight = 1.f;
	Volume->Settings.AddBlendable(Material, 1.f);
	AppliedMode = 1;
}

void UHOLFogRenderSubsystem::Remove()
{
	if (Volume)
	{
		Volume->Destroy();
		Volume = nullptr;
	}
	Material = nullptr;
	AppliedMode = 0;
}

void UHOLFogRenderSubsystem::SendCamera()
{
	const APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	if (!Material || !Controller || !Controller->PlayerCameraManager || !GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport)
	{
		return;
	}
	FVector Location;
	FRotator Rotation;
	Controller->GetPlayerViewPoint(Location, Rotation);
	const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
	const double TanX = FMath::Tan(FMath::DegreesToRadians(Controller->PlayerCameraManager->GetFOVAngle()) * 0.5);
	const double TanY = Size.X > 0 ? TanX * Size.Y / Size.X : TanX;
	const FQuat Quat = Rotation.Quaternion();
	const auto Send = [this](const TCHAR* Name, const FVector& Vector) { Material->SetVectorParameterValue(Name, FLinearColor(Vector.X, Vector.Y, Vector.Z, 0.)); };
	Send(TEXT("CamPos"), Location);
	Send(TEXT("CamFwd"), Quat.GetForwardVector());
	Send(TEXT("CamRight"), Quat.GetRightVector());
	Send(TEXT("CamUp"), Quat.GetUpVector());
	Send(TEXT("CamTan"), FVector(TanX, TanY, 0.));
	Material->SetScalarParameterValue(TEXT("MaskGain"), CVarFogOrderMask.GetValueOnGameThread());
}

FString UHOLFogRenderSubsystem::MakeReport() const
{
	const TCHAR* Names[] = { TEXT("not drawn"), TEXT("post process") };
	return FString::Printf(TEXT("Fog render: mode %d (%s), texture %d x %d, cells in sight %d explored %d, picture %s version %u"), AppliedMode,
		Names[FMath::Clamp(AppliedMode, 0, 1)], Texture ? Texture->GetSizeX() : 0, Texture ? Texture->GetSizeY() : 0, NumVisible, NumExplored,
		bHasPicture ? TEXT("kept") : TEXT("none"), Version);
}
