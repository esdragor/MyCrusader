#include "Screens/HOLTitleScreen.h"

#include "CommonTextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/HOLButton.h"

#define LOCTEXT_NAMESPACE "HOLTitleScreen"

void UHOLTitleScreen::BuildLayout()
{
	UVerticalBox* Column = MakeCenteredColumn();
	// The game's name is not translated.
	Title = AddText(Column, TEXT("Title"), INVTEXT("Heroes of Liberty"), UHOLTextStyleTitle::StaticClass(), 48.f);
	QuitButton = AddButton(Column, TEXT("QuitButton"), LOCTEXT("Quit", "Quitter"));
}

void UHOLTitleScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	if (QuitButton)
	{
		QuitButton->OnClicked().AddUObject(this, &UHOLTitleScreen::HandleQuit);
	}
}

UWidget* UHOLTitleScreen::NativeGetDesiredFocusTarget() const
{
	return QuitButton;
}

void UHOLTitleScreen::HandleQuit()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

#undef LOCTEXT_NAMESPACE
