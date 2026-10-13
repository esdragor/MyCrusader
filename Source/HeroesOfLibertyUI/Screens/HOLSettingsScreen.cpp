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

UHOLSettingsScreen* UHOLSettingsScreen::Open(APlayerController* PlayerController)
{
	UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLSettingsScreen>(Layout->PushMenu(StaticClass())) : nullptr;
}

UHOLSettingsScreen* UHOLSettingsScreen::Find(APlayerController* PlayerController)
{
	const UHOLRootLayout* Layout = UHOLRootLayout::Get(PlayerController);
	return Layout ? Cast<UHOLSettingsScreen>(Layout->GetActiveMenu()) : nullptr;
}

void UHOLSettingsScreen::BuildLayout()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	// The values offered, gathered once.
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
	FrameRates = { 30.f, 60.f, 120.f, 144.f, 240.f, 0.f };
	RenderScales = { 50.f, 60.f, 70.f, 80.f, 90.f, 100.f };

	TArray<FText> ResolutionNames;
	for (const FIntPoint& Resolution : Resolutions)
	{
		ResolutionNames.Add(FText::Format(INVTEXT("{0} × {1}"), FText::AsNumber(Resolution.X, &FNumberFormattingOptions::DefaultNoGrouping()),
			FText::AsNumber(Resolution.Y, &FNumberFormattingOptions::DefaultNoGrouping())));
	}
	TArray<FText> FrameRateNames;
	for (const float Rate : FrameRates)
	{
		FrameRateNames.Add(Rate > 0.f ? FText::AsNumber(FMath::RoundToInt(Rate)) : LOCTEXT("Unlimited", "Illimitée"));
	}
	TArray<FText> RenderScaleNames;
	for (const float Scale : RenderScales)
	{
		RenderScaleNames.Add(FText::AsPercent(Scale / 100.f));
	}
	TArray<FText> VolumeSteps;
	for (int32 Step = 0; Step <= NumVolumeSteps; ++Step)
	{
		VolumeSteps.Add(FText::AsPercent(static_cast<float>(Step) / NumVolumeSteps));
	}

	UVerticalBox* Column = MakeCenteredColumn();
	AddText(Column, TEXT("Heading"), LOCTEXT("Heading", "Réglages"), UHOLTextStyleHeading::StaticClass(), 16.f);

	UHorizontalBox* TabRow = AddButtonRow(Column);
	VideoTab = AddButton(TabRow, TEXT("VideoTab"), LOCTEXT("Video", "Vidéo"));
	SoundTab = AddButton(TabRow, TEXT("SoundTab"), LOCTEXT("Sound", "Son"));
	ControlsTab = AddButton(TabRow, TEXT("ControlsTab"), LOCTEXT("Controls", "Commandes"));
	LanguageTab = AddButton(TabRow, TEXT("LanguageTab"), LOCTEXT("Language", "Langue"));

	TabPages = WidgetTree->ConstructWidget<UWidgetSwitcher>(UWidgetSwitcher::StaticClass(), TEXT("TabPages"));
	UVerticalBoxSlot* PagesSlot = Column->AddChildToVerticalBox(TabPages);
	PagesSlot->SetHorizontalAlignment(HAlign_Center);
	PagesSlot->SetPadding(FMargin(0.f, 24.f, 0.f, 0.f));
	const auto AddPage = [this]()
	{
		UVerticalBox* Page = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		TabPages->AddChild(Page);
		return Page;
	};

	// Video: display on the left, quality on the right, the gamepad hint below.
	UVerticalBox* Video = AddPage();
	UHorizontalBox* VideoColumns = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
	Video->AddChildToVerticalBox(VideoColumns)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
	UVerticalBox* Display = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UVerticalBox* Quality = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	VideoColumns->AddChildToHorizontalBox(Display)->SetPadding(FMargin(0.f, 0.f, 24.f, 0.f));
	VideoColumns->AddChildToHorizontalBox(Quality);
	ResolutionRotator = AddRotatorRow(Display, TEXT("ResolutionRotator"), LOCTEXT("Resolution", "Résolution"), ResolutionNames, 0, VideoLabelWidth, DisplayRotatorWidth);
	WindowModeRotator = AddRotatorRow(Display, TEXT("WindowModeRotator"), LOCTEXT("WindowMode", "Affichage"),
		{ LOCTEXT("Fullscreen", "Plein écran"), LOCTEXT("WindowedFullscreen", "Fenêtre sans bordure"), LOCTEXT("Windowed", "Fenêtre") },
		0, VideoLabelWidth, DisplayRotatorWidth);
	VSyncRotator = AddRotatorRow(Display, TEXT("VSyncRotator"), LOCTEXT("VSync", "Synchronisation verticale"), OffOn(), 0, VideoLabelWidth, DisplayRotatorWidth);
	FrameRateRotator = AddRotatorRow(Display, TEXT("FrameRateRotator"), LOCTEXT("FrameRate", "Images par seconde"), FrameRateNames, 0, VideoLabelWidth, DisplayRotatorWidth);
	RenderScaleRotator = AddRotatorRow(Display, TEXT("RenderScaleRotator"), LOCTEXT("RenderScale", "Échelle de rendu"), RenderScaleNames, 0, VideoLabelWidth, DisplayRotatorWidth);
	TArray<FText> QualityNames = LevelNames();
	QualityNames.Add(LOCTEXT("Custom", "Personnalisée"));
	QualityRotator = AddRotatorRow(Quality, TEXT("QualityRotator"), LOCTEXT("Quality", "Qualité"), QualityNames, 0, VideoLabelWidth, QualityRotatorWidth);
	for (const FGroup& Group : Groups())
	{
		AddRotatorRow(Quality, FName(FString(TEXT("Quality_")) + Group.Name), Group.Label, LevelNames(), 0, VideoLabelWidth, QualityRotatorWidth);
	}
	AddColumnHint(Video);

	UVerticalBox* Sound = AddPage();
	const TArray<FText> Volumes = VolumeNames();
	for (int32 Volume = 0; Volume < Volumes.Num(); ++Volume)
	{
		AddRotatorRow(Sound, FName(FString::Printf(TEXT("Volume_%s"), *StaticEnum<EHOLVolume>()->GetNameStringByIndex(Volume))),
			Volumes[Volume], VolumeSteps, NumVolumeSteps);
	}
	AddText(Sound, TEXT("SoundNote"), LOCTEXT("SoundNote", "Musique, effets et voix agiront dès que le jeu aura des sons."),
		UHOLTextStyleNote::StaticClass(), 0.f);

	// Controls: greyed until M4 (D245).
	UVerticalBox* Controls = AddPage();
	RemapButton = AddButton(Controls, TEXT("RemapButton"), LOCTEXT("Remap", "Changer les touches"));
	CameraSpeedRotator = AddRotatorRow(Controls, TEXT("CameraSpeedRotator"), LOCTEXT("CameraSpeed", "Vitesse de la caméra"),
		{ LOCTEXT("Slow", "Lente"), LOCTEXT("Normal", "Normale"), LOCTEXT("Fast", "Rapide") }, 1);
	EdgeScrollRotator = AddRotatorRow(Controls, TEXT("EdgeScrollRotator"), LOCTEXT("EdgeScroll", "Défilement au bord de l'écran"), OffOn(), 1);
	AddText(Controls, TEXT("ControlsNote"), LOCTEXT("RemapNote", "Changer les touches : pas encore dans le jeu."), UHOLTextStyleNote::StaticClass(), 0.f);

	UVerticalBox* Language = AddPage();
	LanguageRotator = AddRotatorRow(Language, TEXT("LanguageRotator"), LOCTEXT("LanguageLabel", "Langue"), LanguageNames(), 0);

	UHorizontalBox* Buttons = AddButtonRow(Column);
	BackButton = AddButton(Buttons, TEXT("BackButton"), LOCTEXT("Back", "Retour"));
}

void UHOLSettingsScreen::NativeOnInitialized()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	Super::NativeOnInitialized();

	const auto Bind = [this](UHOLRotator* Rotator, void (UHOLSettingsScreen::*Handler)(int32))
	{
		if (Rotator)
		{
			Rotator->OnRotatedEvent.AddWeakLambda(this, [this, Handler](const int32 Value, bool) { (this->*Handler)(Value); });
		}
	};
	Bind(ResolutionRotator, &UHOLSettingsScreen::HandleResolution);
	Bind(WindowModeRotator, &UHOLSettingsScreen::HandleWindowMode);
	Bind(VSyncRotator, &UHOLSettingsScreen::HandleVSync);
	Bind(FrameRateRotator, &UHOLSettingsScreen::HandleFrameRate);
	Bind(RenderScaleRotator, &UHOLSettingsScreen::HandleRenderScale);
	Bind(QualityRotator, &UHOLSettingsScreen::HandleQuality);
	Bind(LanguageRotator, &UHOLSettingsScreen::HandleLanguage);
	Bind(CameraSpeedRotator, &UHOLSettingsScreen::HandleCameraSpeed);
	Bind(EdgeScrollRotator, &UHOLSettingsScreen::HandleEdgeScroll);

	const TArray<FGroup> AllGroups = Groups();
	for (int32 Group = 0; Group < AllGroups.Num(); ++Group)
	{
		UHOLRotator* Rotator = Cast<UHOLRotator>(WidgetTree->FindWidget(FName(FString(TEXT("Quality_")) + AllGroups[Group].Name)));
		GroupRotators.Add(Rotator);
		if (Rotator)
		{
			Rotator->OnRotatedEvent.AddWeakLambda(this, [this, Group](const int32 Value, bool) { HandleGroup(Group, Value); });
		}
	}
	const UEnum* VolumeEnum = StaticEnum<EHOLVolume>();
	for (int32 Volume = 0; Volume < VolumeEnum->NumEnums() - 1; ++Volume)
	{
		UHOLRotator* Rotator = Cast<UHOLRotator>(WidgetTree->FindWidget(FName(FString::Printf(TEXT("Volume_%s"), *VolumeEnum->GetNameStringByIndex(Volume)))));
		VolumeRotators.Add(Rotator);
		if (Rotator)
		{
			Rotator->OnRotatedEvent.AddWeakLambda(this, [this, Volume](const int32 Value, bool) { HandleVolume(Volume, Value); });
		}
	}

	// Greyed until the game has enough actions to remap (D253); camera speed and edge scrolling act since J1.
	if (RemapButton)
	{
		RemapButton->SetIsEnabled(false);
	}

	const UHOLButton* const Tabs[] = { VideoTab, SoundTab, ControlsTab, LanguageTab };
	for (int32 Tab = 0; Tab < UE_ARRAY_COUNT(Tabs); ++Tab)
	{
		if (UHOLButton* Button = const_cast<UHOLButton*>(Tabs[Tab]))
		{
			// Toggleable, or a selected button could never be deselected (the click selects it again, by SelectTab).
			Button->SetIsSelectable(true);
			Button->SetIsToggleable(true);
			Button->OnClicked().AddWeakLambda(this, [this, Tab]() { SelectTab(Tab); });
		}
	}
	if (BackButton)
	{
		BackButton->OnClicked().AddUObject(this, &UHOLSettingsScreen::Close);
	}

	SelectTab(0);
	Refresh();
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

void UHOLSettingsScreen::Refresh()
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings)
	{
		UE_LOG(LogHOLUI, Error, TEXT("Settings: the game user settings are not UHOLGameUserSettings (DefaultEngine.ini, GameUserSettingsClassName)"));
		return;
	}
	// The window as it is, which the command line can impose over the saved settings.
	FIntPoint Resolution;
	EWindowMode::Type WindowMode;
	GetDisplay(*Settings, Resolution, WindowMode);
	if (ResolutionRotator)
	{
		ResolutionRotator->SetSelectedItem(FMath::Max(0, Resolutions.IndexOfByKey(Resolution)));
	}
	if (WindowModeRotator)
	{
		WindowModeRotator->SetSelectedItem(static_cast<int32>(WindowMode));
	}
	if (VSyncRotator)
	{
		VSyncRotator->SetSelectedItem(Settings->IsVSyncEnabled() ? 1 : 0);
	}
	if (FrameRateRotator)
	{
		const float Limit = Settings->GetFrameRateLimit();
		FrameRateRotator->SetSelectedItem(Limit > 0.f ? Nearest(FrameRates, Limit) : FrameRates.Num() - 1);
	}
	if (RenderScaleRotator)
	{
		float Normalized, Value, Min, Max;
		Settings->GetResolutionScaleInformationEx(Normalized, Value, Min, Max);
		RenderScaleRotator->SetSelectedItem(Nearest(RenderScales, Value));
	}
	if (QualityRotator)
	{
		const int32 Overall = Settings->GetOverallScalabilityLevel();
		QualityRotator->SetSelectedItem(Overall >= 0 && Overall < NumLevels ? Overall : CustomQuality);
	}
	const TArray<FGroup> AllGroups = Groups();
	for (int32 Group = 0; Group < GroupRotators.Num(); ++Group)
	{
		if (GroupRotators[Group])
		{
			GroupRotators[Group]->SetSelectedItem(FMath::Clamp((Settings->*AllGroups[Group].Get)(), 0, NumLevels - 1));
		}
	}
	for (int32 Volume = 0; Volume < VolumeRotators.Num(); ++Volume)
	{
		if (VolumeRotators[Volume])
		{
			VolumeRotators[Volume]->SetSelectedItem(FMath::RoundToInt(Settings->GetVolume(static_cast<EHOLVolume>(Volume)) * NumVolumeSteps));
		}
	}
	if (LanguageRotator)
	{
		const FString Current = UKismetInternationalizationLibrary::GetCurrentLanguage();
		int32 Selected = 0;
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Languages); ++Index)
		{
			if (Current.StartsWith(Languages[Index]))
			{
				Selected = Index;
			}
		}
		LanguageRotator->SetSelectedItem(Selected);
	}
	if (CameraSpeedRotator)
	{
		CameraSpeedRotator->SetSelectedItem(static_cast<int32>(Settings->GetCameraSpeed()));
	}
	if (EdgeScrollRotator)
	{
		EdgeScrollRotator->SetSelectedItem(Settings->IsEdgeScrollEnabled() ? 1 : 0);
	}
}

void UHOLSettingsScreen::ApplyAndRefresh(const bool bResolutionChanged)
{
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		if (bResolutionChanged)
		{
			Settings->ApplyResolutionSettings(/*bCheckForCommandLineOverrides*/ false);
		}
		Settings->ApplyNonResolutionSettings();
		Settings->SaveSettings();
	}
	Refresh();
}

void UHOLSettingsScreen::HandleResolution(const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (Settings && Resolutions.IsValidIndex(Value))
	{
		// With the window mode shown, not the saved one, which the command line may have overridden.
		FIntPoint Shown;
		EWindowMode::Type WindowMode;
		GetDisplay(*Settings, Shown, WindowMode);
		Settings->SetFullscreenMode(WindowMode);
		Settings->SetScreenResolution(Resolutions[Value]);
	}
	ApplyAndRefresh(/*bResolutionChanged*/ true);
}

void UHOLSettingsScreen::HandleWindowMode(const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		// With the resolution shown, as above.
		FIntPoint Resolution;
		EWindowMode::Type Shown;
		GetDisplay(*Settings, Resolution, Shown);
		Settings->SetScreenResolution(Resolution);
		Settings->SetFullscreenMode(static_cast<EWindowMode::Type>(Value));
	}
	ApplyAndRefresh(/*bResolutionChanged*/ true);
}

void UHOLSettingsScreen::HandleVSync(const int32 Value)
{
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		Settings->SetVSyncEnabled(Value == 1);
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleFrameRate(const int32 Value)
{
	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (Settings && FrameRates.IsValidIndex(Value))
	{
		Settings->SetFrameRateLimit(FrameRates[Value]);
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleRenderScale(const int32 Value)
{
	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (Settings && RenderScales.IsValidIndex(Value))
	{
		Settings->SetResolutionScaleValueEx(RenderScales[Value]);
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleQuality(const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get();
	if (!Settings)
	{
		return;
	}
	int32 Level = Value;
	if (Level == CustomQuality)
	{
		// "Custom" only shows that the groups differ; it cannot be chosen.
		const int32 Overall = Settings->GetOverallScalabilityLevel();
		const int32 Previous = Overall >= 0 ? Overall : CustomQuality;
		Level = (Level + UHOLRotator::GetStep(Previous, Value, NumLevels + 1) + NumLevels + 1) % (NumLevels + 1);
	}
	Settings->SetOverallScalabilityLevel(Level);
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleGroup(const int32 Group, const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	const TArray<FGroup> AllGroups = Groups();
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get(); Settings && AllGroups.IsValidIndex(Group))
	{
		(Settings->*AllGroups[Group].Set)(Value);
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleVolume(const int32 Volume, const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		Settings->SetVolume(static_cast<EHOLVolume>(Volume), static_cast<float>(Value) / NumVolumeSteps);
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleCameraSpeed(const int32 Value)
{
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		Settings->SetCameraSpeed(static_cast<EHOLCameraSpeed>(FMath::Clamp(Value, 0, 2)));
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleEdgeScroll(const int32 Value)
{
	if (UHOLGameUserSettings* Settings = UHOLGameUserSettings::Get())
	{
		Settings->SetEdgeScrollEnabled(Value == 1);
	}
	ApplyAndRefresh();
}

void UHOLSettingsScreen::HandleLanguage(const int32 Value)
{
	using namespace UE::HeroesOfLiberty::UI::Settings;

	if (Value >= 0 && Value < UE_ARRAY_COUNT(Languages))
	{
		// Switched live and saved by the engine ([Internationalization] in GameUserSettings.ini).
		const bool bSet = UKismetInternationalizationLibrary::SetCurrentLanguage(Languages[Value], /*SaveToConfig*/ true);
		UE_LOG(LogHOLUI, Display, TEXT("Settings: language %s%s"), Languages[Value], bSet ? TEXT("") : TEXT(" refused"));
	}
	Refresh();
}

#undef LOCTEXT_NAMESPACE
