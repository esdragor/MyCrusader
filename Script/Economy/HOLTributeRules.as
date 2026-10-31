// The tribute's rules (D372, GDD 5.5, § 14.1): a Market lets a player give resources to ANY player, ally or not ("pay for peace"),
// taxed about 30 % (indicative, to be tuned in play), announced to all. Pure and in integers (client and server agree): who may give what to
// whom, and what arrives. The command, the stock and the announcement come with the diplomacy panel (D387); nothing here reads the world.
struct FHOLTribute
{
	int Sender = 0;
	int Receiver = 0;
	// 0 food, 1 wood, 2 stone, 3 gold (EHOLResource's order).
	int Resource = 0;
	// What leaves the sender's stock, and what reaches the receiver's.
	int Sent = 0;
	int Received = 0;

	// What the Market keeps.
	int Tax() const
	{
		return Sent - Received;
	}
}

namespace HOLTribute
{
	// The reasons, told in this order (the first that applies).
	const int Ok = 0;
	const int NoMarket = 1;
	const int Self = 2;
	const int NoRecipient = 3;
	const int NotEnough = 4;
	const int BadAmount = 5;
	const int BadResource = 6;

	// About 30 % (D372, indicative).
	const int DefaultTaxPercent = 30;

	// What reaches the receiver of Sent at TaxPercent: the tax rounded down (a gift of 1 is not eaten), the tax bounded to 0..100 %.
	int Received(int Sent, int TaxPercent)
	{
		if (Sent <= 0)
			return 0;
		int Percent = Math::Clamp(TaxPercent, 0, 100);
		int Tax = (Sent * Percent) / 100;
		return Sent - Tax;
	}

	// May the sender give Sent of Resource to Receiver? Order of the reasons: the amount, the resource, oneself, the recipient, the
	// finished Market, then the stock.
	int Check(bool bSenderHasMarket, int Sender, int Receiver, bool bReceiverInMatch, int Stock, int Sent, int Resource)
	{
		if (Sent <= 0)
			return BadAmount;
		if (Resource < 0 || Resource > 3)
			return BadResource;
		if (Sender == Receiver)
			return Self;
		if (!bReceiverInMatch)
			return NoRecipient;
		if (!bSenderHasMarket)
			return NoMarket;
		if (Stock < Sent)
			return NotEnough;
		return Ok;
	}

	// The tribute as announced to all.
	FHOLTribute Make(int Sender, int Receiver, int Resource, int Sent, int TaxPercent)
	{
		FHOLTribute Gift;
		Gift.Sender = Sender;
		Gift.Receiver = Receiver;
		Gift.Resource = Resource;
		Gift.Sent = Sent;
		Gift.Received = Received(Sent, TaxPercent);
		return Gift;
	}
}