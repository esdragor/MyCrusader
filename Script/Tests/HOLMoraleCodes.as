// The morale bridge's codes, for the tests only (no script outside Script\Tests reads them). The rule has ONE source, Simulation/HOLMoraleRules.h
// (the same numbers as its Lever* constants and its Apply* result codes); the bridge UHOLMoraleLibrary (HOLMorale::Native*) takes and returns plain ints.
// No rule here. Replaces the constants of the former relay Script/Morale/HOLMoraleRules.as (REVUE-1123pm genant 4).
namespace HOLMorale
{
	// The three levers.
	const int LeverDealt = 0;
	const int LeverTaken = 1;
	const int LeverRate = 2;
	const int LeverCount = 3;

	// What NativeApply did.
	const int ApplyNothing = 0;
	const int ApplyPosed = 1;
	const int ApplyRefreshed = 2;
	const int ApplyBlockedByImmunity = 3;
	const int ApplyNoRoom = 4;
}
