// N7: compiles only if the script can read the state of the neutral camps through the doors of UHOLCampSubsystem (GetNumCamps,
// IsCampCleared, GetCampLivingMembers, GetCampPost), as the camp indicator will. The effect on the real steps is checked by the
// automation test HeroesOfLiberty.Neutrals.CampStateDoors (the doors found by name and called by reflection, as this binding does).

class UHOLCampStateReadCheck : UObject
{
	// The living members of the camps still alive; -1 where there is no camp subsystem (no world).
	int ReadLivingMembers()
	{
		UHOLCampSubsystem Camps = UHOLCampSubsystem::Get();
		if (Camps == nullptr)
			return -1;
		int Total = 0;
		for (int CampId = 0; CampId < Camps.GetNumCamps(); CampId++)
		{
			FVector Post = Camps.GetCampPost(CampId);
			if (!Camps.IsCampCleared(CampId) && Post.X > -1.0e30)
				Total += Camps.GetCampLivingMembers(CampId);
		}
		return Total;
	}
}
