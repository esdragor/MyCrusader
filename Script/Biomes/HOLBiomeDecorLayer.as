// The generic decor of a biome, LAID on the loaded map (BiomeInProgress.md B3, GDD D651 to D656). The plan (which kind, where, how big, which
// colour) is HOLBiomeDecor::Plan; this file holds the actor the plan is laid on and the gathering of the points the decor keeps clear of.
// Purely visual (D656): one instanced mesh per kind, no collision, no navigation, no shadow (D258), cut at the Python's DECOR_CULL.

// The one actor that holds the generic decor of the match's biome (an instanced mesh component per kind, named BiomeDecor_<kind>).
class AHOLBiomeDecor : AActor
{
	UPROPERTY(DefaultComponent, RootComponent)
	USceneComponent Root;
}

namespace HOLBiomeDecorSites
{
	// The points of the loaded map the decor keeps clear of (800 cm, the Python's DECOR_CLEAR): the start slots, the source fields and the
	// source chunks (gold, stone, wood, berries).
	TArray<FVector2D> Gather()
	{
		TArray<FVector2D> Sites;
		TArray<AHOLStartSlot> Slots;
		GetAllActorsOfClass(Slots);
		for (AHOLStartSlot Slot : Slots)
		{
			FVector At = Slot.GetActorLocation();
			Sites.Add(FVector2D(At.X, At.Y));
		}
		TArray<AHOLSourceField> Fields;
		GetAllActorsOfClass(Fields);
		for (AHOLSourceField Field : Fields)
		{
			FVector At = Field.GetActorLocation();
			Sites.Add(FVector2D(At.X, At.Y));
		}
		TArray<AHOLSourceChunk> Chunks;
		GetAllActorsOfClass(Chunks);
		for (AHOLSourceChunk Chunk : Chunks)
		{
			FVector At = Chunk.GetActorLocation();
			Sites.Add(FVector2D(At.X, At.Y));
		}
		return Sites;
	}
}
