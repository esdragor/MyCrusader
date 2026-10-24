#pragma once

#include "CoreMinimal.h"

/**
 * Version of the game (M6 R1.7): <yyyy-mm-dd>-<short commit> in a package, "dev" outside one (editor, -game). The
 * same for the client, the dedicated server and the manager, which compare it (R2, R3.3). Tools\Package\package.ps1
 * writes it after the build in <package>\<platform>\HeroesOfLiberty\Version.txt, beside the cooked game: no tracked
 * file is touched and nothing is rebuilt for a new version.
 */
namespace HOLVersion
{
	/** Outside a package. */
	inline const TCHAR* Dev = TEXT("dev");
	/** A package without its Version.txt (staged by hand, file removed). */
	inline const TCHAR* Unknown = TEXT("unknown");

	/** The version of this game, read once. */
	HEROESOFLIBERTY_API const FString& Get();

	/** The version in the text of a Version.txt: its first line, trimmed; Unknown if empty. */
	HEROESOFLIBERTY_API FString Parse(const FString& FileText);

	/** Path of Version.txt in a package (the project folder beside the cooked content). */
	HEROESOFLIBERTY_API FString GetFilePath();

	/** URL option a client sends its version in (?Version=), checked by the server at login (R3.3). */
	inline const TCHAR* UrlOption = TEXT("Version");

	/**
	 * Whether a server of version Server lets in a client that sent ClientOption (empty: no ?Version=). Same
	 * version only; a client without one only on a dev server (editor tests). OutError says why not, for the log.
	 */
	HEROESOFLIBERTY_API bool IsAccepted(const FString& Server, const FString& ClientOption, FString* OutError = nullptr);
}
