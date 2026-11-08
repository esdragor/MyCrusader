// hol.Forum.Review <letter> [x y] (VisualInProgress.md, "dix echantillons"): shows ONE variant of the Forum, composed beforehand as
// /Game/Buildings/Forum/Review/SM_ForumReview_<letter> by make_forum_mesh.py --compose --variant=<letter> --review, on the loaded map, in
// place of the previous review variant (one actor, its mesh replaced). Purely a REVIEW tool for the scenario forum_review: no gameplay,
// no collision, no shadow (D258), nothing saved. "Forum review: letter=K shown=1 x=0 y=0" in the log; shown=0 if the mesh is missing.

class AHOLForumReviewActor : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	UStaticMeshComponent Mesh;
}

void HOLForumReviewCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "Forum review: usage hol.Forum.Review <letter> [x y]");
		return;
	}
	FString Letter = Args[0].ToUpper();
	double X = 0.0;
	double Y = 0.0;
	TArray<AHOLForumReviewActor> Existing;
	GetAllActorsOfClass(Existing);
	AHOLForumReviewActor Shown = Existing.Num() > 0 ? Existing[0] : Cast<AHOLForumReviewActor>(SpawnActor(AHOLForumReviewActor::StaticClass(), FVector(X, Y, 0.0)));
	FString Path = f"/Game/Buildings/Forum/Review/SM_ForumReview_{Letter}.SM_ForumReview_{Letter}";
	UStaticMesh Model = Cast<UStaticMesh>(LoadObject(nullptr, Path));
	int Ok = 0;
	if (Shown != nullptr && Model != nullptr)
	{
		Shown.SetActorLocation(FVector(X, Y, 0.0));
		Shown.Mesh.SetStaticMesh(Model);
		Shown.Mesh.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Shown.Mesh.SetCastShadow(false);
		Ok = 1;
	}
	Log(n"LogHOLMatch", f"Forum review: letter={Letter} shown={Ok} x={int(X)} y={int(Y)}");
}

const FConsoleCommand HOLForumReviewConsole("hol.Forum.Review", n"HOLForumReviewCommand");
