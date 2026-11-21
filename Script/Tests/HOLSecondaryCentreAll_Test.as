// SC3 follow-up (HALDIR-1121sa, D879 centre secondaire A): "any centre" = MainCentre AND SecondaryCentre in the script uses of MainCentre found by f85b3c0:
// Script/Player/HOLCommandGridGroups.as (IsAloneInGroup), Script/Audio/HOLAudioBuildings.as (table row, FamilyOf, SelectEventFor).
// The secondary centre is alone in its group like the forum (a real forum more, GDD 5.4) and rings the forum's selection sound (SelectBuildingForum).
// Values asserted are computed by the code under test (the grid's own SplitSubgroups, the audio table's own SelectEventFor), the forum is the reference.
// The shared rule lives in namespace HOLCentreKind (HOLCentreKind::IsCentre), see HOLCommandGridGroups.as.
// Red until the rule is coded (commit after this one): SecondaryCentre is not alone, rings nothing, has no audio row.

FHOLGroupMemberView SCAMember(FName TypeName)
{
	FHOLGroupMemberView Member;
	Member.TypeName = TypeName;
	// The way the game builds a member: bAlone comes from the grid's own rule, never typed here.
	Member.bAlone = HOLCommandGridGroups::IsAloneInGroup(TypeName);
	return Member;
}

// The secondary centre is alone in its group, like the main centre; a barracks is not.
void Test_SecondaryCentreIsAloneInGroupLikeTheForum(FUnitTest& T)
{
	bool bForum = HOLCommandGridGroups::IsAloneInGroup(n"MainCentre");
	bool bSecondary = HOLCommandGridGroups::IsAloneInGroup(n"SecondaryCentre");
	bool bBarracks = HOLCommandGridGroups::IsAloneInGroup(n"Barracks");
	T.AssertTrue(bForum, "the main centre is alone in its group (reference)");
	T.AssertEquals(bForum, bSecondary, "the secondary centre is alone in its group like the main centre");
	T.AssertFalse(bBarracks, "a barracks is not alone in its group");
}

// Effect on the real chain: a main centre, two secondary centres and a barracks split into All + one subgroup each for the three centres + the barracks's own = 5
// (a centre is never joined by another building, even of its own type, and joins no one).
void Test_SecondaryCentresSplitAloneSubgroups(FUnitTest& T)
{
	TArray<FHOLGroupMemberView> Members;
	Members.Add(SCAMember(n"MainCentre"));
	Members.Add(SCAMember(n"SecondaryCentre"));
	Members.Add(SCAMember(n"SecondaryCentre"));
	Members.Add(SCAMember(n"Barracks"));
	TArray<FHOLSubgroupView> Subgroups = HOLCommandGridGroups::SplitSubgroups(Members);
	T.AssertEquals(5, Subgroups.Num(), "All + main centre + two secondary centres, each alone + the barracks = 5 subgroups");
	if (Subgroups.Num() == 5)
	{
		T.AssertEquals(4, Subgroups[0].Members.Num(), "the All subgroup holds the four buildings");
		T.AssertEquals(1, Subgroups[2].Members.Num(), "the first secondary centre is a subgroup of its own");
		T.AssertEquals(1, Subgroups[3].Members.Num(), "the second secondary centre is not joined to the first");
	}
}

// The command priority puts every centre (main or secondary) before the other buildings.
void Test_SecondaryCentreCommandPriorityLikeTheForum(FUnitTest& T)
{
	TArray<FHOLGroupMemberView> Members;
	Members.Add(SCAMember(n"Barracks"));
	Members.Add(SCAMember(n"SecondaryCentre"));
	Members.Add(SCAMember(n"MainCentre"));
	TArray<int> Order = HOLCommandGridGroups::GetCommandPriority(Members);
	T.AssertEquals(3, Order.Num(), "the three buildings are ordered");
	if (Order.Num() == 3)
	{
		T.AssertEquals(1, Order[0], "the secondary centre comes first (index 1)");
		T.AssertEquals(2, Order[1], "then the main centre");
		T.AssertEquals(0, Order[2], "the barracks last");
	}
}

// The secondary centre rings the forum's selection sound, and is of the forum's family (stone).
void Test_SecondaryCentreRingsTheForumSelectionSound(FUnitTest& T)
{
	int ForumEvent = HOLAudioBuildings::SelectEventFor("MainCentre");
	int SecondaryEvent = HOLAudioBuildings::SelectEventFor("SecondaryCentre");
	T.AssertEquals(HOLAudioEvent::SelectBuildingForum, ForumEvent, "the main centre rings the forum sound (reference)");
	T.AssertEquals(ForumEvent, SecondaryEvent, "the secondary centre rings the same sound as the main centre");
	FString ForumFamily = HOLAudioBuildings::FamilyOf("MainCentre");
	FString SecondaryFamily = HOLAudioBuildings::FamilyOf("SecondaryCentre");
	T.AssertEquals(ForumFamily, SecondaryFamily, "the secondary centre is of the main centre's family");
	T.AssertEquals("Stone", SecondaryFamily, "stone");
}

// The audio table has a row for the secondary centre, with the main centre's production alert (it trains Peasants).
void Test_SecondaryCentreHasAnAudioRow(FUnitTest& T)
{
	T.AssertTrue(HOLAudioBuildings::Has("SecondaryCentre"), "the audio table has a row for the secondary centre");
}
