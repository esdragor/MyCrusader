#include "Net/HOLAccessKey.h"

#include "GenericPlatform/GenericPlatformHttp.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace HOLAccessKey
{
	namespace
	{
		FString& ClientKey()
		{
			static FString Key;
			return Key;
		}
	}

	FString ParseServerKey(const TCHAR* CommandLine)
	{
		FString Key;
		if (CommandLine && FParse::Value(CommandLine, CommandLineSwitch, Key))
		{
			Key.TrimStartAndEndInline();
		}
		return Key;
	}

	const FString& GetServerKey()
	{
		static const FString Key = ParseServerKey(FCommandLine::Get());
		return Key;
	}

	bool IsAccepted(const FString& Expected, const FString& ClientOption, FString* OutError)
	{
		const bool bAccepted = Expected.IsEmpty() || ClientOption.Equals(Expected, ESearchCase::CaseSensitive);
		if (!bAccepted && OutError)
		{
			*OutError = MismatchError;
		}
		return bAccepted;
	}

	void SetClientKey(const FString& Key)
	{
		ClientKey() = Key.TrimStartAndEnd();
	}

	const FString& GetClientKey()
	{
		return ClientKey();
	}

	FString MakeLoginOption(const FString& Key)
	{
		return Key.IsEmpty() ? FString() : FString::Printf(TEXT("%s=%s"), UrlOption, *FGenericPlatformHttp::UrlEncode(Key));
	}
}
