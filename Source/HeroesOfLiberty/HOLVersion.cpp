#include "HOLVersion.h"

#include "HAL/PlatformProperties.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace HOLVersion
{
	FString GetFilePath()
	{
		return FPaths::Combine(FPaths::ProjectDir(), TEXT("Version.txt"));
	}

	FString Parse(const FString& FileText)
	{
		FString Line = FileText;
		int32 End = INDEX_NONE;
		if (Line.FindChar(TEXT('\n'), End))
		{
			Line.LeftInline(End);
		}
		Line.TrimStartAndEndInline();
		return Line.IsEmpty() ? FString(Unknown) : Line;
	}

	FString Display(const FString& Version)
	{
		TArray<FString> Parts;
		Version.ParseIntoArray(Parts, TEXT("."), false);
		bool bNumbered = Parts.Num() == 3 && Parts[0] == TEXT("0");
		for (int32 Index = 1; bNumbered && Index < 3; ++Index)
		{
			bNumbered = Parts[Index].IsNumeric() && !Parts[Index].Contains(TEXT("-")) && !Parts[Index].Contains(TEXT("+"));
		}
		return bNumbered ? TEXT("V ") + Version : Version;
	}

	const FString& Get()
	{
		static const FString Version = []()
		{
			// The editor and -game run uncooked: not a package.
			if (!FPlatformProperties::RequiresCookedData())
			{
				return FString(Dev);
			}
			FString Text;
			return FFileHelper::LoadFileToString(Text, *GetFilePath()) ? Parse(Text) : FString(Unknown);
		}();
		return Version;
	}

	bool IsAccepted(const FString& Server, const FString& ClientOption, FString* OutError)
	{
		// An unknown version is no proof of the same build: refused even against itself.
		const bool bAccepted = ClientOption.IsEmpty() ? Server == Dev : ClientOption == Server && Server != Unknown;
		if (!bAccepted && OutError)
		{
			*OutError = ClientOption.IsEmpty()
				? FString::Printf(TEXT("Version mismatch: server %s, client sent no version"), *Server)
				: FString::Printf(TEXT("Version mismatch: server %s, client %s"), *Server, *ClientOption);
		}
		return bAccepted;
	}
}
