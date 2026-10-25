#pragma once

#include "CoreMinimal.h"

/**
 * The shared access key (launcher L11, Manager/PROTOCOL.md): the kit's servers.json gives each manager a "key"; the game
 * sends it to the manager in the X-HOL-Key header, and to a match's server as the login option ?HOLKey=<key>
 * (URL-encoded). The manager starts its match servers with -HOLAccessKey=<key>; such a server refuses a login without
 * that key in PreLogin ("Access key mismatch"). No key (development, a manager without access_key.txt): all open.
 * The key is never logged.
 *
 * The client's key is the one of the manager called (UHOLManagerClient, servers.json's first server), set by that client
 * when it reads servers.json: the local player (this module) cannot see the interface module's subsystem.
 */
namespace HOLAccessKey
{
	/** Login option the client sends its key in (?HOLKey=). */
	inline const TCHAR* UrlOption = TEXT("HOLKey");
	/** HTTP header of every call to the manager. */
	inline const TCHAR* Header = TEXT("X-HOL-Key");
	/** Command line switch of a match's server (-HOLAccessKey=<key>). */
	inline const TCHAR* CommandLineSwitch = TEXT("HOLAccessKey=");
	/** PreLogin's refusal, read back by the client (MakeMatchLossError). */
	inline const TCHAR* MismatchError = TEXT("Access key mismatch");

	/** -HOLAccessKey= of a command line, trimmed; empty without one. */
	HEROESOFLIBERTY_API FString ParseServerKey(const TCHAR* CommandLine);
	/** The key this server expects (-HOLAccessKey= of its command line), read once; empty: none. */
	HEROESOFLIBERTY_API const FString& GetServerKey();

	/**
	 * Whether a server expecting Expected lets in a client that sent ClientOption (?HOLKey=, URL-decoded; empty: none).
	 * No key expected: everyone. OutError is MismatchError when not (never the key).
	 */
	HEROESOFLIBERTY_API bool IsAccepted(const FString& Expected, const FString& ClientOption, FString* OutError = nullptr);

	/** The key of the manager called, set by UHOLManagerClient (empty: none). */
	HEROESOFLIBERTY_API void SetClientKey(const FString& Key);
	HEROESOFLIBERTY_API const FString& GetClientKey();

	/** "HOLKey=<key URL-encoded>", empty without a key: what the local player adds to its login options. */
	HEROESOFLIBERTY_API FString MakeLoginOption(const FString& Key);
}
