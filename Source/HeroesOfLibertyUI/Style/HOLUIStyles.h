#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "HOLUIStyles.generated.h"

/**
 * Interface styles, written in C++ (D242): provisional looks until there is an art direction. The font is the
 * project's composite font F_HOL: the engine's Roboto, with Noto Sans JP in regular and bold for Japanese (D245, D251).
 */
namespace HOLUIStyle
{
	/** Colours shared by the styles and the screens' backgrounds. */
	const FLinearColor Background(0.012f, 0.014f, 0.02f, 1.f);
	const FLinearColor Text(0.92f, 0.9f, 0.85f, 1.f);
	const FLinearColor TextDisabled(0.4f, 0.4f, 0.4f, 1.f);
	const FLinearColor Accent(0.85f, 0.65f, 0.25f, 1.f);
	/** Panel of a dialog, and the veil laid over the screen below it. */
	const FLinearColor Panel(0.03f, 0.035f, 0.045f, 1.f);
	const FLinearColor Veil(0.f, 0.f, 0.f, 0.6f);

	/** Shows the keyboard focus on a button: the focused style while it has it (UHOLButtonStyleFocused by default), its
	 *  own style otherwise. */
	HEROESOFLIBERTYUI_API void ShowFocus(UCommonButtonBase& Button, TSubclassOf<UCommonButtonStyle> FocusedStyle = nullptr);
}

/** Base of the text styles: the interface font (F_HOL) at a given size and typeface. */
UCLASS(Abstract)
class HEROESOFLIBERTYUI_API UHOLTextStyle : public UCommonTextStyle
{
	GENERATED_BODY()

protected:
	void SetFont(float Size, FName Typeface, const FLinearColor& InColor);
};

/** Game title on the title screen. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleTitle : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleTitle();
};

/** Screen headings. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHeading : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHeading();
};

/** Labels and body text. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleBody : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleBody();
};

/** What does not act yet (D240: greyed). */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleDisabled : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleDisabled();
};

/** Notes under a screen (what is greyed and why). */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleNote : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleNote();
};

/** Menu buttons: flat, rounded, the accent colour when hovered. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLButtonStyle : public UCommonButtonStyle
{
	GENERATED_BODY()

public:
	UHOLButtonStyle();
};

/** A menu button that has the keyboard focus: outlined in the accent colour (hovering looks the same with a mouse). */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLButtonStyleFocused : public UHOLButtonStyle
{
	GENERATED_BODY()

public:
	UHOLButtonStyleFocused();
};

/** A drop-down list (UHOLDropdown): the menu button without padding, which its content carries instead, so that the
 *  list opens right under its edge. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLDropdownStyle : public UHOLButtonStyle
{
	GENERATED_BODY()

public:
	UHOLDropdownStyle();
};

/** A drop-down list that has the keyboard focus. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLDropdownStyleFocused : public UHOLButtonStyleFocused
{
	GENERATED_BODY()

public:
	UHOLDropdownStyleFocused();
};
