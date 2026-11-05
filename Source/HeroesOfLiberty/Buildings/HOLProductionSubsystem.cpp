// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLProductionSubsystem.h"

#include "Match/HOLMatchStatsSubsystem.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLFactionRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLPlayerState.h"
#include "Net/HOLUnitReceptionSubsystem.h"
#include "Siege/HOLSiegeRules.h"
#include "Simulation/HOLHeroLibrary.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Technologies/HOLTechnologyRules.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"

namespace UE::HeroesOfLiberty::Production
{
	TAutoConsoleVariable<float> CVarBuildTime(
		TEXT("hol.Prod.BuildTime"),
		0.f,
		TEXT("Production time (s) of every unit type in place of the table's (0: the table). For the scripted checks (D284): a Peasant in 2 s, not 20."));

	/** Gap (cm) between the building's footprint and the first row of units born there. */
	constexpr float BirthGap = 400.f;
	/** Angle (deg) between two neighbours of the arc. */
	constexpr float BirthArcStep = 12.f;
	/** Units in one arc (the middle one, then alternately right and left); the next ones stand a row further. */
	constexpr int32 BirthsPerRow = 9;
	/** Distance (cm) between two rows. */
	constexpr float BirthRowGap = 150.f;

	const TCHAR* DescribeRefusal(const EHOLProductionRefusal Refusal)
	{
		switch (Refusal)
		{
		case EHOLProductionRefusal::NotProducible: return TEXT("not produced here");
		case EHOLProductionRefusal::QueueFull: return TEXT("queue full");
		case EHOLProductionRefusal::NotEnoughResources: return TEXT("not enough resources");
		default: return TEXT("none");
		}
	}
}

bool UHOLProductionSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UHOLProductionSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// The server (or a solo match) decides alone (D282); a client only shows what it receives.
	if (InWorld.GetNetMode() == NM_Client)
	{
		return;
	}
	StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	if (UHOLSimulationSubsystem* Simulation = InWorld.GetSubsystem<UHOLSimulationSubsystem>())
	{
		StepStartedHandle = Simulation->GetOnStepStarted().AddUObject(this, &UHOLProductionSubsystem::OnStepStarted);
	}
}

void UHOLProductionSubsystem::Deinitialize()
{
	if (UWorld* World = GetWorld())
	{
		if (UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>())
		{
			Simulation->GetOnStepStarted().Remove(StepStartedHandle);
		}
	}
	Super::Deinitialize();
}

double UHOLProductionSubsystem::GetDisplayStep(const UWorld* World)
{
	if (!World)
	{
		return 0.0;
	}
	if (World->GetNetMode() == NM_Client)
	{
		const UHOLUnitReceptionSubsystem* Reception = World->GetSubsystem<UHOLUnitReceptionSubsystem>();
		return Reception && Reception->GetClock().IsValid() ? Reception->GetClock().GetStep(FPlatformTime::Seconds()) : 0.0;
	}
	// The step being run is GetStepIndex() - 1 once it is over: an item started at step S is done when step
	// S + BuildSteps runs, which this shows as a full bar.
	const UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	return Simulation ? static_cast<double>(Simulation->GetStepIndex() - 1) + Simulation->GetInterpolationAlpha() : 0.0;
}

int32 UHOLProductionSubsystem::GetBuildSteps(const FHOLUnitTypeRow& Type, const float InStepSeconds)
{
	const float Override = UE::HeroesOfLiberty::Production::CVarBuildTime.GetValueOnGameThread();
	const float BuildTime = Override > 0.f ? Override : Type.BuildTime;
	return FMath::Max(1, FMath::RoundToInt(BuildTime / FMath::Max(InStepSeconds, UE_KINDA_SMALL_NUMBER)));
}

FHOLProductionCost UHOLProductionSubsystem::GetCost(const FName TypeName, const int32 PlayerIndex) const
{
	FHOLProductionCost Cost;
	const UHOLUnitSubsystem* Units = GetWorld()->GetSubsystem<UHOLUnitSubsystem>();
	// L0 (LQ-7): the row as the payer's faction fields it (its overrides: cost, time...); the table's without a payer.
	const FHOLUnitTypeRow* Row = !Units ? nullptr : PlayerIndex == INDEX_NONE ? Units->FindTypeRow(TypeName)
		: Units->FindTypeRow(TypeName, HOLFactionRules::GetSlotFactionKey(GetWorld(), PlayerIndex));
	if (Row)
	{
		Cost.Resources = Row->Cost;
		const FName DiscountStat = HOLTechnology::GetCostDiscountStat(TypeName);
		const AHOLPlayerState* Payer = DiscountStat.IsNone() || PlayerIndex == INDEX_NONE ? nullptr : FindPlayerState(PlayerIndex);
		if (Payer)
		{
			Cost.Resources = HOLTechnology::GetDiscountedCost(Cost.Resources, Payer->GetTechnologyBonusPercent(DiscountStat));
		}
		Cost.BuildSteps = GetBuildSteps(*Row, StepSeconds);
		Cost.Population = Row->Population;
	}
	// A technology (T3, T-Q6 = A): an item of the same queue, no population place.
	else if (const FHOLTechnologyRow* Technology = UHOLTechnologySettings::FindTechnology(TypeName))
	{
		Cost = HOLTechnology::MakeCost(*Technology, StepSeconds);
		const float Override = UE::HeroesOfLiberty::Production::CVarBuildTime.GetValueOnGameThread();
		if (Override > 0.f)
		{
			Cost.BuildSteps = FMath::Max(1, FMath::RoundToInt(Override / FMath::Max(StepSeconds, UE_KINDA_SMALL_NUMBER)));
		}
	}
	return Cost;
}

void UHOLProductionSubsystem::AddCheatPopulation(const int32 PlayerIndex, const int32 Places)
{
	CheatPopulation.FindOrAdd(PlayerIndex) += Places;
}

AHOLPlayerState* UHOLProductionSubsystem::FindPlayerState(const int32 PlayerIndex) const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState || PlayerIndex == INDEX_NONE)
	{
		return nullptr;
	}
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		if (HOLPlayerState && HOLPlayerState->GetPlayerIndex() == PlayerIndex)
		{
			return HOLPlayerState;
		}
	}
	return nullptr;
}

void UHOLProductionSubsystem::OnStepStarted(const int64 StepIndex)
{
	ApplyOrders();
	AdvanceQueues(StepIndex);
	RecoverMarketBook();
}

bool UHOLProductionSubsystem::EnsureMarketSettings()
{
	if (!bMarketSettingsLoaded)
	{
		bMarketSettingsLoaded = HOLMarket::LoadStandardSettings(MarketSettings);
	}
	return bMarketSettingsLoaded;
}

void UHOLProductionSubsystem::RecoverMarketBook()
{
	AHOLGameState* GameState = GetWorld()->GetGameState<AHOLGameState>();
	if (!GameState || !GameState->HasMarketBook() || !EnsureMarketSettings())
	{
		return;
	}
	const int32 IntervalSteps = FMath::Max(1, FMath::RoundToInt(static_cast<float>(MarketSettings.RecoverEverySeconds) / FMath::Max(StepSeconds, UE_KINDA_SMALL_NUMBER)));
	if (++StepsSinceMarketRecover >= IntervalSteps)
	{
		StepsSinceMarketRecover = 0;
		GameState->SetMarketBook(HOLMarket::Recover(MarketSettings, GameState->GetMarketBook(), 1));
	}
}

void UHOLProductionSubsystem::ApplyMarketTrade(const FHOLProductionCommand& Order, const AHOLBuilding& Building, AHOLPlayerState& PlayerState)
{
	// A client could send anything: only a Market, and only whole lots of the three tradable goods, up to a sane count.
	constexpr int32 MaxLotsAnOrder = 50;
	AHOLGameState* GameState = GetWorld()->GetGameState<AHOLGameState>();
	if (Building.GetTypeName() != HOLBuildingTypeNames::Market)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Market: slot %d trade ignored: %s is not a Market"), Order.PlayerIndex, *Building.GetTypeName().ToString());
		return;
	}
	if (!GameState || !GameState->HasMarketBook() || !EnsureMarketSettings())
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Market: slot %d trade ignored: no book of prices"), Order.PlayerIndex);
		return;
	}
	if (Order.Resource < 0 || Order.Resource >= HOLMarket::TradableResources || Order.Lots <= 0 || Order.Lots > MaxLotsAnOrder)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Market: slot %d trade ignored: resource %d, %d lots"), Order.PlayerIndex, Order.Resource, Order.Lots);
		return;
	}
	FHOLResources After;
	const HOLMarket::FTrade Trade = HOLMarket::Apply(MarketSettings, GameState->GetMarketBook(), PlayerState.GetResources(),
		static_cast<EHOLResource>(Order.Resource), Order.Lots, Order.bBuy, MarketSettings.FeePercent, After);
	if (!Trade.bOk)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Market: slot %d cannot %s %d lots of resource %d (%s)"), Order.PlayerIndex,
			Order.bBuy ? TEXT("buy") : TEXT("sell"), Order.Lots, Order.Resource, *Trade.Reason.ToString());
		return;
	}
	// The book first: the stock's change refreshes the HUD, whose Market cells quote the book (they showed the one before).
	GameState->SetMarketBook(Trade.Book);
	PlayerState.SetResources(After);
	UE_LOG(LogHOLCommands, Display, TEXT("Market: slot %d %s %d lots of resource %d for %d gold (price now %d, %s)"), Order.PlayerIndex,
		Order.bBuy ? TEXT("bought") : TEXT("sold"), Order.Lots, Order.Resource, Trade.Gold, Trade.Book.Prices[Order.Resource], *After.ToString());
}

void UHOLProductionSubsystem::ApplyTribute(const FHOLProductionCommand& Order, const AHOLBuilding& Building, AHOLPlayerState& Giver)
{
	// A client could send anything: only from a Market, to another player of the match, one of the four resources, a positive amount in stock.
	AHOLPlayerState* Taker = Order.Receiver != Order.PlayerIndex ? FindPlayerState(Order.Receiver) : nullptr;
	const bool bResource = Order.Resource >= 0 && Order.Resource <= static_cast<int32>(EHOLResource::Gold);
	if (Building.GetTypeName() != HOLBuildingTypeNames::Market || !Taker || !bResource || Order.Amount <= 0)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Tribute: slot %d to slot %d ignored (%s, resource %d, amount %d)"), Order.PlayerIndex, Order.Receiver,
			*Building.GetTypeName().ToString(), Order.Resource, Order.Amount);
		return;
	}
	const EHOLResource Resource = static_cast<EHOLResource>(Order.Resource);
	FHOLResources GiverStock = Giver.GetResources();
	if (GiverStock[Resource] < Order.Amount)
	{
		UE_LOG(LogHOLCommands, Display, TEXT("Tribute: slot %d cannot give %d of resource %d (stock %d)"), Order.PlayerIndex, Order.Amount, Order.Resource, GiverStock[Resource]);
		return;
	}
	const int32 Arrives = HOLTribute::Received(Order.Amount, HOLTribute::DefaultTaxPercent);
	FHOLResources TakerStock = Taker->GetResources();
	GiverStock[Resource] -= Order.Amount;
	TakerStock[Resource] += Arrives;
	Giver.SetResources(GiverStock);
	Taker->SetResources(TakerStock);
	UE_LOG(LogHOLCommands, Display, TEXT("Tribute: slot %d gave %d of resource %d to slot %d, %d arrived (tax %d)"), Order.PlayerIndex, Order.Amount, Order.Resource,
		Order.Receiver, Arrives, Order.Amount - Arrives);
	// Only the receiver is told (TB-Q1).
	if (AHOLGameState* State = GetWorld()->GetGameState<AHOLGameState>())
	{
		State->NoteTributeReceived(Order.PlayerIndex, Order.Receiver, Order.Resource, Arrives, Order.Amount - Arrives);
	}
}

void UHOLProductionSubsystem::ApplyOrders()
{
	using namespace UE::HeroesOfLiberty::Production;

	UHOLCommandSubsystem* Commands = GetWorld()->GetSubsystem<UHOLCommandSubsystem>();
	if (!Commands)
	{
		return;
	}
	TArray<FHOLProductionCommand> Orders;
	Commands->ConsumeProductionOrders(Orders);
	for (const FHOLProductionCommand& Order : Orders)
	{
		AHOLBuilding* Building = Order.Building.Get();
		AHOLPlayerState* PlayerState = FindPlayerState(Order.PlayerIndex);
		// IA0.1 E2/E4: no stock to pay with (an AI slot before IA0.1), not "not its building".
		if (!PlayerState)
		{
			UE_LOG(LogHOLCommands, Warning, TEXT("Production order of slot %d ignored: no player state"), Order.PlayerIndex);
			continue;
		}
		// Only the owner gives its buildings orders: a client could send anything.
		if (!Building || Building->GetOwnerPlayer() != Order.PlayerIndex)
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Production order of slot %d ignored: not its building"), Order.PlayerIndex);
			continue;
		}
		const FHOLBuildingTypeRow* Type = Building->GetType();
		if (!Type)
		{
			continue;
		}
		// A building under construction produces nothing (D312); one that will produce takes its rally point already, from its
		// ghost on (the user's request of 2026-10-19): kept through the site to the finished building.
		const bool bEarlyRally = Order.Kind == FHOLProductionCommand::EKind::Rally && !Type->Produces.IsEmpty();
		if (!Building->IsFinished() && !bEarlyRally)
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Production order of slot %d ignored: %s not finished"), Order.PlayerIndex, *Building->GetTypeName().ToString());
			continue;
		}
		// A main forum in ruin or rebuilding produces nothing (D351); its rally point stays its owner's to move.
		if (Building->IsForumDown() && Order.Kind != FHOLProductionCommand::EKind::Rally)
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Production order of slot %d ignored: its forum is down"), Order.PlayerIndex);
			continue;
		}

		if (Order.Kind == FHOLProductionCommand::EKind::Rally)
		{
			// On the building itself: no rally point any more (the units are born at its foot, D283); with Shift (a step added,
			// EtapeDefini N.1), nothing.
			if (Order.bQueue && Building->HasRallyPoint() && Building->IsOnBody(Order.Location))
			{
				UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d added no step to the rally point of %s: on the building itself"), Order.PlayerIndex,
					*Building->GetTypeName().ToString());
			}
			else if (Building->IsOnBody(Order.Location))
			{
				Building->ClearRallyPoint();
				UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d cleared the rally point of %s"), Order.PlayerIndex, *Building->GetTypeName().ToString());
			}
			else
			{
				// On one of the player's own sites not finished (the user's trial of J6a.4): the units born here build it; on one
				// of its camps, finished or not: once it is, they gather the nearest source it accepts (D312, J6b.1); the flag
				// on its middle (as on a source); a client could name any building (D288).
				AHOLBuilding* Site = Order.RallySite.Get();
				// Its main forum down is rebuilt as a site (V2.3a).
				if (Site && (Site == Building || Site->GetOwnerPlayer() != Order.PlayerIndex || (!Site->NeedsBuilders() && !Site->IsCamp())))
				{
					Site = nullptr;
				}
				const FVector Point = Site ? Site->GetActorLocation() : Order.Location;
				const int32 SourceIndex = Site ? INDEX_NONE : Order.SourceIndex;
				const FString OnWhat = Site ? FString::Printf(TEXT(", on site %s"), *Site->GetTypeName().ToString())
					: SourceIndex != INDEX_NONE ? FString::Printf(TEXT(", on source %d"), SourceIndex) : FString();
				// Shift (EtapeDefini N.1, AoE4): a step at the end of the rally point, MaxRallySteps at most; the first one is the
				// rally point itself.
				if (Order.bQueue && Building->HasRallyPoint())
				{
					if (Building->AddRallyStep(Point, SourceIndex, Site))
					{
						UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d added step %d to the rally point of %s at (%.0f, %.0f)%s"), Order.PlayerIndex,
							Building->GetNumRallySteps(), *Building->GetTypeName().ToString(), Point.X, Point.Y, *OnWhat);
					}
					else
					{
						UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d cannot add a step to the rally point of %s (%d steps at most)"), Order.PlayerIndex,
							*Building->GetTypeName().ToString(), AHOLBuilding::MaxRallySteps);
					}
				}
				else
				{
					Building->SetRallyPoint(Point, SourceIndex, Site);
					UE_LOG(LogHOLCommands, Display, TEXT("Rally: slot %d set the rally point of %s at (%.0f, %.0f)%s"), Order.PlayerIndex,
						*Building->GetTypeName().ToString(), Point.X, Point.Y, *OnWhat);
				}
			}
			continue;
		}

		// M4c: an exchange at a Market, not a queue item.
		if (Order.Kind == FHOLProductionCommand::EKind::Trade)
		{
			ApplyMarketTrade(Order, *Building, *PlayerState);
			continue;
		}
		if (Order.Kind == FHOLProductionCommand::EKind::Tribute)
		{
			ApplyTribute(Order, *Building, *PlayerState);
			continue;
		}

		FHOLResources Stock = PlayerState->GetResources();
		TArray<FHOLProductionItem>& Queue = Building->GetMutableProductionQueue();
		if (Order.Kind == FHOLProductionCommand::EKind::Queue)
		{
			// A technology (T3, T-Q6 = A) goes through the same queue as the units, after the rules of GDD 11.
			const FHOLTechnologyRow* Technology = Type->Produces.Contains(Order.TypeName) ? nullptr : UHOLTechnologySettings::FindTechnology(Order.TypeName);
			if (Technology)
			{
				const FHOLProductionCost Cost = GetCost(Order.TypeName, Order.PlayerIndex);
				HOLTechnology::FFacts Facts;
				const AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
				Facts.HeroTier = HOLGameState ? HOLGameState->GetTier(Order.PlayerIndex) : 0;
				Facts.HeroLevel = UHOLHeroLibrary::GetHeroLevel(this, Order.PlayerIndex);
				Facts.bRequiresDone = Technology->Requires.IsNone() || PlayerState->GetTechnologies().Has(Technology->Requires);
				Facts.bBuildingFinished = Building->GetTypeName() == Technology->Building;
				Facts.bCanPay = Stock.Covers(Cost.Resources);
				Facts.bResearched = PlayerState->GetTechnologies().Has(Order.TypeName);
				for (FHOLBuildingIterator It(GetWorld()); It && !Facts.bQueued; ++It)
				{
					Facts.bQueued = It->GetOwnerPlayer() == Order.PlayerIndex && HOLTechnology::IsQueued(It->GetProductionQueue(), Order.TypeName);
				}
				const HOLTechnology::EBlock Block = HOLTechnology::Check(*Technology, Facts);
				if (Block != HOLTechnology::EBlock::None)
				{
					UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cannot research %s (%s)"), Order.PlayerIndex,
						*Order.TypeName.ToString(), HOLTechnology::DescribeBlock(Block));
					continue;
				}
				const EHOLProductionRefusal Refusal = HOLProduction::TryEnqueue(Queue, Order.TypeName, Cost, Type->MaxQueue, Stock);
				if (Refusal != EHOLProductionRefusal::None)
				{
					UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cannot research %s (%s)"), Order.PlayerIndex,
						*Order.TypeName.ToString(), DescribeRefusal(Refusal));
					continue;
				}
				UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d queued research %s (%d in the queue, %s)"), Order.PlayerIndex,
					*Order.TypeName.ToString(), Queue.Num(), *Stock.ToString());
				PlayerState->SetResources(Stock);
				Building->NotifyProductionChanged();
				continue;
			}
			// S3 (GDD 6.1, D614): a siege engine opens at its tier (Ram 1, Mangonel 2), by the rules of Siege/HOLSiegeRules; the
			// building is finished (a site produces nothing) and the payment is the queue's own check below.
			HOLSiege::EEngine Engine;
			if (Type->Produces.Contains(Order.TypeName) && HOLSiege::FindEngine(Order.TypeName, Engine))
			{
				const AHOLGameState* HOLGameState = GetWorld()->GetGameState<AHOLGameState>();
				const HOLSiege::EBlock Block = HOLSiege::Check(Engine, HOLGameState ? HOLGameState->GetTier(Order.PlayerIndex) : 0, true, true);
				if (Block != HOLSiege::EBlock::None)
				{
					UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cannot queue %s (%s)"), Order.PlayerIndex,
						*Order.TypeName.ToString(), HOLSiege::DescribeBlock(Block));
					continue;
				}
			}
			// L0 (LQ-7): a unit type the owner's faction does not field is refused here whatever a client asked (its grid hides it).
			if (Type->Produces.Contains(Order.TypeName) && !HOLFactionRules::IsUnitOpenTo(GetWorld(), Order.TypeName, Order.PlayerIndex))
			{
				UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cannot queue %s (not a unit of its faction)"), Order.PlayerIndex,
					*Order.TypeName.ToString());
				continue;
			}
			const EHOLProductionRefusal Refusal = Type->Produces.Contains(Order.TypeName)
				? HOLProduction::TryEnqueue(Queue, Order.TypeName, GetCost(Order.TypeName, Order.PlayerIndex), Type->MaxQueue, Stock)
				: EHOLProductionRefusal::NotProducible;
			if (Refusal != EHOLProductionRefusal::None)
			{
				UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cannot queue %s (%s)"), Order.PlayerIndex,
					*Order.TypeName.ToString(), DescribeRefusal(Refusal));
				continue;
			}
			UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d queued %s (%d in the queue, %s)"), Order.PlayerIndex,
				*Order.TypeName.ToString(), Queue.Num(), *Stock.ToString());
		}
		else
		{
			const FName TypeName = Queue.IsValidIndex(Order.Index) ? Queue[Order.Index].TypeName : NAME_None;
			if (!HOLProduction::Cancel(Queue, Order.Index, GetCost(TypeName, Order.PlayerIndex), Stock))
			{
				continue;
			}
			UE_LOG(LogHOLCommands, Display, TEXT("Production: slot %d cancelled %s (%d in the queue, %s)"), Order.PlayerIndex,
				*TypeName.ToString(), Queue.Num(), *Stock.ToString());
		}
		PlayerState->SetResources(Stock);
		Building->NotifyProductionChanged();
	}
}

void UHOLProductionSubsystem::AdvanceQueues(const int64 StepIndex)
{
	UWorld* World = GetWorld();
	UHOLUnitSubsystem* Units = World->GetSubsystem<UHOLUnitSubsystem>();
	if (!Units)
	{
		return;
	}
	const auto GetCostOf = [this](const FName TypeName) { return GetCost(TypeName); };

	// Each player's cap (its buildings) and the places its units in production hold.
	TArray<AHOLBuilding*> Buildings;
	TMap<int32, int32> Caps;
	TMap<int32, int32> Started;
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		AHOLBuilding* Building = *It;
		const FHOLBuildingTypeRow* Type = Building->GetType();
		Buildings.Add(Building);
		// Only a finished building gives places (D312): a house's foundation gives none.
		Caps.FindOrAdd(Building->GetOwnerPlayer()) += Type ? HOLConstruction::GetPopulationProvided(Type->PopulationProvided, Building->IsFinished()) : 0;
		Started.FindOrAdd(Building->GetOwnerPlayer()) += HOLProduction::GetStartedPopulation(Building->GetProductionQueue(), GetCostOf);
	}
	for (const TPair<int32, int32>& Cheat : CheatPopulation)
	{
		if (int32* Cap = Caps.Find(Cheat.Key))
		{
			*Cap += Cheat.Value;
		}
	}
	// No building (the stress test's maps): nothing produces, no population to count.
	if (Buildings.IsEmpty())
	{
		return;
	}

	TArray<int32> UnitPopulation;
	Units->GetPopulationByOwner(UnitPopulation);
	TMap<int32, int32> FreePopulation;
	for (const TPair<int32, int32>& Cap : Caps)
	{
		const int32 Used = (UnitPopulation.IsValidIndex(Cap.Key) ? UnitPopulation[Cap.Key] : 0) + Started.FindRef(Cap.Key);
		FreePopulation.Add(Cap.Key, HOLProduction::GetPopulationCap(Cap.Value) - Used);
	}

	for (AHOLBuilding* Building : Buildings)
	{
		TArray<FHOLProductionItem>& Queue = Building->GetMutableProductionQueue();
		if (Queue.Num() == 0)
		{
			continue;
		}
		const TArray<FHOLProductionItem> Before = Queue;
		TArray<FName> Done;
		HOLProduction::Advance(Queue, StepIndex, GetCostOf, FreePopulation.FindOrAdd(Building->GetOwnerPlayer()), Done);
		for (const FName TypeName : Done)
		{
			// A research done (T3): his player has it, nothing is born.
			if (!Units->FindTypeRow(TypeName) && UHOLTechnologySettings::FindTechnology(TypeName))
			{
				// D719 (T0): in his player state, or in the record kept for him while he is away (given back on his return);
				// nowhere (no state, not away: a slot of nobody) is not logged as researched.
				const AHOLGameState* GameStateOfResearch = World->GetGameState<AHOLGameState>();
				const bool bAway = GameStateOfResearch && GameStateOfResearch->IsSlotAbsent(Building->GetOwnerPlayer());
				if (AHOLGameState::AddSlotTechnology(*World, Building->GetOwnerPlayer(), TypeName))
				{
					UE_LOG(LogHOLCommands, Display, TEXT("Technology: slot %d researched %s at step %lld%s"), Building->GetOwnerPlayer(),
						*TypeName.ToString(), StepIndex, bAway ? TEXT(" (away, kept for him)") : TEXT(""));
				}
				else
				{
					UE_LOG(LogHOLCommands, Display, TEXT("Technology: slot %d research of %s at step %lld went nowhere (no player state, not away, or had it)"),
						Building->GetOwnerPlayer(), *TypeName.ToString(), StepIndex);
				}
				continue;
			}
			int32& NumBorn = NumBirths.FindOrAdd(Building);
			// A tiny radius: the unit is put on the navmesh right under the point (FHOLUnitSpawnRequest).
			FHOLUnitSpawnRequest Request{ TypeName, 1, GetBirthLocation(*Building, NumBorn), 1.f, Building->GetOwnerPlayer() };
			// Born at its foot, then to the rally point if it has one (J4, D288).
			Request.bHasMoveTarget = Building->HasRallyPoint();
			Request.MoveTarget = Building->GetRallyPoint();
			Request.GatherSource = Building->HasRallyPoint() ? Building->GetRallySource() : INDEX_NONE;
			Request.BuildSite = Building->HasRallyPoint() ? Building->GetRallySite() : nullptr;
			// Its steps before the last one (EtapeDefini N.1): passed first, in order.
			for (int32 Step = 0; Building->HasRallyPoint() && Step < Building->GetRallyWaypoints().Num(); ++Step)
			{
				FHOLUnitSpawnWaypoint& Waypoint = Request.Waypoints.AddDefaulted_GetRef();
				Waypoint.Point = Building->GetRallyWaypoints()[Step].Point;
				Waypoint.GatherSource = Building->GetRallyWaypoints()[Step].Source;
				Waypoint.BuildSite = Building->GetRallyWaypointSite(Step);
			}
			Units->RequestUnits(Request);
			++NumBorn;
			if (UHOLMatchStatsSubsystem* MatchStats = World->GetSubsystem<UHOLMatchStatsSubsystem>())
			{
				MatchStats->RecordUnitTrained(Building->GetOwnerPlayer());
			}
			UE_LOG(LogHOLCommands, Display, TEXT("Production: %s of slot %d done at step %lld (%d born here)"), *TypeName.ToString(),
				Building->GetOwnerPlayer(), StepIndex, NumBorn);
		}
		if (Queue != Before)
		{
			Building->NotifyProductionChanged();
		}
	}

	// The population each player sees: its units (the births just asked for included) and its units in production. Every
	// player, not only those with a building: one left without any (a defeated one's collapse, D358) shows 0 / 0, not the
	// population it had before (seen at the trial of victory_surrender).
	Units->GetPopulationByOwner(UnitPopulation);
	const AGameStateBase* GameState = World->GetGameState();
	if (!GameState)
	{
		return;
	}
	for (APlayerState* Each : GameState->PlayerArray)
	{
		AHOLPlayerState* PlayerState = Cast<AHOLPlayerState>(Each);
		const int32 PlayerIndex = PlayerState ? PlayerState->GetPlayerIndex() : INDEX_NONE;
		if (PlayerIndex == INDEX_NONE)
		{
			continue;
		}
		int32 InProduction = 0;
		for (const AHOLBuilding* Building : Buildings)
		{
			if (Building->GetOwnerPlayer() == PlayerIndex)
			{
				InProduction += HOLProduction::GetStartedPopulation(Building->GetProductionQueue(), GetCostOf);
			}
		}
		const int32 Used = (UnitPopulation.IsValidIndex(PlayerIndex) ? UnitPopulation[PlayerIndex] : 0) + InProduction;
		PlayerState->SetPopulation(Used, HOLProduction::GetPopulationCap(Caps.FindRef(PlayerIndex)));
	}
}

FVector UHOLProductionSubsystem::GetBirthLocation(const AHOLBuilding& Building, const int32 Index) const
{
	using namespace UE::HeroesOfLiberty::Production;

	const FVector Origin = Building.GetActorLocation();
	const FHOLBuildingTypeRow* Type = Building.GetType();
	// At the foot of its body (D324), not of its tiles.
	const FVector2D Body = Type ? UHOLBuildingSettings::GetBodySize(Type->Footprint) : FVector2D(1200.0);
	const float HalfFootprint = 0.5f * static_cast<float>(Body.GetMax());

	// On the side of the rally point (user's choice at the J5a trial): the unit walks off at once, so one arc is enough.
	// Without one, below the building on the screen: the camera never turns and looks along +X for every player (D254),
	// so this side is never hidden behind the building (user's choice in J3).
	FVector Forward(-1.0, 0.0, 0.0);
	// Its first step, for a rally point in several (EtapeDefini N.1).
	const FVector ToRally = (Building.GetRallyFirstPoint() - Origin) * FVector(1.0, 1.0, 0.0);
	const bool bTowardRally = Building.HasRallyPoint() && ToRally.Size() > HalfFootprint;
	if (bTowardRally)
	{
		Forward = ToRally.GetSafeNormal();
	}
	const int32 Row = bTowardRally ? 0 : Index / BirthsPerRow;
	const int32 InRow = Index % BirthsPerRow;
	// 0, then +1, -1, +2, -2...: the arc fills from the middle.
	const int32 Side = (InRow + 1) / 2 * (InRow % 2 == 1 ? 1 : -1);
	const float Distance = HalfFootprint + BirthGap + Row * BirthRowGap;
	const FVector Direction = Forward.RotateAngleAxis(Side * BirthArcStep, FVector::UpVector);
	return Origin + Direction * Distance;
}
