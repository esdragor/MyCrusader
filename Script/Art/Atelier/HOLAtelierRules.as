// AR8-b (D929, D930): pure rules of the light atelier. RED STUB: the bodies come with the next commit.

struct FHOLAtelierSend
{
	FString Type;
	FString Faction;
	FString Family;
	FString ModelFile;
	double Yaw = 0.0;
	double Scale = 1.0;
	FString Status = "provisional";
	FString Author;
	FString FileOrPack;
	FString Source;
	FString Licence;
	FString Url;
	FString DownloadedOn;
	FString LicenceProof;
	FString Notes;
	FString MadeWith;
}

namespace HOLAtelier
{
	bool IsAsked(const FString& CommandLine) { return false; }
	FString EntryStep(const FString& MapName, bool bAsked, bool bDone) { return "?"; }
	FString FamilyOf(const FString& Type, const TArray<FString>& Buildings, const TArray<FString>& Sources, const TArray<FString>& Units) { return "?"; }
	FString TypeRefusal(const FString& Type, const TArray<FString>& Buildings, const TArray<FString>& Sources, const TArray<FString>& Units) { return "?"; }
	FText RefusalText(const FString& Key) { return FText::FromString(Key); }
	TArray<FString> TableNames(const FString& Family) { TArray<FString> Out; return Out; }
	TArray<FString> FixedTypes(const TArray<FString>& Buildings, const TArray<FString>& Sources) { TArray<FString> Out; return Out; }
	FString StampOf(const FDateTime& When) { return ""; }
	FString SendFolderName(const FString& Type, const FString& Faction, const FDateTime& When) { return ""; }
	FString ModelPathOf(const FString& Entrees, const FString& Name) { return ""; }
	FString SendModelName(const FString& ModelFile) { return ""; }
	FString EntreesDir() { return ""; }
	FString EnvoisDir() { return ""; }
	FString NumberText(double Value) { return ""; }
	FString ManifestJson(const FHOLAtelierSend& Send) { return ""; }
	TArray<FString> SaveRefusals(const FHOLAtelierSend& Send, bool bModelLoaded, const TArray<FString>& Buildings, const TArray<FString>& Sources, const TArray<FString>& Units) { TArray<FString> Out; return Out; }
}
