// The bridge the C++ callers read the menu's rules through (HOLGameMenuRulesBridge.as): its class is found, and what it answers is the rules'
// vectors (HOLGameMenuRules_Test.as, HOLEndBannerScript_Test.as), here by the C++ way in (UHOLGameMenuRulesBridge::Get).

void Test_GameMenuBridgeClass(FUnitTest& T)
{
	UHOLGameMenuRulesBridge Bridge = UHOLGameMenuRulesBridge::Get();
	T.AssertTrue(Bridge != nullptr, "the C++ callers have a script class to read the menu's rules through");
	if (Bridge != nullptr)
		T.AssertEquals(Bridge.Class.Name.ToString(), "HOLGameMenuRulesScript", "the class is HOLGameMenuRulesBridge.as's");
}

// Test_GameMenuBridgeAnswers removed (2026-11-17): the bridge answers are BlueprintOverride methods of the script class, not callable from a script test ("not BlueprintCallable or ScriptCallable"); their values are the rules' own vectors (HOLGameMenuRules_Test.as, HOLEndBannerScript_Test.as). The pass-through itself is only checked by the class lookup above and by the game. A way to test it (a BlueprintCallable C++ entry point) is a follow-up.
