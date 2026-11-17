#include "Screens/HOLSettingsScreen.h"

#include "Blueprint/WidgetTree.h"
#include "CommonTextBlock.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/WidgetSwitcher.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyUILog.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/HOLGameUserSettings.h"
#include "Style/HOLUIStyles.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/HOLButton.h"
#include "Widgets/HOLRootLayout.h"
#include "Widgets/HOLRotator.h"

#define LOCTEXT_NAMESPACE "HOLSettingsScreen"

namespace UE::HeroesOfLiberty::UI::Settings
{
	constexpr float VideoLabelWidth = 320.f;
	constexpr float DisplayRotatorWidth = 340.f;
	constexpr float QualityRotatorWidth = 260.f;
	constexpr int32 NumLevels = 5;
	constexpr int32 CustomQuality = NumLevels;
	constexpr int32 NumVolumeSteps = 10;
	const FIntPoint MinResolution(1280, 720);

	/**
	 * A scalability group of the Video tab. No Shadows group: the game has none (D258); no Reflections nor
	 * Post-processing group: no reflection and no post-processing effect (D265).
	 */
	struct FGroup
	{
		const TCHAR* Name;
		FText Label;
		int32 (UGameUserSettings::*Get)() const;
		void (UGameUserSettings::*Set)(int32);
	};

	TArray<FGroup> Groups()
	{
		return {
			{ TEXT("ViewDistance"), LOCTEXT("ViewDistance", "Distance d'affichage"), &UGameUserSettings::GetViewDistanceQuality, &UGameUserSettings::SetViewDistanceQuality },
			{ TEXT("AntiAliasing"), LOCTEXT("AntiAliasing", "Anticrénelage"), &UGameUserSettings::GetAntiAliasingQuality, &UGameUserSettings::SetAntiAliasingQuality },
			{ TEXT("GlobalIllumination"), LOCTEXT("GlobalIllumination", "Éclairage global"), &UGameUserSettings::GetGlobalIlluminationQuality, &UGameUserSettings::SetGlobalIlluminationQuality },
			{ TEXT("Texture"), LOCTEXT("Texture", "Textures"), &UGameUserSettings::GetTextureQuality, &UGameUserSettings::SetTextureQuality },
			{ TEXT("VisualEffect"), LOCTEXT("VisualEffect", "Effets"), &UGameUserSettings::GetVisualEffectQuality, &UGameUserSettings::SetVisualEffectQuality },
			{ TEXT("Foliage"), LOCTEXT("Foliage", "Végétation"), &UGameUserSettings::GetFoliageQuality, &UGameUserSettings::SetFoliageQuality },
			{ TEXT("Shading"), LOCTEXT("Shading", "Matériaux"), &UGameUserSettings::GetShadingQuality, &UGameUserSettings::SetShadingQuality },
			{ TEXT("Landscape"), LOCTEXT("Landscape", "Terrain"), &UGameUserSettings::GetLandscapeQuality, &UGameUserSettings::SetLandscapeQuality },
		};
	}

	TArray<FText> LevelNames()
	{
		return { LOCTEXT("Low", "Basse"), LOCTEXT("Medium", "Moyenne"), LOCTEXT("High", "Haute"), LOCTEXT("Epic", "Épique"),
			LOCTEXT("Cinematic", "Cinématique") };
	}

	/** Volumes in the order of EHOLVolume. */
	TArray<FText> VolumeNames()
	{
		return { LOCTEXT("MasterVolume", "Volume général"), LOCTEXT("MusicVolume", "Musique"), LOCTEXT("EffectsVolume", "Effets"),
			LOCTEXT("VoiceVolume", "Voix") };
	}

	/** Languages of D245, by their culture code; shown in their own language, never translated. */
	const TCHAR* const Languages[] = { TEXT("fr"), TEXT("en"), TEXT("ja") };
	TArray<FText> LanguageNames()
	{
		return { INVTEXT("Français"), INVTEXT("English"), INVTEXT("日本語") };
	}

	TArray<FText> OffOn()
	{
		return { LOCTEXT("Off", "Non"), LOCTEXT("On", "Oui") };
	}

	/** Index of the value nearest to Value. */
	int32 Nearest(const TArray<float>& Values, const float Value)
	{
		int32 Best = 0;
		for (int32 Index = 1; Index < Values.Num(); ++Index)
		{
			if (FMath::Abs(Values[Index] - Value) < FMath::Abs(Values[Best] - Value))
			{
				Best = Index;
			}
		}
		return Best;
	}

	/**
	 * The resolution and window mode of the game window, or the ones about to be applied: r.SetRes, set from the command
	 * line at start-up (-windowed, -ResX...) and by every change. The saved settings can differ from them.
	 */
	bool GetShownDisplay(FIntPoint& OutResolution, EWindowMode::Type& OutWindowMode)
	{
		const IConsoleVariable* SetRes = IConsoleManager::Get().FindConsoleVariable(TEXT("r.SetRes"));
		FString Width, Rest;
		if (!SetRes || !SetRes->GetString().Split(TEXT("x"), &Width, &Rest))
		{
			return false;
		}
		int32 Digits = 0;
		while (Digits < Rest.Len() && FChar::IsDigit(Rest[Digits]))
		{
			++Digits;
		}
		const FString Suffix = Rest.Mid(Digits);
		OutResolution = FIntPoint(FCString::Atoi(*Width), FCString::Atoi(*Rest.Left(Digits)));
		OutWindowMode = Suffix.Equals(TEXT("f"), ESearchCase::IgnoreCase) ? EWindowMode::Fullscreen
			: Suffix.Equals(TEXT("wf"), ESearchCase::IgnoreCase) ? EWindowMode::WindowedFullscreen
			: EWindowMode::Windowed;
		return OutResolution.X > 0 && OutResolution.Y > 0;
	}

	/** The shown display, or the saved one when there is no game window. */
	void GetDisplay(const UGameUserSettings& Settings, FIntPoint& OutResolution, EWindowMode::Type& OutWindowMode)
	{
		if (!GetShownDisplay(OutResolution, OutWindowMode))
		{
			OutResolution = Settings.GetScreenResolution();
			OutWindowMode = Settings.GetFullscreenMode();
		}
	}
}

namespace UE::HeroesOfLiberty::UI::Settings
{
	const TArray<float>& FrameRateValues()
	{
		static const TArray<float> Values = { 30.f, 60.f, 120.f, 144.f, 240.f, 0.f };
		return Values;
	}

	const TArray<float>& RenderScaleValues()
	{
		static const TArray<float> Values = { 50.f, 60.f, 70.f, 80.f, 90.f, 100.f };
		return Values;
	}
}

UHOLSettingsScreen* UHOLSettingsScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	if (!Layout)
	{
		return nullptr;
	}
	const TSubclassOf<UHOLSettingsScreen> ScriptClass = FindScriptClass();
	if (!ScriptClass)
	{
		UE_LOG(LogHOLUI, Warning, TEXT("Settings: no script class (Script/UI/HOLSettingsScreen.as)"));
		return nullptr;
	}
	return Cast<UHOLSettingsScreen>(Layout->PushMenu(ScriptClass));
}

UHOLSettingsScreen* UHOLSettingsScreen::Find(APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLSettingsScreen>(Layout->GetActiveMenu()) : nullptr;
}

TSubclassOf<UHOLSettingsScreen> UHOLSettingsScreen::FindScriptClass()
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

void UHOLSettingsScreen::BuildLayout()
{
	BP_BuildSettings();
}

void UHOLSettingsScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SelectTab(0);
	BP_RefreshSettings();
}

UWidget* UHOLSettingsScreen::NativeGetDesiredFocusTarget() const
{
	return VideoTab;
}

void UHOLSettingsScreen::GetColumns(TArray<TArray<UWidget*>>& OutColumns) const
{
	if (!TabPages || TabPages->GetActiveWidgetIndex() != 0)
	{
		return;
	}
	OutColumns.Add({ ResolutionRotator, WindowModeRotator, VSyncRotator, FrameRateRotator, RenderScaleRotator });
	TArray<UWidget*>& Quality = OutColumns.Add_GetRef({ QualityRotator });
	Quality.Append(GroupRotators);
}

void UHOLSettingsScreen::SelectTab(const int32 Index)
{
	if (TabPages)
	{
		TabPages->SetActiveWidgetIndex(Index);
	}
	UHOLButton* const Tabs[] = { VideoTab, SoundTab, ControlsTab, LanguageTab };
	for (int32 Tab = 0; Tab < UE_ARRAY_COUNT(Tabs); ++Tab)
	{
		if (Tabs[Tab])
		{
			Tabs[Tab]->SetIsSelected(Tab == Index, /*bGiveClickFeedback*/ false);
		}
	}
}

int32 UHOLSettingsScreen::GetActiveTab() const
{
	return TabPages ? TabPages->GetActiveWidgetIndex() : -1;
}

void UHOLSettingsScreen::CloseSettings()
{
	Close();
}

UVerticalBox* UHOLSettingsScreen::MakeSettingsColumn()
{
	return MakeCenteredColumn();
}

UCommonTextBlock* UHOLSettingsScreen::AddSettingsText(UVerticalBox* Column, const FName Name, const FText& Text, const int32 Style)
{
	if (!Column)
	{
		return nullptr;
	}
	return Style == 0 ? AddText(Column, Name, Text, UHOLTextStyleHeading::StaticClass(), 16.f)
		: AddText(Column, Name, Text, UHOLTextStyleNote::StaticClass(), 0.f);
}

UHorizontalBox* UHOLSettingsScreen::AddSettingsRow(UVerticalBox* Column)
{
	return Column ? AddButtonRow(Column) : nullptr;
}

UHOLButton* UHOLSettingsScreen::AddSettingsButton(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	return Row ? AddButton(Row, Name, Label) : nullptr;
}

UHOLButton* UHOLSettingsScreen::AddSettingsPageButton(UVerticalBox* Page, const FName Name, const FText& Label)
{
	return Page ? AddButton(Page, Name, Label) : nullptr;
}

UHOLButton* UHOLSettingsScreen::AddSettingsTab(UHorizontalBox* Row, const FName Name, const FText& Label)
{
	UHOLButton* Button = Row ? AddButton(Row, Name, Label) : nullptr;
	if (Button)
	{
		// Toggleable, or a selected button could never be deselected (the click selects it again, by SelectTab).
		Button->SetIsSelectable(true);
		Button->SetIsToggleable(true);
	}
	return Button;
}

UWidgetSwitcher* UHOLSettingsScreen::AddSettingsPages(UVerticalBox* Column)
{
	if (!Column)
	{
		return nullptr;
	}
	TabPages = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("TabPages"));
	UVerticalBoxSlot* PagesSlot = Column->AddChildToVerticalBox(TabPages);
	PagesSlot->SetHorizontalAlignment(HAlign_Center);
	PagesSlot->SetPadding(FMargin(0.f, 24.f, 0.f, 0.f));
	return TabPages;
}

UVerticalBox* UHOLSettingsScreen::AddSettingsPage()
{
	if (!TabPages)
	{
		return nullptr;
	}
	UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	TabPages->AddChild(Page);
	return Page;
}

UHorizontalBox* UHOLSettingsScreen::AddSettingsPair(UVerticalBox* Page)
{
	if (!Page)
	{
		return nullptr;
	}
	UHorizontalBox* Pair = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Page->AddChildToVerticalBox(Pair)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
	return Pair;
}

UVerticalBox* UHOLSettingsScreen::AddSettingsPairColumn(UHorizontalBox* Pair, const bool bLast)
{
	if (!Pair)
	{
		return nullptr;
	}
	UVerticalBox* Column = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UHorizontalBoxSlot* ColumnSlot = Pair->AddChildToHorizontalBox(Column);
	if (!bLast)
	{
		ColumnSlot->SetPadding(FMargin(0.f, 0.f, 24.f, 0.f));
	}
	return Column;
}

UCommonTextBlock* UHOLSettingsScreen::AddSettingsHint(UVerticalBox* Page)
{
	return Page ? AddColumnHint(Page) : nullptr;
}

UHOLRotator* UHOLSettingsScreen::AddSettingsRotator(UVerticalBox* Column, const FName Name, const FText& Label, const TArray<FText>& Options,
	const int32 Index, const int32 Layout)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (!Column)
	{
		return nullptr;
	}
	UHOLRotator* Rotator = nullptr;
	if (Layout == 1)
	{
		Rotator = AddRotatorRow(Column, Name, Label, Options, Index, VideoLabelWidth, DisplayRotatorWidth);
	}
	else if (Layout == 2)
	{
		Rotator = AddRotatorRow(Column, Name, Label, Options, Index, VideoLabelWidth, QualityRotatorWidth);
	}
	else
	{
		Rotator = AddRotatorRow(Column, Name, Label, Options, Index);
	}
	if (Rotator)
	{
		// Bound after the first value is shown, so that showing it is not a change; every later change, shown or not, goes to the script.
		Rotator->OnRotatedEvent.AddWeakLambda(this, [this, Name](const int32 Value, bool) { BP_SettingChanged(Name, Value); });
	}
	return Rotator;
}

FText UHOLSettingsScreen::TextOf(const FName Key)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const TMap<FName, FText> Texts = {
		{ TEXT("Heading"), LOCTEXT("Heading", "Réglages") },
		{ TEXT("Video"), LOCTEXT("Video", "Vidéo") },
		{ TEXT("Sound"), LOCTEXT("Sound", "Son") },
		{ TEXT("Controls"), LOCTEXT("Controls", "Commandes") },
		{ TEXT("Language"), LOCTEXT("Language", "Langue") },
		{ TEXT("LanguageLabel"), LOCTEXT("LanguageLabel", "Langue") },
		{ TEXT("Back"), LOCTEXT("Back", "Retour") },
		{ TEXT("Resolution"), LOCTEXT("Resolution", "Résolution") },
		{ TEXT("WindowMode"), LOCTEXT("WindowMode", "Affichage") },
		{ TEXT("VSync"), LOCTEXT("VSync", "Synchronisation verticale") },
		{ TEXT("FrameRate"), LOCTEXT("FrameRate", "Images par seconde") },
		{ TEXT("RenderScale"), LOCTEXT("RenderScale", "Échelle de rendu") },
		{ TEXT("Quality"), LOCTEXT("Quality", "Qualité") },
		{ TEXT("SoundNote"), LOCTEXT("SoundNote", "Musique, effets et voix agiront dès que le jeu aura des sons.") },
		{ TEXT("Remap"), LOCTEXT("Remap", "Changer les touches") },
		{ TEXT("RemapNote"), LOCTEXT("RemapNote", "Changer les touches : pas encore dans le jeu.") },
		{ TEXT("CameraSpeed"), LOCTEXT("CameraSpeed", "Vitesse de la caméra") },
		{ TEXT("EdgeScroll"), LOCTEXT("EdgeScroll", "Défilement au bord de l'écran") },
	};
	if (const FText* Found = Texts.Find(Key))
	{
		return *Found;
	}
	const TArray<FGroup> AllGroups = Groups();
	for (const FGroup& Group : AllGroups)
	{
		if (Key == FName(Group.Name))
		{
			return Group.Label;
		}
	}
	const TArray<FText> Volumes = VolumeNames();
	for (int32 Volume = 0; Volume < Volumes.Num(); ++Volume)
	{
		if (Key == FName(FString::Printf(TEXT("Volume_%s"), *StaticEnum<EHOLVolume>()->GetNameStringByIndex(Volume))))
		{
			return Volumes[Volume];
		}
	}
	UE_LOG(LogHOLUI, Warning, TEXT("Settings: no text for the key %s"), *Key.ToString());
	return FText::FromName(Key);
}

TArray<FText> UHOLSettingsScreen::OptionsOf(const FName Kind)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (Kind == FName(TEXT("WindowMode")))
	{
		return { LOCTEXT("Fullscreen", "Plein écran"), LOCTEXT("WindowedFullscreen", "Fenêtre sans bordure"), LOCTEXT("Windowed", "Fenêtre") };
	}
	if (Kind == FName(TEXT("OffOn")))
	{
		return OffOn();
	}
	if (Kind == FName(TEXT("FrameRate")))
	{
		TArray<FText> Names;
		for (const float Rate : FrameRateValues())
		{
			Names.Add(Rate > 0.f ? FText::AsNumber(FMath::RoundToInt(Rate)) : LOCTEXT("Unlimited", "Illimitée"));
		}
		return Names;
	}
	if (Kind == FName(TEXT("RenderScale")))
	{
		TArray<FText> Names;
		for (const float Scale : RenderScaleValues())
		{
			Names.Add(FText::AsPercent(Scale / 100.f));
		}
		return Names;
	}
	if (Kind == FName(TEXT("Level")))
	{
		return LevelNames();
	}
	if (Kind == FName(TEXT("Quality")))
	{
		TArray<FText> Names = LevelNames();
		Names.Add(LOCTEXT("Custom", "Personnalisée"));
		return Names;
	}
	if (Kind == FName(TEXT("Volume")))
	{
		TArray<FText> Steps;
		for (int32 Step = 0; Step <= NumVolumeSteps; ++Step)
		{
			Steps.Add(FText::AsPercent(static_cast<float>(Step) / NumVolumeSteps));
		}
		return Steps;
	}
	if (Kind == FName(TEXT("CameraSpeed")))
	{
		return { LOCTEXT("Slow", "Lente"), LOCTEXT("Normal", "Normale"), LOCTEXT("Fast", "Rapide") };
	}
	if (Kind == FName(TEXT("Language")))
	{
		return LanguageNames();
	}
	UE_LOG(LogHOLUI, Warning, TEXT("Settings: no values for the kind %s"), *Kind.ToString());
	return {};
}

TArray<FText> UHOLSettingsScreen::GetResolutionNames()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	Resolutions.Reset();
	TArray<FIntPoint> Supported;
	UKismetSystemLibrary::GetSupportedFullscreenResolutions(Supported);
	for (const FIntPoint& Resolution : Supported)
	{
		if (Resolution.X >= MinResolution.X && Resolution.Y >= MinResolution.Y)
		{
			Resolutions.AddUnique(Resolution);
		}
	}
	if (const UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings())
	{
		FIntPoint Resolution;
		EWindowMode::Type WindowMode;
		GetDisplay(*GameSettings, Resolution, WindowMode);
		Resolutions.AddUnique(Resolution);
	}
	Resolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X < B.X : A.Y < B.Y; });
	TArray<FText> Names;
	for (const FIntPoint& Resolution : Resolutions)
	{
		Names.Add(FText::Format(INVTEXT("{0} × {1}"), FText::AsNumber(Resolution.X, &FNumberFormattingOptions::DefaultNoGrouping()),
			FText::AsNumber(Resolution.Y, &FNumberFormattingOptions::DefaultNoGrouping())));
	}
	return Names;
}

int32 UHOLSettingsScreen::GetShownResolutionIndex() const
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings)
	{
		return 0;
	}
	FIntPoint Resolution;
	EWindowMode::Type WindowMode;
	GetDisplay(*Settings, Resolution, WindowMode);
	return FMath::Max(0, Resolutions.IndexOfByKey(Resolution));
}

int32 UHOLSettingsScreen::GetShownWindowMode()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings)
	{
		return 0;
	}
	FIntPoint Resolution;
	EWindowMode::Type WindowMode;
	GetDisplay(*Settings, Resolution, WindowMode);
	return static_cast<int32>(WindowMode);
}

void UHOLSettingsScreen::ChooseResolution(const int32 Index)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (Settings && Resolutions.IsValidIndex(Index))
	{
		// With the window mode shown, not the saved one, which the command line may have overridden.
		FIntPoint Shown;
		EWindowMode::Type WindowMode;
		GetDisplay(*Settings, Shown, WindowMode);
		Settings->SetFullscreenMode(WindowMode);
		Settings->SetScreenResolution(Resolutions[Index]);
	}
}

void UHOLSettingsScreen::ChooseWindowMode(const int32 Mode)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		// With the resolution shown, as above.
		FIntPoint Resolution;
		EWindowMode::Type Shown;
		GetDisplay(*Settings, Resolution, Shown);
		Settings->SetScreenResolution(Resolution);
		Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(FMath::Clamp(Mode, 0, 2)));
	}
}

int32 UHOLSettingsScreen::GetFrameRateIndex()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings)
	{
		return 0;
	}
	const float Limit = Settings->GetFrameRateLimit();
	return Limit > 0.f ? Nearest(FrameRateValues(), Limit) : FrameRateValues().Num() - 1;
}

void UHOLSettingsScreen::SetFrameRateIndex(const int32 Index)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (Settings && FrameRateValues().IsValidIndex(Index))
	{
		Settings->SetFrameRateLimit(FrameRateValues()[Index]);
	}
}

int32 UHOLSettingsScreen::GetRenderScaleIndex()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings)
	{
		return 0;
	}
	float Normalized, Value, Min, Max;
	Settings->GetResolutionScaleInformationEx(Normalized, Value, Min, Max);
	return Nearest(RenderScaleValues(), Value);
}

void UHOLSettingsScreen::SetRenderScaleIndex(const int32 Index)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (Settings && RenderScaleValues().IsValidIndex(Index))
	{
		Settings->SetResolutionScaleValueEx(RenderScaleValues()[Index]);
	}
}

int32 UHOLSettingsScreen::GetQualityIndex()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	const int32 Overall = Settings ? Settings->GetOverallScalabilityLevel() : -1;
	return Overall >= 0 && Overall < NumLevels ? Overall : CustomQuality;
}

int32 UHOLSettingsScreen::ResolveQualityChoice(const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	int32 Level = Value;
	if (Level == CustomQuality)
	{
		// "Custom" only shows that the groups differ; it cannot be chosen.
		const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
		const int32 Overall = Settings ? Settings->GetOverallScalabilityLevel() : -1;
		const int32 Previous = Overall >= 0 ? Overall : CustomQuality;
		Level = (Level + UHOLRotator::GetStep(Previous, Value, NumLevels + 1) + NumLevels + 1) % (NumLevels + 1);
	}
	return Level;
}

int32 UHOLSettingsScreen::GetGroupLevel(const int32 Group)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const TArray<FGroup> AllGroups = Groups();
	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings || !AllGroups.IsValidIndex(Group))
	{
		return 0;
	}
	return FMath::Clamp((Settings->*AllGroups[Group].Get)(), 0, NumLevels - 1);
}

void UHOLSettingsScreen::SetGroupLevel(const int32 Group, const int32 Level)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const TArray<FGroup> AllGroups = Groups();
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get(); Settings && AllGroups.IsValidIndex(Group))
	{
		(Settings->*AllGroups[Group].Set)(Level);
	}
}

int32 UHOLSettingsScreen::GetLanguageIndex()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const FString Current = UKismetInternationalizationLibrary::GetCurrentLanguage();
	int32 Selected = 0;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Languages); ++Index)
	{
		if (Current.StartsWith(Languages[Index]))
		{
			Selected = Index;
		}
	}
	return Selected;
}

void UHOLSettingsScreen::SetLanguageIndex(const int32 Index)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (Index >= 0 && Index < UE_ARRAY_COUNT(Languages))
	{
		// Switched live and saved by the engine ([Internationalization] in GameUserSettings.ini).
		const bool bSet = UKismetInternationalizationLibrary::SetCurrentLanguage(Languages[Index], /*SaveToConfig*/ true);
		UE_LOG(LogHOLUI, Display, TEXT("Settings: language %s%s"), Languages[Index], bSet ? TEXT("") : TEXT(" refused"));
	}
}

#undef LOCTEXT_NAMESPACE
