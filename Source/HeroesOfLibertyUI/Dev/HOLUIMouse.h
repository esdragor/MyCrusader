#pragma once

#include "CoreMinimal.h"

class SWidget;

/**
 * The scripted checks' mouse (L14-5, real gestures): a cursor of the scripts moved, pressed and released through the entry points the
 * platform calls for a real mouse (FSlateApplication::OnMouseMove, OnMouseDown, OnMouseUp). Slate's own hit test picks the widget, and
 * the hover's enter and leave, the capture, the input preprocessors (Common UI's) and the tooltips follow as for the hand's mouse. The
 * desktop's cursor is never moved: the cursor user's cursor is replaced by a virtual one (FSlateUser::OverrideCursor), which stays where
 * the script left it, as a still mouse does.
 *   hol.Mouse.Click <WidgetName | X Y> [Count=1]   a left click at the centre of a widget of the open menu screen or of the game HUD
 *                                                  (found by its name), or at X,Y in the viewport's pixels (the world: a click on the ground)
 *   hol.Mouse.Hover <WidgetName | X Y> [Frames=30] the mouse moved there and kept still Frames frames; logs "Mouse hover <target>: ..."
 *                                                  with the tooltip Slate shows (income_tooltip: the top bar's FoodAmount...GoldAmount)
 */
namespace UE::HeroesOfLiberty::UI::Mouse
{
	/** Moves the scripts' mouse to Position (Slate's absolute space, the desktop's pixels); installs the virtual cursor on first use. */
	HEROESOFLIBERTYUI_API void MoveTo(const FVector2D& Position);

	/** A left click at Position: the move there, then the press and the release. */
	HEROESOFLIBERTYUI_API void Click(const FVector2D& Position);

	/** Gives the cursor user back the platform's cursor (the desktop's mouse). */
	HEROESOFLIBERTYUI_API void RestorePlatformCursor();

	/** The centre of a widget as last drawn, in Slate's absolute space. */
	HEROESOFLIBERTYUI_API FVector2D GetCentre(const SWidget& Widget);
}
