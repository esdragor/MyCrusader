// Copyright Epic Games, Inc. All Rights Reserved.

#include "HUD/HOLMinimapView.h"

#include "Buildings/HOLBuilding.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HeroesOfLibertyLog.h"
#include "Map/HOLFogRender.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Map/HOLMapBounds.h"
#include "Map/HOLVisionRules.h"
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
static TAutoConsoleVariable<float> CVarMinimapAlertBlink(TEXT("hol.Minimap.AlertBlink"), 10.f,
	TEXT("Half-period of the minimap alerts' blink, seconds: lit that long, dark that long (0.1 to 10). 10 (the default) = no blink within an alert's life: the circle (shape 0) shrinks instead; 0.5 blinks as the square shapes did."), ECVF_Default);
static TAutoConsoleVariable<int32> CVarMinimapAlertShape(TEXT("hol.Minimap.AlertShape"), 0,
	TEXT("Shape of a minimap alert: 0 a red circle (the user's choice, MM-Q1, 2026-11-01) with a core, 1 a red square outline with a core, 2 a filled red square, 3 two nested squares, 4 a cross."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMinimapAlertSize(TEXT("hol.Minimap.AlertSize"), 24.f,
	TEXT("Size of a minimap alert, pixels of the picture (6 to 60): the circle's starting diameter, the square shapes' side."), ECVF_Default);
static TAutoConsoleVariable<float> CVarMinimapAlertShrink(TEXT("hol.Minimap.AlertShrinkSeconds"), 1.f,
	TEXT("The circle alert (shape 0) shrinks from AlertSize to 8 px in that many seconds, then starts again while the alert lasts (0.2 to 10)."), ECVF_Default);
namespace
{
	/** Pixels a side of the terrain layer: 480, twice the 240 px picture (sharp on a screen of dpi 2), the fog's 4 m cells blended between their centres (no block edges). */
	constexpr int32 TerrainSide = 480;
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

	/** The smooth reading of the same picture, 0 (unexplored) to 1 (in sight): the terrain's soft edges. 1 everywhere when there is no fog. */
	float ReadVisionSmooth(const UHOLFogRenderSubsystem* Fog, TConstArrayView<uint8> Blurred, const FVector2D& Point)
	{
		if (!Fog || !Fog->IsFogActive())
		{
			return 1.f;
		}
		return HOLFogPicture::ReadSmooth(Fog->GetGrid(), Blurred, Point);
	}

	/** The terrain's colour for a smooth fog value: black, then the dim green of explored ground (the byte 115), then the clear green of the ground in sight. */
	FLinearColor TerrainColor(const float Vision)
	{
		const FLinearColor Black(0.f, 0.f, 0.f, 1.f);
		const FLinearColor Explored(26.f / 255.f, 40.f / 255.f, 26.f / 255.f, 1.f);
		const FLinearColor Clear(46.f / 255.f, 74.f / 255.f, 44.f / 255.f, 1.f);
		const float ExploredAt = HOLFogPicture::ExploredByte / 255.f;
		return Vision < ExploredAt ? FMath::Lerp(Black, Explored, Vision / ExploredAt) : FMath::Lerp(Explored, Clear, (Vision - ExploredAt) / (1.f - ExploredAt));
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

uint32 HOLMinimapTerrain::GetSourcesSignature(const TConstArrayView<FHOLSource> Sources)
{
	uint32 Signature = 0;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		if (Sources[Index].IsStanding())
		{
			const FVector& Place = Sources[Index].Location;
			Signature = HashCombineFast(Signature, GetTypeHash(Index));
			Signature = HashCombineFast(Signature, GetTypeHash(FIntPoint(FMath::RoundToInt(Place.X), FMath::RoundToInt(Place.Y))));
		}
	}
	return Signature;
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
		// Rounded, anti-aliased marks (a signed distance in the shader, not the hard squares of the white brush): the small ones (the
		// units) are discs, the bigger ones (the buildings) squares with a slight round of the corners.
		const FSlateRoundedBoxBrush Disc(FLinearColor::White);
		const FSlateRoundedBoxBrush Block(FLinearColor::White, FVector4(1.5, 1.5, 1.5, 1.5));
		for (const FHOLMinimapMark& Mark : View->GetMarks())
		{
			const FVector2f Half(Mark.Size * 0.5f, Mark.Size * 0.5f);
			const FVector2f At = FVector2f(Mark.Position) - Half;
			FSlateDrawElement::MakeBox(OutDrawElements, Layer, AllottedGeometry.ToPaintGeometry(FVector2f(Mark.Size, Mark.Size), FSlateLayoutTransform(At)),
				Mark.Size <= 4.f ? &Disc : &Block, ESlateDrawEffect::None, Mark.Color);
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
			// A circle of 24 segments around the alert's centre, Radius px, as a line.
			auto Circle = [&](const float Radius, const float Thickness)
			{
				TArray<FVector2f> Points;
				for (int32 Step = 0; Step <= 24; ++Step)
				{
					const float Angle = Step * UE_TWO_PI / 24.f;
					Points.Add(Centre + FVector2f(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius);
				}
				FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, Points, ESlateDrawEffect::None, AlertRed, true, Thickness);
			};
			switch (Shape)
			{
			case 1:
				Outline(Ring, 2.5f);
				Fill(6.f);
				break;
			case 2:
				Fill(Ring);
				break;
			case 3:
				Outline(Ring, 2.5f);
				Outline(Ring * 0.55f, 2.5f);
				break;
			case 4:
				FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, TArray<FVector2f>{ Centre - FVector2f(Ring * 0.5f, 0.f), Centre + FVector2f(Ring * 0.5f, 0.f) }, ESlateDrawEffect::None, AlertRed, true, 3.f);
				FSlateDrawElement::MakeLines(OutDrawElements, Layer, Whole, TArray<FVector2f>{ Centre - FVector2f(0.f, Ring * 0.5f), Centre + FVector2f(0.f, Ring * 0.5f) }, ESlateDrawEffect::None, AlertRed, true, 3.f);
				break;
			default:
				// 0: the circle (MM-Q1): a ring of the alert's size and a round core.
				{
					// MM-Q1 (2026-11-01): a circle that tightens on the struck point, again and again (the ping / radar manner).
					const float Period = FMath::Clamp(CVarMinimapAlertShrink.GetValueOnGameThread(), 0.2f, 10.f);
					const float Phase = FMath::Fmod(static_cast<float>(FPlatformTime::Seconds()), Period) / Period;
					const float Diameter = FMath::Lerp(Ring, 8.f, Phase);
					Circle(Diameter * 0.5f, 3.f);
					Circle(1.5f, 3.f);
				}
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
	// P2: 230 400 texels read, the fog blurred first, the picture sent to the card: not again when nothing it shows has changed
	// (the fog's picture has a version, the sources a signature, the frame is two corners).
	const UHOLSourceSubsystem* SourceRegistry = World->GetSubsystem<UHOLSourceSubsystem>();
	FTerrainKey Key;
	Key.FogVersion = Fog ? Fog->GetVersion() : 0;
	Key.bFogActive = Fog && Fog->IsFogActive();
	Key.Min = FrameMin;
	Key.Max = FrameMax;
	Key.Sources = SourceRegistry ? HOLMinimapTerrain::GetSourcesSignature(SourceRegistry->GetSources()) : 0;
	if (bTerrainBuilt && Key == BuiltKey)
	{
		return;
	}
	bTerrainBuilt = true;
	BuiltKey = Key;
	++TerrainBuilds;
	// The same line as HOLMinimap::ToWorld (Script/UI/HOLMinimapRules.as): screen right is world +Y, screen up is world +X.
	auto TexelOf = [&](const double WorldX, const double WorldY, int32& OutX, int32& OutY)
	{
		OutX = FMath::FloorToInt((WorldY - FrameMin.Y) / Width * TerrainSide);
		OutY = FMath::FloorToInt((FrameMax.X - WorldX) / Height * TerrainSide);
		return OutX >= 0 && OutX < TerrainSide && OutY >= 0 && OutY < TerrainSide;
	};

	// The cells blurred once (1-2-1), then blended between their centres at each texel: the edge of what is seen is a soft slope a few
	// pixels wide, not the 4 m steps of the grid (which are 1 to 2 px of the picture on a 400 m map).
	TArray<uint8> Blurred;
	if (Fog && Fog->IsFogActive())
	{
		HOLFogPicture::Blur(Fog->GetGrid(), Fog->GetBytes(), Blurred);
	}
	TArray<FColor> Pixels;
	Pixels.SetNumUninitialized(TerrainSide * TerrainSide);
	NumInSight = 0;
	// A texel is "in sight" from halfway between the explored and the in-sight bytes (as HOLFogPicture::FromByte).
	const float InSightFrom = (HOLFogPicture::ExploredByte + HOLFogPicture::VisibleByte) / 2.f / 255.f;
	for (int32 Ty = 0; Ty < TerrainSide; ++Ty)
	{
		for (int32 Tx = 0; Tx < TerrainSide; ++Tx)
		{
			const double WorldY = FrameMin.Y + (Tx + 0.5) / TerrainSide * Width;
			const double WorldX = FrameMax.X - (Ty + 0.5) / TerrainSide * Height;
			const float Vision = ReadVisionSmooth(Fog, Blurred, FVector2D(WorldX, WorldY));
			NumInSight += Vision >= InSightFrom ? 1 : 0;
			Pixels[Ty * TerrainSide + Tx] = TerrainColor(Vision).ToFColor(false);
		}
	}
	// The sources the camp has explored: a round dot each (about 3 texels across), in their resource's colour, its edge blended into the ground.
	NumSourcesDrawn = 0;
	if (const UHOLSourceSubsystem* Sources = SourceRegistry)
	{
		constexpr float DotRadius = 3.2f;
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
			const int32 Reach = FMath::CeilToInt(DotRadius) + 1;
			for (int32 Dy = -Reach; Dy <= Reach; ++Dy)
			{
				for (int32 Dx = -Reach; Dx <= Reach; ++Dx)
				{
					const int32 X = Tx + Dx;
					const int32 Y = Ty + Dy;
					if (X < 0 || Y < 0 || X >= TerrainSide || Y >= TerrainSide)
					{
						continue;
					}
					// Coverage falls from 1 to 0 across the dot's last texel.
					const float Coverage = FMath::Clamp(DotRadius + 0.5f - FMath::Sqrt(static_cast<float>(Dx * Dx + Dy * Dy)), 0.f, 1.f);
					if (Coverage <= 0.f)
					{
						continue;
					}
					FColor& Pixel = Pixels[Y * TerrainSide + X];
					Pixel = FColor(static_cast<uint8>(FMath::Lerp<float>(Pixel.R, Color.R, Coverage)), static_cast<uint8>(FMath::Lerp<float>(Pixel.G, Color.G, Coverage)),
						static_cast<uint8>(FMath::Lerp<float>(Pixel.B, Color.B, Coverage)), 255);
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
	for (FHOLBuildingIterator It(World); It; ++It)
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
	return FString::Printf(TEXT("minimap: %.0f px, %d marks, %d alerts lit, camera frame %d points, terrain %d x %d (sources %d, in sight %d texels, made %d times)"),
		Side, Marks.Num(), Alerts.Num(), CameraFrame.Num(), Terrain ? TerrainSide : 0, Terrain ? TerrainSide : 0, NumSourcesDrawn, NumInSight, TerrainBuilds);
}

void UHOLMinimapView::BroadcastPressed(const FVector2D& Position, const bool bRight, const bool bDrag)
{
	OnPressed.Broadcast(Position, bRight, bDrag);
}
