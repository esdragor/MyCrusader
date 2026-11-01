// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLMinimapView.h"

#include "Buildings/HOLBuilding.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLFogRender.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLVisionSubsystem.h"
#include "Match/HOLPlayerColors.h"
#include "Match/HOLPlayerState.h"
#include "Player/HOLPlayerController.h"
#include "Player/HOLRTSCamera.h"
#include "Rendering/DrawElements.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLUnitView.h"
#include "Styling/CoreStyle.h"
#include "TextureResource.h"
#include "Widgets/SLeafWidget.h"

// The look of the alerts is not decided yet (InterfaceInProgress.md, D386 asks for no shape or duration): everything is a console
// setting, tried with the scenario minimap_alerts_compare.
static TAutoConsoleVariable<float> CVarMinimapAlertSeconds(TEXT("hol.Minimap.AlertSeconds"), 6.f,
	TEXT("How long an alert of the minimap (something of yours hit) blinks, seconds (0.5 to 60)."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMinimapAlertBlink(TEXT("hol.Minimap.AlertBlink"), 0.5f,
	TEXT("Half-period of the minimap alerts' blink, seconds: lit that long, dark that long (0.1 to 10)."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarMinimapAlertShape(TEXT("hol.Minimap.AlertShape"), 0,
	TEXT("Shape of a minimap alert: 0 a red square outline with a core, 1 a filled red disc-like square, 2 two nested outlines, 3 a cross."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMinimapAlertSize(TEXT("hol.Minimap.AlertSize"), 16.f,
	TEXT("Side of a minimap alert, pixels of the picture (6 to 60)."), ECVF_Default);
namespace
{
	/** Pixels a side of the terrain layer: 200 for a map of up to 880 m (4.4 m a pixel), enough for a 240 px picture. */
	constexpr int32 TerrainSide = 200;
	constexpr int32 BuildingIdBit = 0x40000000;

	/**
	 * THE ONE PLACE the minimap reads the fog: what the local viewer's camp knows of a ground point, from the fog render's picture
	 * (UHOLFogRenderSubsystem, FogInProgress.md section 10, the same picture the screen is darkened by). Everything in sight when there is no fog
	 * (mode Disabled, a spectator, the picture not made yet).
	 */
	EHOLCellVision ReadVision(const UHOLFogRenderSubsystem* Fog, const FVector& Point)
	{
		if (!Fog || !Fog->IsFogActive())
		{
			return EHOLCellVision::Visible;
		}
		return Fog->GetCellVision(FVector2D(Point.X, Point.Y));
	}

	FColor ResourceColor(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Food: return FColor(210, 80, 130);
		case EHOLResource::Wood: return FColor(70, 160, 70);
		case EHOLResource::Stone: return FColor(190, 190, 200);
		default: return FColor(245, 205, 40);
		}
	}
}

/** The Slate side: paints what the view holds, takes the mouse. */
class SHOLMinimap : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SHOLMinimap) {}
	SLATE_END_ARGS()

	void Construct(const FArguments&, UHOLMinimapView* InOwner)
	{
		Owner = InOwner;
		SetCanTick(true);
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double CurrentTime, const float DeltaTime) override
	{
		if (UHOLMinimapView* View = Owner.Get())
		{
			View->BroadcastTicked();
		}
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		const float Side = Owner.IsValid() ? Owner->GetSide() : 240.f;
		return FVector2D(Side, Side);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		const UHOLMinimapView* View = Owner.Get();
		if (!View)
		{
			return LayerId;
		}
		const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
		const FVector2f Size = AllottedGeometry.GetLocalSize();
		const FPaintGeometry Whole = AllottedGeometry.ToPaintGeometry();
		OutDrawElements.PushClip(FSlateClippingZone(AllottedGeometry));

		FSlateDrawElement::MakeBox(OutDrawElements, LayerId, Whole, White, ESlateDrawEffect::None, FLinearColor(0.f, 0.f, 0.f, 1.f));
		if (UTexture2D* Terrain = View->GetTerrain())
		{
			FSlateBrush Brush;
			Brush.SetResourceObject(Terrain);
			Brush.ImageSize = FVector2f(TerrainSide, TerrainSide);
			Brush.DrawAs = ESlateBrushDrawType::Image;
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1, Whole, &Brush, ESlateDrawEffect::None, FLinearColor::White);
		}
		int32 Layer = LayerId + 2;
		for (const FHOLMinimapMark& Mark : View->GetMarks())
		{
			const FVector2f Half(Mark.Size * 0.5f, Mark.Size * 0.5f);
			const FVector2f At = FVector2f(Mark.Position) - Half;
			FSlateDrawElement::MakeBox(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(Mark.Size, Mark.Size), FSlateLayoutTransform(At)), White,
				ESlateDrawEffect::None, Mark.Color);
		}
		++Layer;
		// The alerts (shape, size: console settings, see above): red, big enough to see from a distance.
		const int32 Shape = CVarMinimapAlertShape.GetValueOnGameThread();
		const float Ring = FMath::Clamp(CVarMinimapAlertSize.GetValueOnGameThread(), 6.f, 60.f);
		const FLinearColor AlertRed(1.f, 0.1f, 0.1f, 1.f);
		for (const FVector2D& Alert : View->GetAlerts())
		{
			const FVector2f Centre(Alert);
			auto Outline = [&](const float Side, const float Thickness)
			{
				const FVector2f Corner = Centre - FVector2f(Side * 0.5f, Side * 0.5f);
				const FVector2f A = Corner;
				const FVector2f B = Corner + FVector2f(Side, 0.f);
				const FVector2f C = Corner + FVector2f(Side, Side);
				const FVector2f D = Corner + FVector2f(0.f, Side);
				FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, TArray<FVector2f>{ A, B, C, D, A }, ESlateDrawEffect::None, AlertRed, true, Thickness);
			};
			auto Fill = [&](const float Side)
			{
				FSlateDrawElement::MakeBox(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(Side, Side), FSlateLayoutTransform(Centre - FVector2f(Side * 0.5f, Side * 0.5f))),
					White, ESlateDrawEffect::None, AlertRed);
			};
			switch (Shape)
			{
			case 1:
				Fill(Ring);
				break;
			case 2:
				Outline(Ring, 2.5f);
				Outline(Ring * 0.55f, 2.5f);
				break;
			case 3:
				FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, TArray<FVector2f>{ Centre - FVector2f(Ring * 0.5f, 0.f), Centre + FVector2f(Ring * 0.5f, 0.f) }, ESlateDrawEffect::None, AlertRed, true, 3.f);
				FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, TArray<FVector2f>{ Centre - FVector2f(0.f, Ring * 0.5f), Centre + FVector2f(0.f, Ring * 0.5f) }, ESlateDrawEffect::None, AlertRed, true, 3.f);
				break;
			default:
				Outline(Ring, 2.5f);
				Fill(6.f);
				break;
			}
		}		++Layer;
		const TArray<FVector2D>& Frame = View->GetCameraFrame();
		if (Frame.Num() >= 3)
		{
			TArray<FVector2f> Points;
			for (const FVector2D& Point : Frame)
			{
				Points.Add(FVector2f(Point));
			}
			Points.Add(FVector2f(Frame[0]));
			FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, Points, ESlateDrawEffect::None, FLinearColor::White, true, 1.5f);
		}
		++Layer;
		OutDrawElements.PopClip();
		// The frame of the picture.
		const TArray<FVector2f> Edge{ FVector2f(0.f, 0.f), FVector2f(Size.X, 0.f), FVector2f(Size.X, Size.Y), FVector2f(0.f, Size.Y), FVector2f(0.f, 0.f) };
		FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, Edge, ESlateDrawEffect::None, FLinearColor(0.75f, 0.62f, 0.25f, 1.f), true, 2.f);
		return Layer;
	}

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		const bool bLeft = MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton;
		const bool bRight = MouseEvent.GetEffectingButton() == EKeys::RightMouseButton;
		if (!bLeft && !bRight)
		{
			return FReply::Unhandled();
		}
		if (UHOLMinimapView* View = Owner.Get())
		{
			View->BroadcastPressed(FVector2D(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())), bRight, false);
		}
		// The left button is held: dragging goes on moving the camera.
		return bLeft ? FReply::Handled().CaptureMouse(SharedThis(this)) : FReply::Handled();
	}

	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (HasMouseCapture() && MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
		{
			if (UHOLMinimapView* View = Owner.Get())
			{
				View->BroadcastPressed(FVector2D(MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition())), false, true);
			}
			return FReply::Handled();
		}
		return FReply::Unhandled();
	}

	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && HasMouseCapture())
		{
			return FReply::Handled().ReleaseMouseCapture();
		}
		return MouseEvent.GetEffectingButton() == EKeys::RightMouseButton ? FReply::Handled() : FReply::Unhandled();
	}

private:
	TWeakObjectPtr<UHOLMinimapView> Owner;
};

TSharedRef<SWidget> UHOLMinimapView::RebuildWidget()
{
	Slate = SNew(SHOLMinimap, this);
	return Slate.ToSharedRef();
}

void UHOLMinimapView::ReleaseSlateResources(const bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	Slate.Reset();
}

void UHOLMinimapView::SetSide(const float InSide)
{
	Side = FMath::Max(32.f, InSide);
	if (Slate.IsValid())
	{
		Slate->Invalidate(EInvalidateWidgetReason::Layout);
	}
}

void UHOLMinimapView::SetFrame(const FVector2D InMin, const FVector2D InMax)
{
	FrameMin = InMin;
	FrameMax = InMax;
}

void UHOLMinimapView::SetLayers(const TArray<FHOLMinimapMark>& InMarks, const TArray<FVector2D>& InAlerts)
{
	Marks = InMarks;
	Alerts = InAlerts;
	if (Slate.IsValid())
	{
		Slate->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void UHOLMinimapView::SetCameraFrame(const TArray<FVector2D>& InCameraFrame)
{
	CameraFrame = InCameraFrame;
	if (Slate.IsValid())
	{
		Slate->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

FVector2D UHOLMinimapView::GetCameraFocus() const
{
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
	const AHOLRTSCamera* Camera = Controller ? Controller->GetRTSCamera() : nullptr;
	return Camera ? FVector2D(Camera->GetFocus()) : FVector2D::ZeroVector;
}

void UHOLMinimapView::RefreshTerrain(const int32 /*ViewerSlot: the fog render knows the local viewer*/)
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (!Terrain)
	{
		Terrain = UTexture2D::CreateTransient(TerrainSide, TerrainSide, PF_B8G8R8A8);
		if (!Terrain)
		{
			return;
		}
		Terrain->Filter = TF_Bilinear;
		Terrain->AddressX = TA_Clamp;
		Terrain->AddressY = TA_Clamp;
		Terrain->NeverStream = true;
		Terrain->UpdateResource();
	}
	const UHOLFogRenderSubsystem* Fog = World->GetSubsystem<UHOLFogRenderSubsystem>();
	const double Width = FrameMax.Y - FrameMin.Y;
	const double Height = FrameMax.X - FrameMin.X;
	if (Width <= 0.0 || Height <= 0.0)
	{
		return;
	}
	// The same line as HOLMinimap::ToWorld (Script/UI/HOLMinimapRules.as): screen right is world +Y, screen up is world +X.
	auto TexelOf = [&](const double WorldX, const double WorldY, int32& OutX, int32& OutY)
	{
		OutX = FMath::FloorToInt((WorldY - FrameMin.Y) / Width * TerrainSide);
		OutY = FMath::FloorToInt((FrameMax.X - WorldX) / Height * TerrainSide);
		return OutX >= 0 && OutX < TerrainSide && OutY >= 0 && OutY < TerrainSide;
	};

	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(TerrainSide * TerrainSide);
	NumInSight = 0;
	for (int32 Ty = 0; Ty < TerrainSide; ++Ty)
	{
		for (int32 Tx = 0; Tx < TerrainSide; ++Tx)
		{
			const double WorldY = FrameMin.Y + (Tx + 0.5) / TerrainSide * Width;
			const double WorldX = FrameMax.X - (Ty + 0.5) / TerrainSide * Height;
			FColor Color;
			switch (ReadVision(Fog, FVector(WorldX, WorldY, 0.0)))
			{
			case EHOLCellVision::Unexplored: Color = FColor(0, 0, 0, 255); break;
			case EHOLCellVision::Explored: Color = FColor(26, 40, 26, 255); break;
			default: Color = FColor(46, 74, 44, 255); ++NumInSight; break;
			}
			Pixels[Ty * TerrainSide + Tx] = Color;
		}
	}
	// The sources the camp has explored: a dot each (two texels), in their resource's colour.
	NumSourcesDrawn = 0;
	if (const UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>())
	{
		for (const FHOLSource& Source : Sources->GetSources())
		{
			int32 Tx = 0;
			int32 Ty = 0;
			if (!Source.IsStanding() || !TexelOf(Source.Location.X, Source.Location.Y, Tx, Ty)
				|| ReadVision(Fog, Source.Location) == EHOLCellVision::Unexplored)
			{
				continue;
			}
			const FColor Color = ResourceColor(Source.Resource);
			for (int32 Dy = 0; Dy < 2; ++Dy)
			{
				for (int32 Dx = 0; Dx < 2; ++Dx)
				{
					if (Tx + Dx < TerrainSide && Ty + Dy < TerrainSide)
					{
						Pixels[(Ty + Dy) * TerrainSide + Tx + Dx] = Color;
					}
				}
			}
			++NumSourcesDrawn;
		}
	}
	if (FTexturePlatformData* Data = Terrain->GetPlatformData())
	{
		FTexture2DMipMap& Mip = Data->Mips[0];
		void* Dest = Mip.BulkData.Lock(LOCK_READ_WRITE);
		FMemory::Memcpy(Dest, Pixels.GetData(), Pixels.Num() * sizeof(FColor));
		Mip.BulkData.Unlock();
		Terrain->UpdateResource();
	}
	if (Slate.IsValid())
	{
		Slate->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void UHOLMinimapView::GatherEntities(const int32 ViewerSlot, TArray<FHOLMinimapEntity>& OutEntities) const
{
	OutEntities.Reset();
	const UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const UHOLVisionSubsystem* Vision = World->GetSubsystem<UHOLVisionSubsystem>();
	const UHOLFogRenderSubsystem* Fog = World->GetSubsystem<UHOLFogRenderSubsystem>();
	auto ColorOf = [World](const int32 OwnerSlot)
	{
		const int32 ColorIndex = OwnerSlot >= 0 ? AHOLPlayerState::FindColorIndex(*World, OwnerSlot) : INDEX_NONE;
		return ColorIndex >= 0 ? FLinearColor(HOLPlayerColors::GetColor(ColorIndex)) : FLinearColor(0.6f, 0.6f, 0.6f);
	};
	TArray<FHOLUnitView> Views;
	HOLUnitView::Gather(*World, Views);
	for (const FHOLUnitView& View : Views)
	{
		if (Vision && ViewerSlot >= 0 && !Vision->IsUnitShown(ViewerSlot, View.OwnerPlayer, View.Location))
		{
			continue;
		}
		FHOLMinimapEntity& Entity = OutEntities.AddDefaulted_GetRef();
		Entity.Id = static_cast<int32>(View.UnitId & 0x3FFFFFFF);
		Entity.bBuilding = false;
		Entity.Slot = View.OwnerPlayer;
		Entity.bOwn = View.OwnerPlayer == ViewerSlot;
		Entity.Location = FVector2D(View.Location);
		Entity.Color = ColorOf(View.OwnerPlayer);
		Entity.Health = View.Health;
	}
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		const AHOLBuilding* Building = *It;
		const int32 OwnerSlot = Building->GetOwnerPlayer();
		const FVector Place = Building->GetActorLocation();
		if (Building->GetHealth() <= 0 || (OwnerSlot != ViewerSlot && ReadVision(Fog, Place) == EHOLCellVision::Unexplored))
		{
			continue;
		}
		FHOLMinimapEntity& Entity = OutEntities.AddDefaulted_GetRef();
		Entity.Id = BuildingIdBit | static_cast<int32>(Building->GetUniqueID() & 0x3FFFFFFF);
		Entity.bBuilding = true;
		Entity.Slot = OwnerSlot;
		Entity.bOwn = OwnerSlot == ViewerSlot;
		Entity.Location = FVector2D(Place);
		Entity.Color = ColorOf(OwnerSlot);
		Entity.Health = Building->GetHealth();
	}
}

float UHOLMinimapView::GetAlertSeconds() const
{
	return CVarMinimapAlertSeconds.GetValueOnGameThread();
}

float UHOLMinimapView::GetAlertBlink() const
{
	return CVarMinimapAlertBlink.GetValueOnGameThread();
}
bool UHOLMinimapView::GetMapArea(FVector2D& OutMin, FVector2D& OutMax) const
{
	if (const AHOLMapBounds* Bounds = AHOLMapBounds::Find(GetWorld()))
	{
		const FBox2D Area = Bounds->GetArea();
		OutMin = Area.Min;
		OutMax = Area.Max;
		return Area.bIsValid && Area.GetSize().X > 0.0 && Area.GetSize().Y > 0.0;
	}
	return false;
}

bool UHOLMinimapView::GetCameraGround(TArray<FVector2D>& OutCorners) const
{
	OutCorners.Reset();
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
	const AHOLRTSCamera* Camera = Controller ? Controller->GetRTSCamera() : nullptr;
	if (!Camera)
	{
		return false;
	}
	int32 SizeX = 0;
	int32 SizeY = 0;
	Controller->GetViewportSize(SizeX, SizeY);
	if (SizeX <= 0 || SizeY <= 0)
	{
		return false;
	}
	// The screen's middle first: the view the deprojection reads is the last frame's, the camera's focus is this one's; the corners are
	// moved by the difference so that the frame is centred on the focus whatever the camera did since (a click, a jump).
	const FVector2D Screen[5] = { FVector2D(SizeX * 0.5, SizeY * 0.5), FVector2D(0.0, 0.0), FVector2D(SizeX, 0.0), FVector2D(SizeX, SizeY), FVector2D(0.0, SizeY) };
	FVector2D Middle = FVector2D::ZeroVector;
	for (int32 Index = 0; Index < 5; ++Index)
	{
		FVector Origin;
		FVector Direction;
		FVector Ground;
		if (!Controller->DeprojectScreenPositionToWorld(Screen[Index].X, Screen[Index].Y, Origin, Direction) || !Camera->GetGroundPoint(Origin, Direction, Ground))
		{
			OutCorners.Reset();
			return false;
		}
		if (Index == 0)
		{
			Middle = FVector2D(Ground.X, Ground.Y);
		}
		else
		{
			OutCorners.Add(FVector2D(Ground.X, Ground.Y) - Middle + FVector2D(Camera->GetFocus()));
		}
	}
	return true;
}

void UHOLMinimapView::FocusCamera(const FVector2D World)
{
	if (const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer()))
	{
		if (AHOLRTSCamera* Camera = Controller->GetRTSCamera())
		{
			Camera->SetFocus(FVector(World.X, World.Y, Camera->GetFocus().Z));
		}
	}
}

bool UHOLMinimapView::HasOrderableSelection() const
{
	const AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
	return Controller && Controller->GetCommandUnits().Num() > 0;
}

bool UHOLMinimapView::OrderMove(const FVector2D World)
{
	AHOLPlayerController* Controller = Cast<AHOLPlayerController>(GetOwningPlayer());
	if (!Controller)
	{
		return false;
	}
	const TArray<uint32> Units = Controller->GetCommandUnits();
	if (Units.IsEmpty())
	{
		return false;
	}
	const AHOLRTSCamera* Camera = Controller->GetRTSCamera();
	const FVector Ground(World.X, World.Y, Camera ? Camera->GetFocus().Z : 0.0);
	UE_LOG(LogHOLMatch, Display, TEXT("Minimap order: %d units to (%.0f, %.0f)"), Units.Num(), Ground.X, Ground.Y);
	Controller->RequestMoveUnits(Units, Ground, false);
	Controller->OnOrderGiven.Broadcast(Ground);
	return true;
}

FString UHOLMinimapView::Describe() const
{
	return FString::Printf(TEXT("minimap: %.0f px, %d marks, %d alerts lit, camera frame %d points, terrain %d x %d (sources %d, in sight %d cells)"),
		Side, Marks.Num(), Alerts.Num(), CameraFrame.Num(), Terrain ? TerrainSide : 0, Terrain ? TerrainSide : 0, NumSourcesDrawn, NumInSight);
}

void UHOLMinimapView::BroadcastPressed(const FVector2D& Position, const bool bRight, const bool bDrag)
{
	OnPressed.Broadcast(Position, bRight, bDrag);
}
