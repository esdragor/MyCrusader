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
