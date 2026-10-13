// Copyright Epic Games, Inc. All Rights Reserved.

#include "Dom/JsonObject.h"
#include "HAL/FileManager.h"
#include "Internationalization/TextLocalizationResource.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Settings/HOLGameUserSettings.h"

#if WITH_DEV_AUTOMATION_TESTS

// Front-end checks (D246): the saved settings and the translations of the three languages (D245).
namespace UE::HeroesOfLiberty::Tests::FrontEnd
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** "Namespace,Key" -> text, from a manifest (source texts) or an archive (translations) of the Game target. */
	using FTexts = TMap<FString, FString>;

	FString LocalizationDir()
	{
		return FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Localization"), TEXT("Game"));
	}

	TSharedPtr<FJsonObject> ReadJson(const FString& Path)
	{
		FString Json;
		TSharedPtr<FJsonObject> Object;
		if (FFileHelper::LoadFileToString(Json, *Path))
		{
			FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Object);
		}
		return Object;
	}

	/**
	 * Walks the namespaces of a manifest or an archive. A manifest entry has a source text and several keys; an
	 * archive entry has one key and a translation.
	 */
	void CollectTexts(const TSharedPtr<FJsonObject>& Node, const FString& Namespace, const bool bArchive, FTexts& OutTexts)
	{
		const TArray<TSharedPtr<FJsonValue>>* Children = nullptr;
		if (Node->TryGetArrayField(TEXT("Children"), Children))
		{
			for (const TSharedPtr<FJsonValue>& Child : *Children)
			{
				const TSharedPtr<FJsonObject> Entry = Child->AsObject();
				if (bArchive)
				{
					const FString Text = Entry->GetObjectField(TEXT("Translation"))->GetStringField(TEXT("Text"));
					OutTexts.Add(Namespace + TEXT(",") + Entry->GetStringField(TEXT("Key")), Text);
				}
				else
				{
					const FString Text = Entry->GetObjectField(TEXT("Source"))->GetStringField(TEXT("Text"));
					for (const TSharedPtr<FJsonValue>& Key : Entry->GetArrayField(TEXT("Keys")))
					{
						OutTexts.Add(Namespace + TEXT(",") + Key->AsObject()->GetStringField(TEXT("Key")), Text);
					}
				}
			}
		}
		const TArray<TSharedPtr<FJsonValue>>* Subnamespaces = nullptr;
		if (Node->TryGetArrayField(TEXT("Subnamespaces"), Subnamespaces))
		{
			for (const TSharedPtr<FJsonValue>& Sub : *Subnamespaces)
			{
				const TSharedPtr<FJsonObject> SubObject = Sub->AsObject();
				CollectTexts(SubObject, SubObject->GetStringField(TEXT("Namespace")), bArchive, OutTexts);
			}
		}
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLSettingsVolumesTest, "HeroesOfLiberty.Settings.Volumes", UE::HeroesOfLiberty::Tests::FrontEnd::Flags)
bool FHOLSettingsVolumesTest::RunTest(const FString& Parameters)
{
	UHOLGameUserSettings* Written = NewObject<UHOLGameUserSettings>();
	Written->SetToDefaults();
	for (const EHOLVolume Volume : { EHOLVolume::Master, EHOLVolume::Music, EHOLVolume::Effects, EHOLVolume::Voice })
	{
		TestEqual(TEXT("full volume by default"), Written->GetVolume(Volume), 1.f);
	}
	Written->SetVolume(EHOLVolume::Master, 1.5f);
	Written->SetVolume(EHOLVolume::Music, 0.3f);
	Written->SetVolume(EHOLVolume::Effects, 0.7f);
	Written->SetVolume(EHOLVolume::Voice, -1.f);
	TestEqual(TEXT("above 1 brought back to 1"), Written->GetVolume(EHOLVolume::Master), 1.f);
	TestEqual(TEXT("below 0 brought back to 0"), Written->GetVolume(EHOLVolume::Voice), 0.f);

	// Saved then read back through a file of its own, never the player's GameUserSettings.ini.
	const FString Ini = FConfigCacheIni::NormalizeConfigIniPath(
		FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Automation"), TEXT("HOLSettingsVolumes.ini")));
	IFileManager::Get().Delete(*Ini);
	Written->SaveConfig(CPF_Config, *Ini);
	UHOLGameUserSettings* Read = NewObject<UHOLGameUserSettings>();
	Read->LoadConfig(nullptr, *Ini);
	TestEqual(TEXT("music read back"), Read->GetVolume(EHOLVolume::Music), 0.3f, UE_KINDA_SMALL_NUMBER);
	TestEqual(TEXT("effects read back"), Read->GetVolume(EHOLVolume::Effects), 0.7f, UE_KINDA_SMALL_NUMBER);
	TestEqual(TEXT("voice read back"), Read->GetVolume(EHOLVolume::Voice), 0.f);
	GConfig->UnloadFile(Ini);
	IFileManager::Get().Delete(*Ini);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLLocalizationTranslationsTest, "HeroesOfLiberty.Localization.Translations", UE::HeroesOfLiberty::Tests::FrontEnd::Flags)
bool FHOLLocalizationTranslationsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::FrontEnd;

	const FString Dir = LocalizationDir();
	const TSharedPtr<FJsonObject> Manifest = ReadJson(FPaths::Combine(Dir, TEXT("Game.manifest")));
	if (!TestNotNull(TEXT("manifest of the Game target (gather it: Config/Localization/Game.ini)"), Manifest.Get()))
	{
		return false;
	}
	FTexts Sources;
	CollectTexts(Manifest, FString(), /*bArchive*/ false, Sources);
	TestTrue(TEXT("texts gathered"), Sources.Num() > 0);

	// French is the source language (D245); English and Japanese must translate every text.
	for (const TCHAR* Culture : { TEXT("fr"), TEXT("en"), TEXT("ja") })
	{
		const bool bNative = FCString::Strcmp(Culture, TEXT("fr")) == 0;
		const TSharedPtr<FJsonObject> Archive = ReadJson(FPaths::Combine(Dir, Culture, TEXT("Game.archive")));
		if (!TestNotNull(FString::Printf(TEXT("archive %s"), Culture), Archive.Get()))
		{
			continue;
		}
		FTexts Translations;
		CollectTexts(Archive, FString(), /*bArchive*/ true, Translations);

		FTextLocalizationResource Compiled;
		TestTrue(FString::Printf(TEXT("compiled %s.locres read"), Culture),
			Compiled.LoadFromFile(FPaths::Combine(Dir, Culture, TEXT("Game.locres")), 0));

		int32 Missing = 0;
		for (const TPair<FString, FString>& Source : Sources)
		{
			const FString* Translation = Translations.Find(Source.Key);
			if (!Translation || Translation->IsEmpty())
			{
				AddError(FString::Printf(TEXT("%s: no translation for %s (\"%s\"), see Tools/Localization/fill_po.py"), Culture, *Source.Key, *Source.Value));
				++Missing;
				continue;
			}
			if (bNative && *Translation != Source.Value)
			{
				AddError(FString::Printf(TEXT("%s: %s differs from its source text"), Culture, *Source.Key));
			}

			FString Namespace, Key;
			Source.Key.Split(TEXT(","), &Namespace, &Key);
			const FTextLocalizationResource::FEntry* Entry = Compiled.Entries.Find(FTextId(FTextKey(Namespace), FTextKey(Key)));
			if (!Entry || !Entry->LocalizedString || *Entry->LocalizedString != *Translation)
			{
				AddError(FString::Printf(TEXT("%s: %s not compiled, or compiled from an older translation (gather again)"), Culture, *Source.Key));
			}
		}
		AddInfo(FString::Printf(TEXT("%s: %d texts, %d missing"), Culture, Sources.Num(), Missing));
	}
	return true;
}

#endif
