#include "Containers/Ticker.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "InputCoreTypes.h"

/**
 * Key press sent to Slate as if it came from the keyboard or the gamepad, for the scripted end-to-end checks (D246):
 * it takes the path of a real key (focused widget, then the game viewport and the Common UI action router), without
 * driving the desktop.
 *   hol.UI.Key <KeyName> [DelaySeconds=3] [HoldSeconds=0.1]   (Enter, Escape, Gamepad_FaceButton_Bottom, Left...)
 * MouseScrollUp and MouseScrollDown send one wheel step over the middle of the game window instead (the wheel is not a
 * key for Slate).
 */
namespace UE::HeroesOfLiberty::UI
{
	void PressKey(const TArray<FString>& Args)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Key <KeyName> [DelaySeconds=3] [HoldSeconds=0.1]"));
			return;
		}

		const FKey Key(*Args[0]);
		if (!Key.IsValid())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Key: unknown key '%s'"), *Args[0]);
			return;
		}
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 3.f;
		const float Hold = Args.Num() > 2 ? FCString::Atof(*Args[2]) : 0.1f;

		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Key, Hold](float)
		{
			if (!FSlateApplication::IsInitialized())
			{
				return false;
			}
			FSlateApplication& Slate = FSlateApplication::Get();
			if (Key == EKeys::MouseScrollUp || Key == EKeys::MouseScrollDown)
			{
				const TSharedPtr<SWindow> Window = GEngine && GEngine->GameViewport ? GEngine->GameViewport->GetWindow() : nullptr;
				if (!Window)
				{
					UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Key: no game window for the wheel"));
					return false;
				}
				const FVector2D Middle = Window->GetPositionInScreen() + Window->GetSizeInScreen() / 2.0;
				const FPointerEvent Wheel(Slate.GetUserIndexForMouse(), Middle, Middle, TSet<FKey>(), EKeys::MouseWheelAxis,
					Key == EKeys::MouseScrollUp ? 1.f : -1.f, FModifierKeysState());
				UE_LOG(LogHOLUI, Display, TEXT("Wheel: %s"), *Key.ToString());
				Slate.ProcessMouseWheelOrGestureEvent(Wheel, nullptr);
				return false;
			}
			const FKeyEvent Event(Key, FModifierKeysState(), Slate.GetUserIndexForKeyboard(), /*bIsRepeat*/ false, 0, 0);
			UE_LOG(LogHOLUI, Display, TEXT("Key pressed: %s"), *Key.ToString());
			Slate.ProcessKeyDownEvent(Event);

			// Released after the hold (a tenth of a second by default, so that the press and the release are not seen in
			// the same frame).
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Event](float)
			{
				if (FSlateApplication::IsInitialized())
				{
					FSlateApplication::Get().ProcessKeyUpEvent(Event);
				}
				return false;
			}), FMath::Max(Hold, 0.02f));
			return false;
		}), Delay);
	}

	FAutoConsoleCommand KeyCommand(
		TEXT("hol.UI.Key"),
		TEXT("hol.UI.Key <KeyName> [DelaySeconds=3] [HoldSeconds=0.1]: sends a key press to the interface as if it came from the keyboard or the gamepad (MouseScrollUp/Down: one wheel step)"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&PressKey));
}
