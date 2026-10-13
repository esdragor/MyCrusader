#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLFonts.generated.h"

class UFont;
class UFontFace;

/**
 * The font of the interface (D251): a composite font of the project, /Game/UI/Fonts/F_HOL, made by
 * Tools/Editor/make_ui_fonts.py. The composite font's sub-fonts cannot be set from Python, hence this helper.
 */
UCLASS()
class HEROESOFLIBERTYUI_API UHOLFonts : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Makes Font the typefaces of Base (Roboto of the engine: Latin, its fallback for other scripts) with a Japanese
	 * sub-font over kana, kanji, Japanese punctuation and full-width forms, in regular and bold.
	 */
	UFUNCTION(BlueprintCallable, Category = "HOL|Fonts")
	static bool MakeCompositeFont(UFont* Font, const UFont* Base, const UFontFace* JapaneseRegular, const UFontFace* JapaneseBold);
};
