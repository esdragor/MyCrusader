#include "Containers/Ticker.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "InputCoreTypes.h"

/**
 * Key press sent to Slate as if it came from the keyboard or the gamepad, for the scripted end-to-end checks (D246):
 * it takes the path of a real key (focused widget, then the game viewport and the Common UI action router), without
 * driving the desktop.
 *   hol.UI.Key <KeyName> [DelaySeconds=3]   (Enter, Escape, Gamepad_FaceButton_Bottom, Gamepad_FaceButton_Right...)
 */
namespace UE::HeroesOfLiberty::UI
{
	void PressKey(const TArray<FString>& Args)
	{
		if (Args.IsEmpty())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("Usage: hol.UI.Key <KeyName> [DelaySeconds=3]"));
			return;
		}

		const FKey Key(*Args[0]);
		if (!Key.IsValid())
		{
			UE_LOG(LogHOLUI, Warning, TEXT("hol.UI.Key: unknown key '%s'"), *Args[0]);
			return;
		}
		const float Delay = Args.Num() > 1 ? FCString::Atof(*Args[1]) : 3.f;

		FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Key](float)
		{
			if (!FSlateApplication::IsInitialized())
			{
				return false;
			}
			FSlateApplication& Slate = FSlateApplication::Get();
			const FKeyEvent Event(Key, FModifierKeysState(), Slate.GetUserIndexForKeyboard(), /*bIsRepeat*/ false, 0, 0);
			UE_LOG(LogHOLUI, Display, TEXT("Key pressed: %s"), *Key.ToString());
			Slate.ProcessKeyDownEvent(Event);

			// Released a tenth of a second later, so that the press and the release are not seen in the same frame.
			FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([Event](float)
			{
				if (FSlateApplication::IsInitialized())
				{
					FSlateApplication::Get().ProcessKeyUpEvent(Event);
				}
				return false;
			}), 0.1f);
			return false;
		}), Delay);
	}

	FAutoConsoleCommand KeyCommand(
		TEXT("hol.UI.Key"),
		TEXT("hol.UI.Key <KeyName> [DelaySeconds=3]: sends a key press to the interface as if it came from the keyboard or the gamepad"),
		FConsoleCommandWithArgsDelegate::CreateStatic(&PressKey));
}
