// hol.Forum.Review <letter> (VisualInProgress.md, "dix echantillons"): shows ONE variant of the Forum, composed beforehand as
// /Game/Buildings/Forum/Review/SM_ForumReview_<letter> by make_forum_mesh.py --compose --variant=<letter> --review, on the loaded map, in
// place of the previous review variant (one actor at ReviewSpot, its mesh replaced). Purely a REVIEW tool for the scenario forum_review:
// no gameplay, no collision, no shadow (D258), nothing saved. A yellow label "FORUM <letter>" is put on the ground between the Forum and the
// camera, turned towards the camera at the moment of the command (so call it again after moving the camera).
// ReviewSpot is away from the map centre (a ring of trees hides it there) and from the players' bases: the scenario focuses the camera on it.
// "Forum review: letter=K shown=1" in the log; shown=0 if the mesh is missing.

const FVector ReviewSpot = FVector(0.0, -3000.0, 0.0);

class AHOLForumReviewActor : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	UStaticMeshComponent Mesh;

	UPROPERTY(DefaultComponent)
	UTextRenderComponent Label;
}

void HOLForumReviewCommand(const TArray<FString>& Args)
{
	if (Args.Num() < 1)
	{
		Log(n"LogHOLMatch", "Forum review: usage hol.Forum.Review <letter>");
		return;
	}
	FString Letter = Args[0].ToUpper();
	TArray<AHOLForumReviewActor> Existing;
	GetAllActorsOfClass(Existing);
	AHOLForumReviewActor Shown = Existing.Num() > 0 ? Existing[0] : Cast<AHOLForumReviewActor>(SpawnActor(AHOLForumReviewActor::StaticClass(), ReviewSpot));
	FString Path = f"/Game/Buildings/Forum/Review/SM_ForumReview_{Letter}.SM_ForumReview_{Letter}";
	UStaticMesh Model = Cast<UStaticMesh>(LoadObject(nullptr, Path));
	int Ok = 0;
	if (Shown != nullptr && Model != nullptr)
	{
		Shown.Mesh.SetMobility(EComponentMobility::Movable);
		Shown.Mesh.SetStaticMesh(Model);
		Shown.Mesh.SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Shown.Mesh.SetCastShadow(false);
		Shown.SetActorLocation(ReviewSpot);
		FVector CameraPlace = Gameplay::GetPlayerCameraManager(0).GetCameraLocation();
		FVector ToCamera = CameraPlace - ReviewSpot;
		ToCamera.Z = 0.0;
		ToCamera = ToCamera.GetSafeNormal();
		FVector LabelPlace = ReviewSpot + ToCamera * 1000.0 + FVector(0.0, 0.0, 150.0);
		Shown.Label.SetText(FText::FromString(f"FORUM {Letter}"));
		Shown.Label.SetWorldSize(260.0);
		Shown.Label.SetTextRenderColor(FColor::Yellow);
		Shown.Label.SetCastShadow(false);
		Shown.Label.SetWorldLocation(LabelPlace);
		Shown.Label.SetWorldRotation((CameraPlace - LabelPlace).Rotation());
		Ok = 1;
	}
	Log(n"LogHOLMatch", f"Forum review: letter={Letter} shown={Ok}");
}

const FConsoleCommand HOLForumReviewConsole("hol.Forum.Review", n"HOLForumReviewCommand");
