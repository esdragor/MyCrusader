#include "Dev/HOLUIMouse.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "CommonActivatableWidget.h"
#include "Components/Widget.h"
#include "Containers/Ticker.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/ICursor.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "HUD/HOLCheatPanel.h"
#include "HUD/HOLGameHUD.h"
#include "HUD/HOLGameScreen.h"
#include "Widgets/HOLRootLayout.h"
#include "Widgets/IToolTip.h"
#include "Widgets/SViewport.h"
#include "Widgets/SWidget.h"

namespace UE::HeroesOfLiberty::UI::Mouse
{
	namespace
	{
		/** True once the scripts took the cursor user's cursor (Slate's faux cursor: a position only, the desktop's cursor is not touched). */
		bool bScriptCursor = false;

		/** The cursor user with Slate's own faux cursor in place of the platform's (installed on first use), as a gamepad's cursor is. */
		TSharedPtr<FSlateUser> TakeCursor()
		{
			if (!FSlateApplication::IsInitialized())
			{
				return nullptr;
			}
			FSlateApplication& Slate = FSlateApplication::Get();
			if (!bScriptCursor)
			{
				const FVector2f Start = Slate.GetCursorPos();
				Slate.UsePlatformCursorForCursorUser(false);
				bScriptCursor = true;
				if (const TSharedPtr<FSlateUser> User = Slate.GetCursorUser())
				{
					User->SetCursorPosition(Start);
				}
			}
			return Slate.GetCursorUser();
		}

		/** A mouse event of the cursor user at Position, as FSlateApplication::OnMouseDown / OnMouseUp build the platform's. */
		FPointerEvent MakeEvent(const FSlateUser& User, const FVector2D& Position, const FVector2D& Last, const FKey Button)
		{
			FSlateApplication& Slate = FSlateApplication::Get();
			return FPointerEvent(User.GetUserIndex(), FSlateApplication::CursorPointerIndex, Position, Last, Slate.GetPressedMouseButtons(), Button, 0.f,
				Slate.GetModifierKeys());
		}
	}

	void MoveTo(const FVector2D& Position)
	{
		const TSharedPtr<FSlateUser> User = TakeCursor();
		if (!User.IsValid())
		{
			return;
		}
		const FVector2D Last(User->GetCursorPosition());
		User->SetCursorPosition(FMath::RoundToInt(Position.X), FMath::RoundToInt(Position.Y));
		// As FSlateApplication::OnMouseMove does for the platform's move (without its count of the platform's moves, which would give
		// the cursor back to the desktop's mouse): Slate hit tests, sends the leave, the enter and the move.
		FSlateApplication::Get().ProcessMouseMoveEvent(MakeEvent(*User, FVector2D(User->GetCursorPosition()), Last, EKeys::Invalid));
	}

	void Click(const FVector2D& Position)
	{
		MoveTo(Position);
		const TSharedPtr<FSlateUser> User = bScriptCursor ? FSlateApplication::Get().GetCursorUser() : nullptr;
		if (!User.IsValid())
		{
			return;
		}
		FSlateApplication& Slate = FSlateApplication::Get();
		const FVector2D Point(User->GetCursorPosition());
		Slate.ProcessMouseButtonDownEvent(nullptr, MakeEvent(*User, Point, Point, EKeys::LeftMouseButton));
		Slate.ProcessMouseButtonUpEvent(MakeEvent(*User, Point, Point, EKeys::LeftMouseButton));
	}

	void RestorePlatformCursor()
	{
		if (bScriptCursor && FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().UsePlatformCursorForCursorUser(true);
		}
		bScriptCursor = false;
	}

	FVector2D GetCentre(const SWidget& Widget)
	{
		return FVector2D(Widget.GetTickSpaceGeometry().GetAbsolutePositionAtCoordinates(FVector2f(0.5f, 0.5f)));
	}

	namespace
	{
		/** The widget named Name in Root's tree or in the trees of the user widgets it holds. */
		UWidget* FindIn(const UUserWidget* Root, const FName Name, const int32 Depth)
		{
			if (!Root || !Root->WidgetTree || Depth > 8)
			{
				return nullptr;
			}
			if (UWidget* Found = Root->WidgetTree->FindWidget(Name))
			{
				return Found;
			}
			TArray<UWidget*> Widgets;
			Root->WidgetTree->GetAllWidgets(Widgets);
			for (UWidget* Widget : Widgets)
			{
				if (UWidget* Found = FindIn(Cast<UUserWidget>(Widget), Name, Depth + 1))
				{
					return Found;
				}
			}
			return nullptr;
		}

		/** A drawn widget named Name: on the open menu screen, then the game HUD (screen, cheats' panel, scenario's panel), then the layout. */
		UWidget* FindNamed(UWorld* World, const FName Name)
		{
			APlayerController* Player = World ? World->GetFirstPlayerController() : nullptr;
			const UHOLRootLayout* Layout = UHOLRootLayout::Get(Player);
			const UUserWidget* Roots[] = { Layout ? Layout->GetActiveMenu() : nullptr, AHOLGameHUD::FindScreen(Player),
				AHOLGameHUD::FindCheatPanel(Player), Layout };
			for (const UUserWidget* Root : Roots)
			{
				UWidget* Found = FindIn(Root, Name, 0);
				if (Found && Found->GetCachedWidget().IsValid())
				{
					return Found;
				}
			}
			return nullptr;
		}

		/** The point of "<WidgetName>" or "<X> <Y>" (the viewport's pixels); Next: the index of the first argument after the target. */
		bool ResolveTarget(const TArray<FString>& Args, UWorld* World, FVector2D& OutPoint, FString& OutLabel, UWidget*& OutWidget, int32& OutNext)
		{
			OutWidget = nullptr;
			if (Args.Num() >= 2 && Args[0].IsNumeric() && Args[1].IsNumeric())
			{
				const UGameViewportClient* GameViewport = World ? World->GetGameViewport() : nullptr;
				const TSharedPtr<SViewport> Viewport = GameViewport ? GameViewport->GetGameViewportWidget() : nullptr;
				if (!Viewport.IsValid())
				{
					return false;
				}
				OutPoint = FVector2D(Viewport->GetTickSpaceGeometry().LocalToAbsolute(FVector2D(FCString::Atof(*Args[0]), FCString::Atof(*Args[1]))));
				OutLabel = FString::Printf(TEXT("%s,%s"), *Args[0], *Args[1]);
				OutNext = 2;
				return true;
			}
			if (Args.IsEmpty())
			{
				return false;
			}
			OutWidget = FindNamed(World, FName(*Args[0]));
			if (!OutWidget)
			{
				return false;
			}
			OutPoint = GetCentre(*OutWidget->GetCachedWidget());
			OutLabel = Args[0];
			OutNext = 1;
			return true;
		}

		/** The deepest widget Slate's hit test finds at Point, and the nearest tooltip on its path ("none" without one). */
		FString DescribePoint(const FVector2D& Point, TSharedPtr<IToolTip>* OutTip = nullptr)
		{
			FSlateApplication& Slate = FSlateApplication::Get();
			const FWidgetPath Path = Slate.LocateWindowUnderMouse(Point, Slate.GetInteractiveTopLevelWindows(), true);
			if (!Path.IsValid() || Path.Widgets.Num() == 0)
			{
				return TEXT("nothing under the point");
			}
			if (OutTip)
			{
				for (int32 Level = Path.Widgets.Num() - 1; Level >= 0; --Level)
				{
					const TSharedPtr<IToolTip> Tip = Path.Widgets[Level].Widget->GetToolTip();
					if (Tip.IsValid() && !Tip->IsEmpty())
					{
						*OutTip = Tip;
						break;
					}
				}
			}
			return Path.Widgets.Last().Widget->GetTypeAsString();
		}
	}

	void ClickVerb(const TArray<FString>& Args, UWorld* World)
	{
		FVector2D Point;
		FString Label;
		UWidget* Widget = nullptr;
		int32 Next = 0;
		if (!FSlateApplication::IsInitialized() || !ResolveTarget(Args, World, Point, Label, Widget, Next))
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.Mouse.Click <WidgetName | X Y> [Count=1]: no such drawn widget or no viewport (%s)"), *FString::Join(Args, TEXT(" ")));
			return;
		}
		const int32 Count = Args.IsValidIndex(Next) ? FMath::Clamp(FCString::Atoi(*Args[Next]), 1, 10) : 1;
		const FString Under = DescribePoint(Point);
		for (int32 Index = 0; Index < Count; ++Index)
		{
			Click(Point);
		}
		UE_LOG(LogHOLUI, Display, TEXT("Mouse click %s x%d at %.0f,%.0f: on %s"), *Label, Count, Point.X, Point.Y, *Under);
	}

	void HoverVerb(const TArray<FString>& Args, UWorld* World)
	{
		FVector2D Point;
		FString Label;
		UWidget* Widget = nullptr;
		int32 Next = 0;
		if (!FSlateApplication::IsInitialized() || !ResolveTarget(Args, World, Point, Label, Widget, Next))
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.Mouse.Hover <WidgetName | X Y> [Frames=30]: no such drawn widget or no viewport (%s)"), *FString::Join(Args, TEXT(" ")));
			return;
		}
		const int32 Frames = Args.IsValidIndex(Next) ? FMath::Clamp(FCString::Atoi(*Args[Next]), 1, 600) : 30;
		MoveTo(Point);
		const FText TipText = Widget ? Widget->GetToolTipText() : FText();
		// The mouse stays still: Slate's tick opens the tooltip (after its delay); the verdict is read after Frames frames.
		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda(
			[Remaining = Frames, Label, Point, TipText, Frames](float) mutable
			{
				if (--Remaining > 0)
				{
					return true;
				}
				if (!FSlateApplication::IsInitialized())
				{
					return false;
				}
				FSlateApplication& Slate = FSlateApplication::Get();
				TSharedPtr<IToolTip> Tip;
				const FString Under = DescribePoint(Point, &Tip);
				const bool bShown = Tip.IsValid() && Slate.FindWidgetWindow(Tip->AsWidget()).IsValid();
				UE_LOG(LogHOLUI, Display, TEXT("Mouse hover %s after %d frames at %.0f,%.0f: on %s, tooltip %s%s, tip text \"%s\""), *Label, Frames, Point.X, Point.Y,
					*Under, !Tip.IsValid() ? TEXT("none") : bShown ? TEXT("shown") : TEXT("not shown"),
					Slate.IsActive() ? TEXT("") : TEXT(" (application inactive)"), *TipText.ToString());
				return false;
			}));
	}

	FAutoConsoleCommandWithWorldAndArgs ClickCommand(
		TEXT("hol.Mouse.Click"),
		TEXT("hol.Mouse.Click <WidgetName | X Y> [Count=1]: a real left click (Slate's platform entry points: move, press, release) at the centre of a widget of the open menu screen or of the game HUD, or at X,Y in the viewport's pixels; the desktop's cursor is not moved"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&ClickVerb));

	FAutoConsoleCommandWithWorldAndArgs HoverCommand(
		TEXT("hol.Mouse.Hover"),
		TEXT("hol.Mouse.Hover <WidgetName | X Y> [Frames=30]: the mouse moved (Slate's platform entry point) to a widget's centre or to X,Y in the viewport's pixels and kept still Frames frames; logs \"Mouse hover ...\" with the tooltip Slate shows"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&HoverVerb));
}
