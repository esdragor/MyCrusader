#include "Blueprint/WidgetTree.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Dev/HOLUIType.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// D872 (GALADRIEL-1120pf): hol.UI.Type must fill the chat field of the game screen (a text box inside an overlay of the HUD tree), not only
// the menu screens' boxes, so that the scenario chat_input types the message instead of posting it by hol.Chat.Say.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUITypeChatFieldTest, "HeroesOfLiberty.UI.Type.GameScreenChatField",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLUITypeChatFieldTest::RunTest(const FString& Parameters)
{
	namespace HOLUI = UE::HeroesOfLiberty::UI;
	UWidgetTree* Tree = NewObject<UWidgetTree>(GetTransientPackage());
	UOverlay* Frame = Tree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Frame"));
	Tree->RootWidget = Frame;
	UEditableTextBox* Field = Tree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass(), TEXT("ChatInputBox"));
	Frame->AddChildToOverlay(Field);

	const FString Typed = TEXT("/w Joueur 2 salut");
	const bool bTyped = HOLUI::TypeIntoTree(Tree, FName(TEXT("ChatInputBox")), Typed);
	TestTrue(TEXT("the chat field of the game screen is found by its name and typed in"), bTyped);
	TestEqual(TEXT("the chat field holds the typed text (the field the Enter key then commits)"), Field->GetText().ToString(), Typed);

	const bool bOther = HOLUI::TypeIntoTree(Tree, FName(TEXT("NoSuchBox")), TEXT("ignored"));
	TestFalse(TEXT("a name that is no text box of the tree is refused"), bOther);
	TestEqual(TEXT("a refused type leaves the chat field as it was"), Field->GetText().ToString(), Typed);
	return true;
}

#endif