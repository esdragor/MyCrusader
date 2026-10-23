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
}
