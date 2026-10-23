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
}
