#include "Style/HOLUIStyles.h"

#include "Brushes/SlateRoundedBoxBrush.h"
#include "Engine/Font.h"
#include "UObject/ConstructorHelpers.h"

void UHOLTextStyle::SetFont(const float Size, const FName Typeface, const FLinearColor& InColor)
{
	static ConstructorHelpers::FObjectFinder<UFont> Roboto(TEXT("/Engine/EngineFonts/Roboto.Roboto"));
	Font = FSlateFontInfo(Roboto.Object, Size, Typeface);
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

void HOLUIStyle::ShowFocus(UCommonButtonBase& Button)
{
	const TWeakObjectPtr<UCommonButtonBase> WeakButton(&Button);
	const TSubclassOf<UCommonButtonStyle> OwnStyle = Button.GetStyle() ? Button.GetStyle()->GetClass() : nullptr;
	Button.OnFocusReceived().AddLambda([WeakButton]()
	{
		if (WeakButton.IsValid())
		{
			WeakButton->SetStyle(UHOLButtonStyleFocused::StaticClass());
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
