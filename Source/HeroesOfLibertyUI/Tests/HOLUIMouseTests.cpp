#include "Dev/HOLUIMouse.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Rendering/DrawElements.h"
#include "Widgets/IToolTip.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace UE::HeroesOfLiberty::Tests::Mouse
{
	/** A tooltip that counts the times Slate opens it (IToolTip::OnOpening, called by the cursor user when it shows the tip). */
	class SCountingToolTip : public SCompoundWidget, public IToolTip
	{
	public:
		SLATE_BEGIN_ARGS(SCountingToolTip) {}
		SLATE_END_ARGS()

		void Construct(const FArguments&)
		{
			ChildSlot
			[
				SNew(STextBlock).Text(INVTEXT("+120 or/min"))
			];
		}

		int32 Openings = 0;

		virtual TSharedRef<SWidget> AsWidget() override { return AsShared(); }
		virtual TSharedRef<SWidget> GetContentWidget() override { return AsShared(); }
		virtual void SetContentWidget(const TSharedRef<SWidget>&) override {}
		virtual bool IsEmpty() const override { return false; }
		virtual bool IsInteractive() const override { return false; }
		virtual void OnOpening() override { ++Openings; }
		virtual void OnClosed() override {}
	};
}

// L14-5 (real gestures): the scripts' mouse goes through Slate's platform entry points. In a real window (out of the desktop's
// sight), a click at the centre of a button runs its OnClicked (hover, press, capture and release as the hand's mouse: the menu's
// PlayButton did not open with hol.UI.Click's routed press), and the mouse kept on an amount makes it the hovered widget, the one
// under the cursor user's cursor, and Slate opens its tooltip (income_tooltip).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUIMouseClickHoverTest, "HeroesOfLiberty.UI.Mouse.ClickAndHover",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLUIMouseClickHoverTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty;
	if (!FSlateApplication::IsInitialized())
	{
		AddError(TEXT("no Slate application"));
		return false;
	}
	FSlateApplication& Slate = FSlateApplication::Get();
	int32 Clicks = 0;
	TSharedPtr<SButton> Button;
	TSharedPtr<STextBlock> Amount;
	const TSharedRef<Tests::Mouse::SCountingToolTip> Tip = SNew(Tests::Mouse::SCountingToolTip);
	const TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(INVTEXT("HOL mouse test"))
		.CreateTitleBar(false)
		.UseOSWindowBorder(false)
		.HasCloseButton(false)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::FixedSize)
		.AutoCenter(EAutoCenter::None)
		.ScreenPosition(FVector2D(-12000.0, -12000.0))
		.ClientSize(FVector2D(320.0, 160.0))
		.ActivationPolicy(EWindowActivationPolicy::Never)
		.FocusWhenFirstShown(false)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(20.f)
			[
				SAssignNew(Button, SButton)
				.OnClicked_Lambda([&Clicks]() { ++Clicks; return FReply::Handled(); })
				[
					SNew(STextBlock).Text(INVTEXT("Jouer"))
				]
			]
			+ SVerticalBox::Slot().AutoHeight().Padding(20.f)
			[
				SAssignNew(Amount, STextBlock)
				.Text(INVTEXT("2100"))
				.ToolTip(Tip)
			]
		];
	Slate.AddWindow(Window, true);
	ON_SCOPE_EXIT
	{
		UI::Mouse::RestorePlatformCursor();
		FSlateApplication::Get().CloseToolTip();
		FSlateApplication::Get().DestroyWindowImmediately(Window);
	};
	// Drawn once, as a frame does: the window's hit test grid and the widgets' geometry.
	Window->SlatePrepass(Slate.GetApplicationScale() * Window->GetDPIScaleFactor());
	{
		FSlateWindowElementList Elements(Window);
		Window->PaintWindow(Slate.GetCurrentTime(), 0.f, Elements, FWidgetStyle(), true);
	}

	UI::Mouse::Click(UI::Mouse::GetCentre(*Button));
	TestEqual(TEXT("a click of the scripts' mouse at the button's centre runs its OnClicked once"), Clicks, 1);

	UI::Mouse::MoveTo(UI::Mouse::GetCentre(*Amount));
	TestTrue(TEXT("the amount is the hovered widget after the move"), Amount->IsHovered());
	TestFalse(TEXT("the button is left when the mouse goes to the amount"), Button->IsHovered());
	const TSharedPtr<FSlateUser> User = Slate.GetCursorUser();
	TestTrue(TEXT("the amount is under the cursor user's cursor (the tooltips' hit test)"), User.IsValid() && User->IsWidgetDirectlyUnderCursor(Amount));
	// Slate opens a tooltip once the mouse has been still a moment (0.05 s), on its tick's tooltip update.
	FPlatformProcess::Sleep(0.1f);
	Slate.UpdateToolTip(true);
	if (Slate.IsActive())
	{
		TestEqual(TEXT("Slate opens the amount's tooltip under the still mouse"), Tip->Openings, 1);
	}
	else
	{
		AddInfo(TEXT("application inactive: Slate opens no tooltip, the tooltip's opening is not checked"));
	}
	return true;
}

#endif
