#include "Widgets/HOLRootLayout.h"

#include "Blueprint/WidgetTree.h"
#include "CommonActivatableWidget.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Dev/HOLTestHooks.h"
#include "HUD/HOLGameHUD.h"
#include "Menu/HOLMenuPlayerController.h"
#include "Style/HOLUIStyles.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Widgets/HOLScreenStack.h"

void UHOLRootLayout::NativeOnInitialized()
{
	if (!WidgetTree->RootWidget)
	{
		UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
		WidgetTree->RootWidget = Root;

		UImage* Background = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Background"));
		Background->SetColorAndOpacity(HOLUIStyle::Background);
		// Over a match (D356: the in-game menu), the game shows through and the clicks go to it while no screen is open.
		if (IsOverMatch())
		{
			Background->SetVisibility(ESlateVisibility::Collapsed);
			SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		// E2-3: stacks that keep a screen covered by a push while it fades in.
		MenuLayer = WidgetTree->ConstructWidget<UHOLScreenStack>(UHOLScreenStack::StaticClass(), TEXT("MenuLayer"));
		ModalLayer = WidgetTree->ConstructWidget<UHOLScreenStack>(UHOLScreenStack::StaticClass(), TEXT("ModalLayer"));

		for (UWidget* Child : { static_cast<UWidget*>(Background), static_cast<UWidget*>(MenuLayer.Get()), static_cast<UWidget*>(ModalLayer.Get()) })
		{
			UOverlaySlot* LayerSlot = Root->AddChildToOverlay(Child);
			LayerSlot->SetHorizontalAlignment(HAlign_Fill);
			LayerSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
	Super::NativeOnInitialized();
	if (IsOverMatch())
	{
		// No fade over a match (V1.3b): a dialog half faded let the in-game menu be read through it.
		for (UCommonActivatableWidgetStack* Layer : { MenuLayer.Get(), ModalLayer.Get() })
		{
			if (Layer)
			{
				Layer->SetTransitionDuration(0.f);
			}
		}
		// The game screen keeps the scripted checks' hooks (hud=...).
		return;
	}

	// For the scripted checks (hol.Test.Run): wait for the screens to stop fading, name the screens shown.
	for (UCommonActivatableWidgetStack* Layer : { MenuLayer.Get(), ModalLayer.Get() })
	{
		if (Layer)
		{
			Layer->OnTransitioningChanged.AddWeakLambda(this, [this](const UCommonActivatableWidgetContainerBase* Changed, const bool bIsTransitioning)
			{
				(Changed == MenuLayer ? bMenuFading : bModalFading) = bIsTransitioning;
			});
		}
	}
	const TWeakObjectPtr<UHOLRootLayout> WeakThis(this);
	HOLTestHooks::IsInterfaceBusy() = [WeakThis]()
	{
		return WeakThis.IsValid() && (WeakThis->bMenuFading || WeakThis->bModalFading);
	};
	HOLTestHooks::DescribeInterface() = [WeakThis]()
	{
		if (!WeakThis.IsValid())
		{
			return FString(TEXT("no interface"));
		}
		const auto Top = [](const UCommonActivatableWidgetStack* Layer)
		{
			const UCommonActivatableWidget* Screen = Layer ? Layer->GetActiveWidget() : nullptr;
			return Screen ? Screen->GetClass()->GetName() : FString(TEXT("none"));
		};
		return FString::Printf(TEXT("menu=%s modal=%s"), *Top(WeakThis->MenuLayer), *Top(WeakThis->ModalLayer));
	};
}

UCommonActivatableWidget* UHOLRootLayout::PushMenu(const TSubclassOf<UCommonActivatableWidget> ScreenClass)
{
	return MenuLayer ? MenuLayer->AddWidget(ScreenClass) : nullptr;
}

UCommonActivatableWidget* UHOLRootLayout::PushModal(const TSubclassOf<UCommonActivatableWidget> ScreenClass)
{
	return ModalLayer ? ModalLayer->AddWidget(ScreenClass) : nullptr;
}

UCommonActivatableWidget* UHOLRootLayout::GetActiveMenu() const
{
	return MenuLayer ? MenuLayer->GetActiveWidget() : nullptr;
}

bool UHOLRootLayout::HasMenuOtherThan(const UCommonActivatableWidget* Screen) const
{
	if (!MenuLayer)
	{
		return false;
	}
	for (const UCommonActivatableWidget* Widget : MenuLayer->GetWidgetList())
	{
		if (Widget && Widget != Screen)
		{
			return true;
		}
	}
	return false;
}

UCommonActivatableWidget* UHOLRootLayout::GetActiveModal() const
{
	return ModalLayer ? ModalLayer->GetActiveWidget() : nullptr;
}

UHOLRootLayout* UHOLRootLayout::Get(const APlayerController* PlayerController)
{
	if (const AHOLMenuPlayerController* MenuController = Cast<AHOLMenuPlayerController>(PlayerController))
	{
		return MenuController->GetRootLayout();
	}
	const AHOLGameHUD* GameHUD = PlayerController ? Cast<AHOLGameHUD>(PlayerController->GetHUD()) : nullptr;
	return GameHUD ? GameHUD->GetMenuLayout() : nullptr;
}

bool UHOLRootLayout::IsOverMatch() const
{
	return !Cast<AHOLMenuPlayerController>(GetOwningPlayer());
}
