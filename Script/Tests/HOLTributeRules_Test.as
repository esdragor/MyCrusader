// The tribute's rules (D372, GDD 5.5): who may give what to whom, and what arrives (HOLTributeRules.as). Pure: the command, the
// stock and the announcement to all come with the diplomacy panel (D387).

// 100 sent at the default tax (30 %) arrives as 70; the tax is rounded down so that a small gift is not eaten whole.
void Test_TributeReceived(FUnitTest& T)
{
	T.AssertEquals(70, HOLTribute::Received(100, HOLTribute::DefaultTaxPercent), "100 at 30 %");
	T.AssertEquals(175, HOLTribute::Received(250, HOLTribute::DefaultTaxPercent), "250 at 30 %: tax 75");
	T.AssertEquals(1, HOLTribute::Received(1, HOLTribute::DefaultTaxPercent), "1 at 30 %: tax 0.3 rounded down to 0");
	T.AssertEquals(7, HOLTribute::Received(10, HOLTribute::DefaultTaxPercent), "10 at 30 %");
	T.AssertEquals(100, HOLTribute::Received(100, 0), "no tax");
	T.AssertEquals(50, HOLTribute::Received(100, 50), "half");
	T.AssertEquals(0, HOLTribute::Received(100, 100), "all taxed");
	T.AssertEquals(100, HOLTribute::Received(100, -5), "a negative tax is none");
	T.AssertEquals(0, HOLTribute::Received(100, 140), "a tax above 100 % takes everything");
	T.AssertEquals(0, HOLTribute::Received(-20, 30), "nothing to send");
}

// Each reason alone, then the order they are told in (the first that applies).
void Test_TributeCheck(FUnitTest& T)
{
	// bMarket, sender, receiver, receiver in the match, stock, sent, resource (0 food, 1 wood, 2 stone, 3 gold).
	T.AssertEquals(HOLTribute::Ok, HOLTribute::Check(true, 0, 1, true, 500, 100, 1), "a gift of wood to another player");
	T.AssertEquals(HOLTribute::Ok, HOLTribute::Check(true, 0, 1, true, 100, 100, 3), "the whole stock is allowed");
	T.AssertEquals(HOLTribute::NotEnough, HOLTribute::Check(true, 0, 1, true, 99, 100, 3), "one short");
	T.AssertEquals(HOLTribute::NoMarket, HOLTribute::Check(false, 0, 1, true, 500, 100, 1), "no finished Market");
	T.AssertEquals(HOLTribute::Self, HOLTribute::Check(true, 2, 2, true, 500, 100, 1), "to oneself");
	T.AssertEquals(HOLTribute::NoRecipient, HOLTribute::Check(true, 0, 5, false, 500, 100, 1), "nobody there");
	T.AssertEquals(HOLTribute::BadAmount, HOLTribute::Check(true, 0, 1, true, 500, 0, 1), "nothing sent");
	T.AssertEquals(HOLTribute::BadAmount, HOLTribute::Check(true, 0, 1, true, 500, -50, 1), "a negative amount");
	T.AssertEquals(HOLTribute::BadResource, HOLTribute::Check(true, 0, 1, true, 500, 100, 4), "no such resource");
	T.AssertEquals(HOLTribute::BadResource, HOLTribute::Check(true, 0, 1, true, 500, 100, -1), "no such resource (negative)");
	// An ally or an enemy: the rule does not ask (pay for peace, D372): the same call, the same answer.
	T.AssertEquals(HOLTribute::BadAmount, HOLTribute::Check(false, 2, 2, false, 0, 0, 9), "the amount comes first");
	T.AssertEquals(HOLTribute::BadResource, HOLTribute::Check(false, 2, 2, false, 0, 100, 9), "then the resource");
	T.AssertEquals(HOLTribute::Self, HOLTribute::Check(false, 2, 2, false, 0, 100, 1), "then oneself");
	T.AssertEquals(HOLTribute::NoRecipient, HOLTribute::Check(false, 0, 1, false, 0, 100, 1), "then the recipient");
	T.AssertEquals(HOLTribute::NoMarket, HOLTribute::Check(false, 0, 1, true, 0, 100, 1), "then the Market");
}

// What is announced to all, from a made tribute: who gave whom what, what left and what arrived.
void Test_TributeMake(FUnitTest& T)
{
	FHOLTribute Gift = HOLTribute::Make(0, 3, 2, 250, HOLTribute::DefaultTaxPercent);
	T.AssertEquals(0, Gift.Sender, "sender");
	T.AssertEquals(3, Gift.Receiver, "receiver");
	T.AssertEquals(2, Gift.Resource, "stone");
	T.AssertEquals(250, Gift.Sent, "what left the sender");
	T.AssertEquals(175, Gift.Received, "what reached the receiver");
	T.AssertEquals(75, Gift.Tax(), "the tax is the difference");
}