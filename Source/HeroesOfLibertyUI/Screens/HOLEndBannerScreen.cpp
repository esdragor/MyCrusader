#include "Screens/HOLEndBannerScreen.h"

#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Engine/GameInstance.h"
#include "HeroesOfLibertyUILog.h"
#include "Kismet/GameplayStatics.h"
#include "Net/HOLManagerClient.h"
#include "Player/HOLGameMenuRules.h"
#include "Screens/HOLGameMenuScreen.h"
#include "Screens/HOLStatsScreen.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"

namespace UE::HeroesOfLiberty::UI::EndBanner
{
	/** The menu's map (DefaultEngine.ini, GameDefaultMap). */
	const TCHAR* const MainMenuMap = TEXT("/Game/Maps/MainMenu");
}

UHOLEndBannerScreen* UHOLEndBannerScreen::Open(APlayerController* PlayerController, const EHOLEndBanner InResult)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout || !Layout->IsOverMatch())
	{
		return nullptr;
	}
	const TSubclassOf<UHOLEndBannerScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("End banner: no script class (Script/UI/HOLEndBannerScreen.as)"));
		return nullptr;
	}
	// The in-game menu gives way (its confirmation with it): the banner is the answer.
	if (UCommonActivatableWidget* Modal = Layout->GetActiveModal())
	{
		Modal->DeactivateWidget();
	}
	if (UHOLGameMenuScreen* Menu = UHOLGameMenuScreen::Find(PlayerController))
	{
		Menu->DeactivateWidget();
	}
	UHOLEndBannerScreen* Banner = Cast<UHOLEndBannerScreen>(Layout->PushMenu(ScriptClass.Get()));
	if (Banner)
	{
		Banner->SetResult(InResult);
	}
	return Banner;
}

TSubclassOf<UHOLEndBannerScreen> UHOLEndBannerScreen::FindScriptClass()
{
	// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
		{
			return *It;
		}
	}
	return nullptr;
}

int32 UHOLEndBannerScreen::GetResultValue() const
{
	return static_cast<int32>(Result);
}

int32 UHOLEndBannerScreen::GetOriginValue() const
{
	return static_cast<int32>(Origin);
}

void UHOLEndBannerScreen::SetResult(const EHOLEndBanner InResult)
{
	Result = InResult;
	// Open pushes (and activates) the banner before giving it its result: the view follows the result.
	BP_RefreshBanner();
}

void UHOLEndBannerScreen::SetDetail(const FText& InDetail)
{
	if (Detail)
	{
		Detail->SetText(InDetail);
		Detail->SetVisibility(InDetail.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	if (!InDetail.IsEmpty())
	{
		UE_LOG(LogHOLUI, Display, TEXT("End banner: detail \"%s\""), *InDetail.ToString());
	}
}

void UHOLEndBannerScreen::Continue()
{
	if (!IsActivated())
	{
		return;
	}
	const EHOLEndBanner Closed = Result;
	DeactivateWidget();
	UE_LOG(LogHOLUI, Display, TEXT("End banner: closed"));
	OnClosedByPlayer.Broadcast(Closed);
}

void UHOLEndBannerScreen::ReturnToLobby()
{
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Client)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("End banner: back to lobby"));
	Client->SetAfterMatch(UHOLManagerClient::EAfterMatch::Lobby);
	LeaveMatch(GetOwningPlayer());
}

void UHOLEndBannerScreen::QuitToList()
{
	UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	if (!Client)
	{
		return;
	}
	UE_LOG(LogHOLUI, Display, TEXT("End banner: quit to list"));
	Client->ForgetLobby();
	Client->SetAfterMatch(UHOLManagerClient::EAfterMatch::List);
	LeaveMatch(GetOwningPlayer());
}

void UHOLEndBannerScreen::QuitToMenu()
{
	UE_LOG(LogHOLUI, Display, TEXT("End banner: quit to menu"));
	LeaveMatch(GetOwningPlayer());
}

void UHOLEndBannerScreen::ShowStats()
{
	UHOLStatsScreen::Open(GetOwningPlayer());
}

void UHOLEndBannerScreen::LeaveMatch(APlayerController* PlayerController)
{
	// The way back of UHOLNewGameScreen::Launch (OpenLevel): a local map, which ends this machine's network game, a
	// listen server's (its clients dropped) or a client's (D357).
	UGameplayStatics::OpenLevel(PlayerController, FName(UE::HeroesOfLiberty::UI::EndBanner::MainMenuMap));
}

UVerticalBox* UHOLEndBannerScreen::MakeBannerColumn()
{
	return MakeDialogColumn();
}

UCommonTextBlock* UHOLEndBannerScreen::AddBannerText(UVerticalBox* Column, const FName Name, const FText& Text, const bool bTitle)
{
	if (!Column)
	{
		return nullptr;
	}
	return bTitle ? AddText(Column, Name, Text, UHOLTextStyleTitle::StaticClass(), 24.f)
		: AddText(Column, Name, Text, UHOLTextStyleBody::StaticClass(), 16.f);
}

UHorizontalBox* UHOLEndBannerScreen::AddBannerRow(UVerticalBox* Column)
{
	return Column ? AddButtonRow(Column) : nullptr;
}

UHOLButton* UHOLEndBannerScreen::AddBannerButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	return Row ? AddButton(Row, Name, Label) : nullptr;
}

void UHOLEndBannerScreen::BuildLayout()
{
	BP_BuildBanner();
}

void UHOLEndBannerScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	UHOLGameMenuScreen::SetMatchMenuInput(GetOwningPlayer(), true, this);
	// From a lobby (R4.8): « Retour au salon » and « Quitter vers la liste », nobody sent back by himself (M6-Q7
	// revised); the spectator's banner (M6-Q10), from a lobby or the list's « Regarder », has no way back to a lobby he
	// is not in, but quits to the list as well.
	const UHOLManagerClient* Client = GetGameInstance() ? GetGameInstance()->GetSubsystem<UHOLManagerClient>() : nullptr;
	Origin = Client ? Client->GetMatchOrigin() : EHOLMatchOrigin::Local;
	BP_RefreshBanner();
}

void UHOLEndBannerScreen::NativeOnDeactivated()
{
	UHOLGameMenuScreen::SetMatchMenuInput(GetOwningPlayer(), false, this);
	Super::NativeOnDeactivated();
}

bool UHOLEndBannerScreen::NativeOnHandleBackAction()
{
	// Back is « Continuer à regarder ».
	Continue();
	return true;
}

UWidget* UHOLEndBannerScreen::NativeGetDesiredFocusTarget() const
{
	return FocusTarget;
}
