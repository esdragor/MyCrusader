// hol.Biome.DecorWorld (B3): what the LOADED WORLD holds of the biome's generic decor, read on the instanced meshes themselves (not on the
// plan, not on what the applier says it laid): how many pieces, of how many kinds, how far the nearest piece is from a site (start slot,
// source), and whether any component could collide, cut the navigation mesh or cast a shadow. One line in the log:
//   "Biome decor world: layers=1 pieces=60 kinds=4 sites=12 minAvoid=812 collide=0 shadow=0"
// (No component can cut the navigation mesh: the navigation exports a component only if it collides, and collide=0 is read here. The
// script cannot read bCanEverAffectNavigation back, the placer only sets it.)
// Tools/Tests/biome_decor_check.ps1 compares it with the applier's "Biome decor expect:" line and with the promises (D255-style proof:
// nothing within 800 cm of a site, nothing that touches the walkable ground's collision or navigation).

void HOLBiomeDecorWorldCommand(const TArray<FString>& Args)
{
	TArray<AHOLBiomeDecor> Layers;
	GetAllActorsOfClass(Layers);
	TArray<FVector2D> Sites = HOLBiomeDecorSites::Gather();
	int Pieces = 0;
	int Kinds = 0;
	int Collide = 0;
	int Shadow = 0;
	double MinAvoid = 1.0e9;
	for (AHOLBiomeDecor Layer : Layers)
	{
		TArray<UInstancedStaticMeshComponent> Parts;
		Layer.GetComponentsByClass(Parts);
		for (UInstancedStaticMeshComponent Part : Parts)
		{
			int Count = Part.GetInstanceCount();
			if (Count <= 0)
			{
				continue;
			}
			Kinds++;
			Pieces += Count;
			if (Part.GetCollisionEnabled() != ECollisionEnabled::NoCollision)
			{
				Collide++;
			}
			if (Part.CastShadow)
			{
				Shadow++;
			}
			for (int I = 0; I < Count; I++)
			{
				FTransform At;
				Part.GetInstanceTransform(I, At, true);
				for (FVector2D Site : Sites)
				{
					double Dx = At.Location.X - Site.X;
					double Dy = At.Location.Y - Site.Y;
					double Distance = Math::Sqrt(Dx * Dx + Dy * Dy);
					if (Distance < MinAvoid)
					{
						MinAvoid = Distance;
					}
				}
			}
		}
	}
	int Nearest = Sites.Num() > 0 ? int(MinAvoid) : -1;
	Log(n"LogHOLMatch", f"Biome decor world: layers={Layers.Num()} pieces={Pieces} kinds={Kinds} sites={Sites.Num()} minAvoid={Nearest} collide={Collide} shadow={Shadow}");
}

const FConsoleCommand HOLBiomeDecorWorldConsole("hol.Biome.DecorWorld", n"HOLBiomeDecorWorldCommand");
