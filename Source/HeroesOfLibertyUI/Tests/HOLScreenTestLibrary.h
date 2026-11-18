// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLScreenTestLibrary.generated.h"

class UUserWidget;
class UWidget;

/**
 * Test harness of the screens for Angelscript (namespace HOLScreenTest there, D839: a binding exposed to the script, a test harness).
 * The world of a script unit test has no local player and no player controller, so a script test cannot create a screen (a UUserWidget
 * built without a player never runs NativeOnInitialized, UUserWidget::Initialize): this library gives it the set up the C++ screen tests
 * make (HOLNewGameSitesBoxTests.cpp): a game world, its context, a local player and its controller, then the screen created and
 * initialized as the game does. The bodies exist only where WITH_DEV_AUTOMATION_TESTS (the editor target); elsewhere (a packaged game)
 * every function does nothing and returns nullptr / false / empty text. One screen test at a time: the objects live in statics until
 * EndScreenTest (or until the next CreateScreenForTest, which ends the previous set up first).
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLScreenTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** A game world with a local player and its controller, then a widget of ScreenClass created in it and initialized (the layout built).
	 * nullptr, with a line in the log, if a step fails. Call EndScreenTest when the test is done. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static UUserWidget* CreateScreenForTest(TSubclassOf<UUserWidget> ScreenClass);

	/** Ends the set up of CreateScreenForTest: the screen released, the world and its context destroyed. Safe to call twice. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static void EndScreenTest();

	/** Same as EndScreenTest (the screen is the one CreateScreenForTest returned). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static void DestroyScreenForTest(UUserWidget* Screen);

	/** The widget named Name in the screen's widget tree (UWidgetTree::FindWidget), nullptr if none (or no screen / no tree). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static UWidget* FindWidgetByName(UUserWidget* Screen, FName Name);

	/** UWidget::GetIsEnabled of the widget (false for nullptr): the state of a box the screen greys or enables. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static bool IsWidgetEnabled(UWidget* Widget);

	/** The text a UHOLDropdown shows (GetShownText); empty if the widget is not a UHOLDropdown. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static FText GetDropdownText(UWidget* Widget);

	/** The index a UHOLDropdown has selected (GetSelectedIndex); -1 if the widget is not a UHOLDropdown. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|ScreenTest")
	static int32 GetDropdownIndex(UWidget* Widget);
};
