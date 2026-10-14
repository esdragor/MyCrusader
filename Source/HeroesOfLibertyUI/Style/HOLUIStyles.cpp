#include "Style/HOLUIStyles.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

void UHOLTextStyle::SetFont(const float Size, const FName Typeface, const FLinearColor& InColor)
{
	// Roboto of the engine with Noto Sans JP for Japanese (D251), made by Tools/Editor/make_ui_fonts.py.
	static ConstructorHelpers::FObjectFinder<UFont> InterfaceFont(TEXT("/Game/UI/Fonts/F_HOL.F_HOL"));
	Font = FSlateFontInfo(InterfaceFont.Object, Size, Typeface);
	Color = InColor;
}

UHOLTextStyleTitle::UHOLTextStyleTitle()
{
	SetFont(56.f, TEXT("Bold"), HOLUIStyle::Accent);
}

UHOLTextStyleHeading::UHOLTextStyleHeading()
{
	SetFont(28.f, TEXT("Bold"), HOLUIStyle::Text);
}

UHOLTextStyleBody::UHOLTextStyleBody()
{
	SetFont(18.f, TEXT("Regular"), HOLUIStyle::Text);
}

UHOLTextStyleDisabled::UHOLTextStyleDisabled()
{
	SetFont(18.f, TEXT("Regular"), HOLUIStyle::TextDisabled);
}

UHOLTextStyleNote::UHOLTextStyleNote()
{
	SetFont(14.f, TEXT("Italic"), HOLUIStyle::TextDisabled);
}

UHOLTextStyleHUD::UHOLTextStyleHUD()
{
	SetFont(16.f, TEXT("Regular"), HOLUIStyle::Text);
}

UHOLTextStyleHUDHeading::UHOLTextStyleHUDHeading()
{
	SetFont(18.f, TEXT("Bold"), HOLUIStyle::Text);
}

UHOLTextStyleHUDSmall::UHOLTextStyleHUDSmall()
{
	SetFont(13.f, TEXT("Regular"), HOLUIStyle::Text);
}

UHOLTextStyleHUDLabel::UHOLTextStyleHUDLabel()
{
	SetFont(13.f, TEXT("Regular"), HOLUIStyle::TextMuted);
}

UHOLTextStyleHUDAlert::UHOLTextStyleHUDAlert()
{
	SetFont(13.f, TEXT("Bold"), HOLUIStyle::Accent);
}

UHOLHUDButtonStyle::UHOLHUDButtonStyle()
{
	constexpr float Radius = 3.f;
	NormalBase = FSlateRoundedBoxBrush(FLinearColor(0.07f, 0.07f, 0.09f, 1.f), Radius);
	NormalHovered = FSlateRoundedBoxBrush(FLinearColor(0.16f, 0.12f, 0.05f, 1.f), Radius, HOLUIStyle::Accent, 1.f);
	NormalPressed = FSlateRoundedBoxBrush(FLinearColor(0.25f, 0.18f, 0.06f, 1.f), Radius, HOLUIStyle::Accent, 1.f);
	SelectedBase = NormalHovered;
	SelectedHovered = NormalHovered;
	SelectedPressed = NormalPressed;
	// Greyed cells and empty ones: a plain dark square.
	Disabled = FSlateRoundedBoxBrush(FLinearColor(0.035f, 0.035f, 0.045f, 1.f), Radius);
	ButtonPadding = FMargin(4.f);
	MinWidth = 0;
	MinHeight = 0;
	NormalTextStyle = UHOLTextStyleHUDSmall::StaticClass();
	NormalHoveredTextStyle = UHOLTextStyleHUDSmall::StaticClass();
	SelectedTextStyle = UHOLTextStyleHUDSmall::StaticClass();
	SelectedHoveredTextStyle = UHOLTextStyleHUDSmall::StaticClass();
	DisabledTextStyle = UHOLTextStyleHUDLabel::StaticClass();
}

UHOLButtonStyle::UHOLButtonStyle()
{
	constexpr float Radius = 4.f;
	NormalBase = FSlateRoundedBoxBrush(FLinearColor(0.06f, 0.06f, 0.08f, 0.9f), Radius);
	NormalHovered = FSlateRoundedBoxBrush(FLinearColor(0.16f, 0.12f, 0.05f, 0.95f), Radius, HOLUIStyle::Accent, 1.f);
	NormalPressed = FSlateRoundedBoxBrush(FLinearColor(0.25f, 0.18f, 0.06f, 1.f), Radius, HOLUIStyle::Accent, 1.f);
	SelectedBase = NormalHovered;
	SelectedHovered = NormalHovered;
	SelectedPressed = NormalPressed;
	Disabled = FSlateRoundedBoxBrush(FLinearColor(0.04f, 0.04f, 0.05f, 0.6f), Radius);
	ButtonPadding = FMargin(24.f, 10.f);
	// Widths come from the layout (columns, size boxes), so that the focused style never changes a button's size.
	MinWidth = 0;
	MinHeight = 48;
	NormalTextStyle = UHOLTextStyleBody::StaticClass();
	NormalHoveredTextStyle = UHOLTextStyleBody::StaticClass();
	SelectedTextStyle = UHOLTextStyleBody::StaticClass();
	SelectedHoveredTextStyle = UHOLTextStyleBody::StaticClass();
	DisabledTextStyle = UHOLTextStyleDisabled::StaticClass();
}

UHOLButtonStyleFocused::UHOLButtonStyleFocused()
{
	NormalBase = FSlateRoundedBoxBrush(FLinearColor(0.06f, 0.06f, 0.08f, 0.9f), 4.f, HOLUIStyle::Accent, 2.f);
}

UHOLDropdownStyle::UHOLDropdownStyle()
{
	ButtonPadding = FMargin(0.f);
}

UHOLDropdownStyleFocused::UHOLDropdownStyleFocused()
{
	ButtonPadding = FMargin(0.f);
}

void HOLUIStyle::ShowFocus(UCommonButtonBase& Button, TSubclassOf<UCommonButtonStyle> FocusedStyle)
{
	if (!FocusedStyle)
	{
		FocusedStyle = UHOLButtonStyleFocused::StaticClass();
	}
	const TWeakObjectPtr<UCommonButtonBase> WeakButton(&Button);
	const TSubclassOf<UCommonButtonStyle> OwnStyle = Button.GetStyle() ? Button.GetStyle()->GetClass() : nullptr;
	Button.OnFocusReceived().AddLambda([WeakButton, FocusedStyle]()
	{
		if (WeakButton.IsValid())
		{
			WeakButton->SetStyle(FocusedStyle);
		}
	});
	Button.OnFocusLost().AddLambda([WeakButton, OwnStyle]()
	{
		if (WeakButton.IsValid())
		{
			WeakButton->SetStyle(OwnStyle);
		}
	});
}
