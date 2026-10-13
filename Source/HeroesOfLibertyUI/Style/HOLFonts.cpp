#include "Style/HOLFonts.h"

#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "HeroesOfLibertyUILog.h"

namespace UE::HeroesOfLiberty::UI
{
	/** Code points drawn by the Japanese sub-font. */
	const TPair<int32, int32> JapaneseRanges[] = {
		{ 0x3000, 0x303F }, // CJK symbols and punctuation
		{ 0x3040, 0x309F }, // Hiragana
		{ 0x30A0, 0x30FF }, // Katakana
		{ 0x31F0, 0x31FF }, // Katakana phonetic extensions
		{ 0x3400, 0x4DBF }, // CJK unified ideographs, extension A
		{ 0x4E00, 0x9FFF }, // CJK unified ideographs
		{ 0xF900, 0xFAFF }, // CJK compatibility ideographs
		{ 0xFF00, 0xFFEF }, // Half-width and full-width forms
	};
}

bool UHOLFonts::MakeCompositeFont(UFont* Font, const UFont* Base, const UFontFace* JapaneseRegular, const UFontFace* JapaneseBold)
{
	using namespace UE::HeroesOfLiberty::UI;

	if (!Font || !Base || !JapaneseRegular || !JapaneseBold)
	{
		UE_LOG(LogHOLUI, Error, TEXT("Fonts: missing font or font face, composite font not made"));
		return false;
	}
	Font->Modify();
	Font->FontCacheType = EFontCacheType::Runtime;
	Font->CompositeFont = Base->CompositeFont;

	FCompositeSubFont& Japanese = Font->CompositeFont.SubTypefaces.AddDefaulted_GetRef();
	// Slate looks a face up by name, else by the attributes (bold, italic) of the base's face of that name: italic text
	// gets the regular face.
	Japanese.Typeface.Fonts.Add(FTypefaceEntry(TEXT("Regular")));
	Japanese.Typeface.Fonts.Last().Font = FFontData(JapaneseRegular);
	Japanese.Typeface.Fonts.Add(FTypefaceEntry(TEXT("Bold")));
	Japanese.Typeface.Fonts.Last().Font = FFontData(JapaneseBold);
	for (const TPair<int32, int32>& Range : JapaneseRanges)
	{
		Japanese.CharacterRanges.Add(FInt32Range(FInt32Range::BoundsType::Inclusive(Range.Key), FInt32Range::BoundsType::Inclusive(Range.Value)));
	}
#if WITH_EDITORONLY_DATA
	Japanese.EditorName = TEXT("Japanese");
#endif
	UE_LOG(LogHOLUI, Display, TEXT("Fonts: %s = %s + Japanese (%s, %s)"), *Font->GetPathName(), *Base->GetPathName(),
		*JapaneseRegular->GetName(), *JapaneseBold->GetName());
	return true;
}
