#include "Widgets/HOLDropdown.h"

#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateRoundedBoxBrush.h"
#include "CommonTextBlock.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Framework/Application/SlateApplication.h"
#include "HeroesOfLibertyUILog.h"
#include "Style/HOLUIStyles.h"

namespace UE::HeroesOfLiberty::UI::Dropdown
{
	/** The padding of the menu buttons (UHOLButtonStyle), carried by the content of the dropdown. */
	const FMargin ContentPadding(24.f, 10.f);
	constexpr float SwatchSize = 20.f;
	/** A click that closes the list by landing on the dropdown also clicks it: no reopening this soon after. */
	constexpr double ReopenDelaySeconds = 0.3;
}

UHOLDropdown::UHOLDropdown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Style = UHOLDropdownStyle::StaticClass();
}

void UHOLDropdown::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::Dropdown;

	// Runs inside UCommonButtonBase::Initialize, before it wraps the root widget into its internal button. The anchor is
	// the whole face of the button, so the list opens right under its edge.
	if (!WidgetTree->RootWidget)
	{
		Anchor = WidgetTree->ConstructWidget<UHOLMenuAnchor>(UHOLMenuAnchor::StaticClass(), TEXT("Anchor"));
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
		ValueText = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("ValueText"));
		Arrow = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("Arrow"));
		// The chevron of the interface font turned down (the font has no down arrow).
		Arrow->SetText(INVTEXT("›"));
		Arrow->SetRenderTransformAngle(90.f);
		ValueText->SetJustification(ETextJustify::Center);
		UHorizontalBoxSlot* TextSlot = Row->AddChildToHorizontalBox(ValueText);
		TextSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		TextSlot->SetVerticalAlignment(VAlign_Center);
		TextSlot->SetPadding(FMargin(ContentPadding.Left, ContentPadding.Top, 8.f, ContentPadding.Bottom));
		UHorizontalBoxSlot* ArrowSlot = Row->AddChildToHorizontalBox(Arrow);
		ArrowSlot->SetVerticalAlignment(VAlign_Center);
		ArrowSlot->SetPadding(FMargin(0.f, 0.f, ContentPadding.Right * 0.5f, 0.f));
		Anchor->SetContent(Row);
		WidgetTree->RootWidget = Anchor;
	}
	if (Anchor)
	{
		Anchor->OnGetUserMenuContentEvent.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(UHOLDropdown, MakeList));
		Anchor->OnMenuOpenChanged.AddDynamic(this, &UHOLDropdown::HandleListOpenChanged);
	}
	Super::NativeOnInitialized();
	HOLUIStyle::ShowFocus(*this, UHOLDropdownStyleFocused::StaticClass());
}

void UHOLDropdown::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();
	for (UCommonTextBlock* Block : { ValueText.Get(), Arrow.Get() })
	{
		if (Block)
		{
			Block->SetStyle(GetCurrentTextStyleClass());
		}
	}
}

void UHOLDropdown::NativeOnClicked()
{
	Super::NativeOnClicked();
	if (FPlatformTime::Seconds() - ListClosedTime > UE::HeroesOfLiberty::UI::Dropdown::ReopenDelaySeconds)
	{
		OpenList();
	}
}

void UHOLDropdown::SetOptions(const TArray<FText>& InOptions, const int32 InSelectedIndex)
{
	Options = InOptions;
	OptionEnabled.Init(true, Options.Num());
	SetSelectedIndex(InSelectedIndex);
}

void UHOLDropdown::SetOptionSwatches(const TArray<FLinearColor>& InSwatches)
{
	Swatches = InSwatches;
}

void UHOLDropdown::SetOptionEnabled(const int32 Index, const bool bEnabled)
{
	if (OptionEnabled.IsValidIndex(Index))
	{
		OptionEnabled[Index] = bEnabled;
		if (UHOLDropdownOption* Option = List ? List->GetOption(Index) : nullptr)
		{
			Option->SetIsEnabled(bEnabled);
		}
	}
}

void UHOLDropdown::SetSelectedIndex(const int32 Index)
{
	SelectedIndex = FMath::Clamp(Index, 0, FMath::Max(0, Options.Num() - 1));
	if (ValueText)
	{
		ValueText->SetText(Options.IsValidIndex(SelectedIndex) ? Options[SelectedIndex] : FText::GetEmpty());
	}
}

FText UHOLDropdown::GetShownText() const
{
	return ValueText ? ValueText->GetText() : Options.IsValidIndex(SelectedIndex) ? Options[SelectedIndex] : FText::GetEmpty();
}

void UHOLDropdown::SetShownText(const FText& Text)
{
	if (ValueText)
	{
		ValueText->SetText(Text);
	}
}

bool UHOLDropdown::IsListOpen() const
{
	return Anchor && Anchor->IsOpen();
}

void UHOLDropdown::OpenList()
{
	if (Anchor && !Anchor->IsOpen() && GetIsEnabled() && !Options.IsEmpty())
	{
		// The anchor puts the list into its own pop-up layer (game viewport: current window) and gives it the focus.
		Anchor->Open(/*bFocusMenu*/ true);
		if (List)
		{
			List->FocusSelected();
		}
	}
}

void UHOLDropdown::CloseList()
{
	if (Anchor && Anchor->IsOpen())
	{
		Anchor->Close();
	}
}

UUserWidget* UHOLDropdown::MakeList()
{
	List = CreateWidget<UHOLDropdownList>(this, UHOLDropdownList::StaticClass());
	List->Build(*this, GetCachedGeometry().GetLocalSize().X);
	return List;
}

void UHOLDropdown::HandleListOpenChanged(const bool bIsOpen)
{
	UE_LOG(LogHOLUI, Display, TEXT("Dropdown %s: list %s"), *GetName(), bIsOpen ? TEXT("opened") : TEXT("closed"));
	if (!bIsOpen)
	{
		ListClosedTime = FPlatformTime::Seconds();
		List = nullptr;
		// The focus was in the list, which is gone: back on the dropdown (Back, a value taken with the gamepad).
		SetFocus();
	}
}

void UHOLDropdown::Choose(const int32 Index)
{
	CloseList();
	if (Index != SelectedIndex && OptionEnabled.IsValidIndex(Index) && OptionEnabled[Index])
	{
		SetSelectedIndex(Index);
		UE_LOG(LogHOLUI, Display, TEXT("Dropdown %s: value %d chosen"), *GetName(), Index);
		OnSelectionChanged.Broadcast(Index);
	}
}

void UHOLDropdownOption::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::Dropdown;

	if (!WidgetTree->RootWidget)
	{
		UHorizontalBox* Row = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Row"));
		Swatch = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Swatch"));
		Swatch->SetDesiredSizeOverride(FVector2D(SwatchSize, SwatchSize));
		Swatch->SetVisibility(ESlateVisibility::Collapsed);
		Label = WidgetTree->ConstructWidget<UCommonTextBlock>(UCommonTextBlock::StaticClass(), TEXT("Label"));
		Label->SetJustification(ETextJustify::Center);
		UHorizontalBoxSlot* SwatchSlot = Row->AddChildToHorizontalBox(Swatch);
		SwatchSlot->SetVerticalAlignment(VAlign_Center);
		SwatchSlot->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
		UHorizontalBoxSlot* LabelSlot = Row->AddChildToHorizontalBox(Label);
		LabelSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
		WidgetTree->RootWidget = Row;
	}
	Super::NativeOnInitialized();
}

void UHOLDropdownOption::SetSwatch(const TOptional<FLinearColor> Color)
{
	if (Swatch)
	{
		Swatch->SetVisibility(Color.IsSet() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		Swatch->SetColorAndOpacity(Color.Get(FLinearColor::White));
	}
	if (Label)
	{
		// A swatch and its name read as one column of values.
		Label->SetJustification(Color.IsSet() ? ETextJustify::Left : ETextJustify::Center);
	}
}

void UHOLDropdownList::Build(UHOLDropdown& InOwner, const float MinWidth)
{
	Owner = &InOwner;

	USizeBox* Box = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("Box"));
	Box->SetMinDesiredWidth(MinWidth);
	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrush(FSlateRoundedBoxBrush(HOLUIStyle::Panel, 4.f, HOLUIStyle::Accent, 1.f));
	Panel->SetPadding(FMargin(4.f));
	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Column"));
	Panel->SetContent(Column);
	Box->SetContent(Panel);

	for (int32 Index = 0; Index < InOwner.Options.Num(); ++Index)
	{
		UHOLDropdownOption* Option = WidgetTree->ConstructWidget<UHOLDropdownOption>(UHOLDropdownOption::StaticClass());
		Option->SetLabel(InOwner.Options[Index]);
		Option->SetSwatch(InOwner.Swatches.IsValidIndex(Index) ? TOptional<FLinearColor>(InOwner.Swatches[Index]) : TOptional<FLinearColor>());
		Option->SetIsEnabled(InOwner.OptionEnabled[Index]);
		Option->OnClicked().AddWeakLambda(this, [this, Index]()
		{
			if (Owner.IsValid())
			{
				Owner->Choose(Index);
			}
		});
		Column->AddChildToVerticalBox(Option)->SetPadding(FMargin(0.f, 1.f));
		Buttons.Add(Option);
	}

	// Up and Down go round the list; nothing leaves it but a choice, Back or a click outside.
	Box->SetNavigationRuleBase(EUINavigation::Up, EUINavigationRule::Wrap);
	Box->SetNavigationRuleBase(EUINavigation::Down, EUINavigationRule::Wrap);
	Box->SetNavigationRuleBase(EUINavigation::Left, EUINavigationRule::Stop);
	Box->SetNavigationRuleBase(EUINavigation::Right, EUINavigationRule::Stop);
	Box->SetNavigationRuleBase(EUINavigation::Next, EUINavigationRule::Wrap);
	Box->SetNavigationRuleBase(EUINavigation::Previous, EUINavigationRule::Wrap);
	WidgetTree->RootWidget = Box;
}

void UHOLDropdownList::FocusSelected()
{
	const int32 Selected = Owner.IsValid() ? Owner->GetSelectedIndex() : 0;
	UHOLDropdownOption* Target = Buttons.IsValidIndex(Selected) && Buttons[Selected]->GetIsEnabled() ? Buttons[Selected].Get() : nullptr;
	for (int32 Index = 0; Index < Buttons.Num() && !Target; ++Index)
	{
		if (Buttons[Index]->GetIsEnabled())
		{
			Target = Buttons[Index];
		}
	}
	if (Target)
	{
		Target->SetFocus();
	}
}

FReply UHOLDropdownList::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// Back closes the list only: it must not reach the screen, whose Back would close it too.
	const FKey Key = InKeyEvent.GetKey();
	if (FSlateApplication::Get().GetNavigationActionFromKey(InKeyEvent) == EUINavigationAction::Back
		|| Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)
	{
		if (Owner.IsValid())
		{
			Owner->CloseList();
		}
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
