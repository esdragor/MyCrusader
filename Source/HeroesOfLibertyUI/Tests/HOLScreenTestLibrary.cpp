// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tests/HOLScreenTestLibrary.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Widgets/HOLDropdown.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Text.h"
#include "Internationalization/TextKey.h"
#include "Internationalization/TextLocalizationResource.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/StrongObjectPtr.h"

DEFINE_LOG_CATEGORY_STATIC(LogHOLScreenTest, Log, All);

namespace HOLScreenTestLibNs
{
	// What CreateScreenForTest made, kept alive (strong pointers) until EndScreenTest. Prefixed names: unity build.
	TStrongObjectPtr<UWorld> HOLScreenTestLibWorld;
	TStrongObjectPtr<ULocalPlayer> HOLScreenTestLibPlayer;
	TStrongObjectPtr<UUserWidget> HOLScreenTestLibScreen;
}
#endif

UUserWidget* UHOLScreenTestLibrary::CreateScreenForTest(TSubclassOf<UUserWidget> ScreenClass)
{
#if WITH_DEV_AUTOMATION_TESTS
	EndScreenTest();
	if (!ScreenClass)
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: no screen class given."));
		return nullptr;
	}
	if (!GEngine)
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: no GEngine."));
		return nullptr;
	}
	// The set up of HOLNewGameSitesBoxTests.cpp: a game world, its context, a local player and its controller.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	if (!World)
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: UWorld::CreateWorld returned nullptr."));
		return nullptr;
	}
	HOLScreenTestLibNs::HOLScreenTestLibWorld.Reset(World);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ULocalPlayer* Player = NewObject<ULocalPlayer>(GEngine);
	HOLScreenTestLibNs::HOLScreenTestLibPlayer.Reset(Player);
	APlayerController* Controller = World->SpawnActor<APlayerController>();
	if (!Controller)
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: the player controller could not be spawned."));
		EndScreenTest();
		return nullptr;
	}
	Controller->Player = Player;
	Player->PlayerController = Controller;
	if (!World->GetPlayerControllerIterator())
	{
		World->AddController(Controller);
	}
	UUserWidget* Screen = NewObject<UUserWidget>(GetTransientPackage(), ScreenClass);
	if (!Screen)
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: the widget of %s could not be created."), *ScreenClass->GetName());
		EndScreenTest();
		return nullptr;
	}
	HOLScreenTestLibNs::HOLScreenTestLibScreen.Reset(Screen);
	Screen->SetPlayerContext(FLocalPlayerContext(Player, World));
	if (!Screen->GetPlayerContext().IsValid())
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: the screen has no valid player context."));
		EndScreenTest();
		return nullptr;
	}
	// UUserWidget::Initialize builds the widget tree and plays NativeOnInitialized (the layout) only with a player.
	Screen->Initialize();
	if (!Screen->WidgetTree)
	{
		UE_LOG(LogHOLScreenTest, Error, TEXT("CreateScreenForTest: %s has no widget tree after Initialize."), *ScreenClass->GetName());
		EndScreenTest();
		return nullptr;
	}
	return Screen;
#else
	return nullptr;
#endif
}

void UHOLScreenTestLibrary::EndScreenTest()
{
#if WITH_DEV_AUTOMATION_TESTS
	HOLScreenTestLibNs::HOLScreenTestLibScreen.Reset();
	HOLScreenTestLibNs::HOLScreenTestLibPlayer.Reset();
	UWorld* World = HOLScreenTestLibNs::HOLScreenTestLibWorld.Get();
	HOLScreenTestLibNs::HOLScreenTestLibWorld.Reset();
	if (World && GEngine)
	{
		// As the end of the C++ test does.
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	}
#endif
}

void UHOLScreenTestLibrary::DestroyScreenForTest(UUserWidget* Screen)
{
	EndScreenTest();
}

UWidget* UHOLScreenTestLibrary::FindWidgetByName(UUserWidget* Screen, const FName Name)
{
	if (!Screen || !Screen->WidgetTree)
	{
		return nullptr;
	}
	return Screen->WidgetTree->FindWidget(Name);
}

bool UHOLScreenTestLibrary::IsWidgetEnabled(UWidget* Widget)
{
	return Widget && Widget->GetIsEnabled();
}

FText UHOLScreenTestLibrary::GetDropdownText(UWidget* Widget)
{
	const UHOLDropdown* Dropdown = Cast<UHOLDropdown>(Widget);
	return Dropdown ? Dropdown->GetShownText() : FText::GetEmpty();
}

int32 UHOLScreenTestLibrary::GetDropdownIndex(UWidget* Widget)
{
	const UHOLDropdown* Dropdown = Cast<UHOLDropdown>(Widget);
	return Dropdown ? Dropdown->GetSelectedIndex() : -1;
}

FString UHOLScreenTestLibrary::GetLocalizedTextIn(const FString& Culture, const FText& Shown)
{
#if WITH_DEV_AUTOMATION_TESTS
	// The same way as HOLScenarioPanelTests.cpp (TranslationOf): the culture's Game.locres, the entry of the text's own namespace and key.
	FTextLocalizationResource Resource;
	const FString File = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Localization/Game"), Culture, TEXT("Game.locres"));
	if (!Resource.LoadFromFile(File, 0))
	{
		UE_LOG(LogHOLScreenTest, Warning, TEXT("GetLocalizedTextIn: %s is not readable"), *File);
		return FString();
	}
	const auto Translate = [&Resource](const FText& Text, FString& OutTranslation)
	{
		const TOptional<FString> Namespace = FTextInspector::GetNamespace(Text);
		const TOptional<FString> Key = FTextInspector::GetKey(Text);
		if (!Namespace.IsSet() || !Key.IsSet())
		{
			return false;
		}
		const FTextLocalizationResource::FEntry* Entry = Resource.Entries.Find(FTextId(FTextKey(*Namespace.GetValue()), FTextKey(*Key.GetValue())));
		if (!Entry || !Entry->LocalizedString.IsValid())
		{
			return false;
		}
		OutTranslation = *Entry->LocalizedString;
		return true;
	};
	FString Translated;
	if (Translate(Shown, Translated))
	{
		return Translated;
	}
	// A formatted text: its keyed pattern translated, its arguments kept (the title "Scenario {0}").
	TArray<FHistoricTextFormatData> Formats;
	FTextInspector::GetHistoricFormatData(Shown, Formats);
	for (const FHistoricTextFormatData& Data : Formats)
	{
		if (Translate(Data.SourceFmt.GetSourceText(), Translated))
		{
			return FText::Format(FTextFormat::FromString(Translated), Data.Arguments).ToString();
		}
	}
#endif
	return FString();
}
