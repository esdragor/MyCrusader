#include "Blueprint/WidgetTree.h"
#include "CommonActivatableWidget.h"
#include "Components/Widget.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "InputCoreTypes.h"
#include "Input/Events.h"
#include "Widgets/HOLRootLayout.h"

/**
 * Left mouse clicks sent to Slate at the centre of a widget of the open menu screen, for the scripted end-to-end checks
 * (FactionChoiceInProgress.md: the triple click on the title screen's title). The click goes the way of a real one: the widget
 * path is found by Slate's own hit test at that point (LocateWindowUnderMouse), the press and the release are routed to it and
 * bubble up to the screen (NativeOnMouseButtonDown).
 *   hol.UI.Click <WidgetName> [Count=1]   (Count clicks, one right after the other, within the same few milliseconds)
 */
namespace UE::HeroesOfLiberty::UI
{
	void Click(const TArray<FString>& Args, UWorld* World)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Click <WidgetName> [Count=1]"));
			return;
		}
		const int32 Count = Args.Num() > 1 ? FMath::Clamp(FCString::Atoi(*Args[1]), 1, 10) : 1;
		const UHOLRootLayout* Layout = UHOLRootLayout::Get(World ? World->GetFirstPlayerController() : nullptr);
		const UCommonActivatableWidget* Screen = Layout ? Layout->GetActiveMenu() : nullptr;
		const UWidget* Widget = Screen ? Screen->WidgetTree->FindWidget(FName(*Args[0])) : nullptr;
		if (!Widget || !FSlateApplication::IsInitialized())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Click: no widget '%s' on the open menu screen"), *Args[0]);
			return;
		}
		FSlateApplication& Slate = FSlateApplication::Get();
		const FGeometry Geometry = Widget->GetCachedGeometry();
		const FVector2D Centre = Geometry.GetAbsolutePositionAtCoordinates(FVector2D(0.5, 0.5));
		const FWidgetPath Path = Slate.LocateWindowUnderMouse(Centre, Slate.GetInteractiveTopLevelWindows());
		if (!Path.IsValid())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Click: nothing under the centre of '%s'"), *Args[0]);
			return;
		}
		const uint32 UserIndex = Slate.GetUserIndexForMouse();
		const TSet<FKey> Pressed = { EKeys::LeftMouseButton };
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const FPointerEvent Down(UserIndex, FSlateApplication::CursorPointerIndex, Centre, Centre, Pressed, EKeys::LeftMouseButton, 0.f, FModifierKeysState());
			Slate.RoutePointerDownEvent(Path, Down);
			const FPointerEvent Up(UserIndex, FSlateApplication::CursorPointerIndex, Centre, Centre, TSet<FKey>(), EKeys::LeftMouseButton, 0.f, FModifierKeysState());
			Slate.RoutePointerUpEvent(Path, Up);
		}
		UE_LOG(LogHOLUI, Display, TEXT("Clicked %s x%d"), *Args[0], Count);
	}

	FAutoConsoleCommandWithWorldAndArgs ClickCommand(
		TEXT("hol.UI.Click"),
		TEXT("hol.UI.Click <WidgetName> [Count=1]: left clicks at the centre of a widget of the open menu screen, through Slate's hit test and routing"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Click));
}
