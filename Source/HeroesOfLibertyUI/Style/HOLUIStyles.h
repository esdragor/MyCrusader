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
	/** Panels of the game HUD (J3, D281): see-through enough to keep the map readable around them. */
	const FLinearColor HUDPanel(0.03f, 0.035f, 0.045f, 0.88f);
	/** Labels and key letters of the HUD: dimmer than the values they name. */
	const FLinearColor TextMuted(0.62f, 0.6f, 0.56f, 1.f);
	/** Markers of the four resources (§ 5.1), in the top bar and on the costs: no icons yet. */
	const FLinearColor Food(0.7f, 0.12f, 0.08f, 1.f);
	const FLinearColor Wood(0.32f, 0.16f, 0.05f, 1.f);
	const FLinearColor Stone(0.45f, 0.45f, 0.45f, 1.f);
	const FLinearColor Gold(0.85f, 0.6f, 0.08f, 1.f);
	/** A cost the stock does not cover (D313): its amount in red. */
	const FLinearColor Missing(0.95f, 0.22f, 0.16f, 1.f);

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

/** Values of the game HUD (amounts, owner, health). */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUD : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUD();
};

/** Name of the selection in the HUD. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUDHeading : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUDHeading();
};

/** Small values of the HUD: names and costs in the command grid and the production queue. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUDSmall : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUDSmall();
};

/**
 * The players' names in the players' table, in their colour (V2.3d): bold, with a dark outline and a drop shadow so a
 * dark colour (blue, green, violet) still reads on the HUD's dark panel.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUDPlayerName : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUDPlayerName();
};

/**
 * The names in the cells of the command grid (J6a.5: « Camp de bûcherons » on two lines of a 76 px cell, the cell
 * showing nothing else, at 1440p as at 900p).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUDTiny : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUDTiny();
};

/** Labels, key letters and hints of the HUD. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUDLabel : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUDLabel();
};

/** What holds the player up, in the HUD ("Population full"). */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLTextStyleHUDAlert : public UHOLTextStyle
{
	GENERATED_BODY()

public:
	UHOLTextStyleHUDAlert();
};

/** Buttons of the HUD (cells of the command grid, items of the production queue): flat, square, the accent colour
 *  when hovered; their content gives their size. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLHUDButtonStyle : public UCommonButtonStyle
{
	GENERATED_BODY()

public:
	UHOLHUDButtonStyle();
};

/** A greyed cell of the grid that still takes a click (D313: not enough resources, the click says why): darker, a dim
 *  outline when hovered. */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLHUDButtonStyleGreyed : public UHOLHUDButtonStyle
{
	GENERATED_BODY()

public:
	UHOLHUDButtonStyleGreyed();
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
