// Lot E3-07 of the migration (D830, D836, D839): the script port of HOLVersionTests.cpp (4 tests), run against the read libraries of the
// rules that STAY in C++ (HOLVersionReadLibrary, HOLAccessKeyReadLibrary, S1D-1116b). Same vectors as the C++ tests; the C++ file is removed
// at lot E3-FIN.

// HeroesOfLiberty.Version: version of the game (R1.7): "dev" outside a package, the first line of Version.txt in one.
void Test_Version(FUnitTest& T)
{
	T.AssertTrue(HOLVersionRead::Get() == HOLVersionRead::GetDev(), "dev outside a package");
	T.AssertTrue(HOLVersionRead::Parse(" 2026-10-23-f7269a8\r\nmore\n") == "2026-10-23-f7269a8", "first line, trimmed");
	T.AssertTrue(HOLVersionRead::Parse("2026-10-23-f7269a8") == "2026-10-23-f7269a8", "no newline");
	T.AssertTrue(HOLVersionRead::Parse("\r\n") == HOLVersionRead::GetUnknown(), "empty file");
}

// HeroesOfLiberty.VersionDisplay (L14): the V only before a numbered version "0.Y.X".
void Test_VersionDisplay(FUnitTest& T)
{
	T.AssertTrue(HOLVersionRead::Display("0.3.0") == "V 0.3.0", "numbered");
	T.AssertTrue(HOLVersionRead::Display("0.12.105") == "V 0.12.105", "numbered, two digits");
	T.AssertTrue(HOLVersionRead::Display("2026-10-30-r9") == "2026-10-30-r9", "old dated name");
	T.AssertTrue(HOLVersionRead::Display(HOLVersionRead::GetDev()) == HOLVersionRead::GetDev(), "dev");
	T.AssertTrue(HOLVersionRead::Display(HOLVersionRead::GetUnknown()) == HOLVersionRead::GetUnknown(), "unknown");
	T.AssertTrue(HOLVersionRead::Display("0.3") == "0.3", "two parts");
	T.AssertTrue(HOLVersionRead::Display("0.a.1") == "0.a.1", "letters");
	T.AssertTrue(HOLVersionRead::Display("1.3.0") == "1.3.0", "not stage 0");
}

// HeroesOfLiberty.VersionCheck (R3.3): the server lets in a client of its own version only.
void Test_VersionCheck(FUnitTest& T)
{
	const FString Packaged = "2026-10-23-b4a8c8c";
	const FString Other = "2026-10-24-0123abc";
	const FString Dev = HOLVersionRead::GetDev();
	const FString Unknown = HOLVersionRead::GetUnknown();
	FString Error;
	T.AssertTrue(HOLVersionRead::IsAccepted(Packaged, Packaged, Error), "same packaged version");
	T.AssertTrue(HOLVersionRead::IsAccepted(Dev, Dev, Error), "dev with dev");
	T.AssertTrue(HOLVersionRead::IsAccepted(Dev, "", Error), "dev server, no version (editor tests)");

	T.AssertFalse(HOLVersionRead::IsAccepted(Packaged, Other, Error), "other packaged version");
	T.AssertTrue(Error == "Version mismatch: server " + Packaged + ", client " + Other, "mismatch message");
	T.AssertFalse(HOLVersionRead::IsAccepted(Packaged, Dev, Error), "dev client, packaged server");
	T.AssertFalse(HOLVersionRead::IsAccepted(Dev, Packaged, Error), "packaged client, dev server");
	T.AssertFalse(HOLVersionRead::IsAccepted(Packaged, "", Error), "no version, packaged server");
	T.AssertTrue(Error == "Version mismatch: server " + Packaged + ", client sent no version", "missing message");
	T.AssertFalse(HOLVersionRead::IsAccepted(Unknown, Unknown, Error), "unknown server refuses unknown client");
}

// HeroesOfLiberty.AccessKey (Launcher L11): a match server started with -HOLAccessKey= lets in only the clients sending ?HOLKey= with that key.
void Test_AccessKey(FUnitTest& T)
{
	FString Error;
	T.AssertTrue(HOLAccessKeyRead::IsAccepted("", "", Error), "no key expected, none sent");
	T.AssertTrue(HOLAccessKeyRead::IsAccepted("", "abc", Error), "no key expected, one sent");
	T.AssertTrue(HOLAccessKeyRead::IsAccepted("Ab-_09", "Ab-_09", Error), "same key");
	T.AssertFalse(HOLAccessKeyRead::IsAccepted("Ab-_09", "", Error), "no key sent");
	T.AssertTrue(Error == "Access key mismatch", "mismatch message, without the key");
	T.AssertFalse(HOLAccessKeyRead::IsAccepted("Ab-_09", "Ab-_08", Error), "other key");
	T.AssertFalse(HOLAccessKeyRead::IsAccepted("Ab-_09", "ab-_09", Error), "case matters");

	T.AssertTrue(HOLAccessKeyRead::ParseServerKey("/Game/Maps/Plain -server -HOLAccessKey=Ab-_09 -Port=7778") == "Ab-_09", "server key from the command line");
	T.AssertTrue(HOLAccessKeyRead::ParseServerKey("/Game/Maps/Plain -server -Port=7778") == "", "no server key");

	T.AssertTrue(HOLAccessKeyRead::MakeLoginOption("Ab-_09") == "HOLKey=Ab-_09", "login option");
	T.AssertTrue(HOLAccessKeyRead::MakeLoginOption("a?b=c") == "HOLKey=a%3Fb%3Dc", "login option encoded");
	T.AssertTrue(HOLAccessKeyRead::MakeLoginOption("") == "", "no key, no option");
}
