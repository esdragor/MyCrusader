// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleRules.h"

// The pure morale rule (PLAN-MORAL-1120 step 2). The header says the rules; the script copy it replaces is Script/Morale/HOLMoraleRules.as.
namespace HOLMorale
{
	int32 BonusCap(int32 Lever)
	{
		// Damage dealt: 40 since Q-legions-rage-cap = A (2026-11-11), so the necrotic rage gives its whole +40 %.
		return Lever == LeverTaken ? 50 : (Lever == LeverDealt ? 40 : 30);
	}

	int32 MalusCap(int32 Lever)
	{
		// Damage dealt: stays 30, only the bonus rose to 40 (D791, "bonus only").
		return Lever == LeverTaken ? 50 : 30;
	}

	FHOLMoraleEffect MakeEffect(int32 Id, int32 Dealt, int32 Taken, int32 Rate, int32 DurationMs, bool bAura)
	{
		FHOLMoraleEffect Effect;
		Effect.Id = Id;
		Effect.Dealt = Dealt;
		Effect.Taken = Taken;
		Effect.Rate = Rate;
		Effect.DurationMs = DurationMs;
		Effect.bAura = bAura;
		return Effect;
	}

	FHOLMoraleEffect MakeEffectFromRow(int32 RowId)
	{
		const HOLMoraleTableRows::FHOLMoraleTableRow* Row = HOLMoraleTableRows::Find(RowId);
		if (Row == nullptr)
		{
			return FHOLMoraleEffect();
		}
		// A source that gives another row's levers (a banner gives the Dawn Aura) poses that row's effect, under that row's id.
		int32 EffectId = RowId;
		if (Row->Gives >= 0)
		{
			const HOLMoraleTableRows::FHOLMoraleTableRow* Given = HOLMoraleTableRows::Find(Row->Gives);
			if (Given == nullptr)
			{
				return FHOLMoraleEffect();
			}
			EffectId = Row->Gives;
			return MakeEffect(EffectId, Given->Dealt, Given->Taken, Given->Rate, Given->DurationMs, Given->bAura);
		}
		return MakeEffect(EffectId, Row->Dealt, Row->Taken, Row->Rate, Row->DurationMs, Row->bAura);
	}

	int32 LeverValue(const FHOLMoraleEffect& Effect, int32 Lever)
	{
		if (Lever == LeverDealt)
		{
			return Effect.Dealt;
		}
		if (Lever == LeverTaken)
		{
			return Effect.Taken;
		}
		if (Lever == LeverRate)
		{
			return Effect.Rate;
		}
		return 0;
	}

	bool IsMalus(const FHOLMoraleEffect& Effect)
	{
		return Effect.Dealt < 0 || Effect.Taken < 0 || Effect.Rate < 0;
	}

	int32 Scale(int32 Value, int32 FactorPercent)
	{
		const int32 Product = Value * FactorPercent;
		if (Product >= 0)
		{
			return (Product + 50) / 100;
		}
		return -((-Product + 50) / 100);
	}

	FHOLMoraleEffect WithIntensity(const FHOLMoraleEffect& Effect, int32 FactorPercent)
	{
		FHOLMoraleEffect Scaled = Effect;
		Scaled.Dealt = Scale(Effect.Dealt, FactorPercent);
		Scaled.Taken = Scale(Effect.Taken, FactorPercent);
		Scaled.Rate = Scale(Effect.Rate, FactorPercent);
		return Scaled;
	}

	bool HasIntensity(const FHOLMoraleEffect& Effect)
	{
		return Effect.Dealt != 0 || Effect.Taken != 0 || Effect.Rate != 0;
	}

	FHOLMoraleEffect Attenuated(const FHOLMoraleEffect& Effect, int32 AttenuationPercent)
	{
		if (AttenuationPercent <= 0 || !IsMalus(Effect))
		{
			return Effect;
		}
		const int32 Kept = 100 - FMath::Min(AttenuationPercent, AttenuationCap);
		if (Effect.bAura)
		{
			return WithIntensity(Effect, Kept);
		}
		FHOLMoraleEffect Shorter = Effect;
		Shorter.DurationMs = Scale(Effect.DurationMs, Kept);
		return Shorter;
	}

	int32 CombineAttenuation(const int32* Natures, const int32* Percents, int32 Count)
	{
		if (Natures == nullptr || Percents == nullptr)
		{
			return 0;
		}
		int32 Total = 0;
		for (int32 Index = 0; Index < Count; Index++)
		{
			// Counted at the first source of its nature, with the strongest of them.
			bool bFirst = true;
			int32 Best = Percents[Index];
			for (int32 Other = 0; Other < Count; Other++)
			{
				if (Other == Index || Natures[Other] != Natures[Index])
				{
					continue;
				}
				if (Other < Index)
				{
					bFirst = false;
				}
				Best = FMath::Max(Best, Percents[Other]);
			}
			if (bFirst && Best > 0)
			{
				Total += Best;
			}
		}
		return FMath::Min(Total, AttenuationCap);
	}

	bool Has(const FHOLMoraleState& State, int32 Id)
	{
		for (int32 Index = 0; Index < State.Num; Index++)
		{
			if (State.Effects[Index].Id == Id)
			{
				return true;
			}
		}
		return false;
	}

	namespace
	{
		// Of two benefit percents of one effect, the one with the larger magnitude.
		int32 Strongest(int32 A, int32 B)
		{
			return FMath::Abs(B) > FMath::Abs(A) ? B : A;
		}

		void RemoveAt(FHOLMoraleState& State, int32 Index)
		{
			for (int32 Next = Index + 1; Next < State.Num; Next++)
			{
				State.Effects[Next - 1] = State.Effects[Next];
			}
			State.Num--;
			State.Effects[State.Num] = FHOLMoraleEffect();
		}
	}

	int32 Apply(FHOLMoraleState& State, const FHOLMoraleEffect& Effect, int32 AttenuationPercent)
	{
		if (IsMalus(Effect) && State.ImmunityMs > 0)
		{
			return ApplyBlockedByImmunity;
		}
		const FHOLMoraleEffect Posed = Attenuated(Effect, AttenuationPercent);
		if (!HasIntensity(Posed) || (!Posed.bAura && Posed.DurationMs <= 0))
		{
			return ApplyNothing;
		}
		for (int32 Index = 0; Index < State.Num; Index++)
		{
			if (State.Effects[Index].Id != Posed.Id)
			{
				continue;
			}
			State.Effects[Index].Dealt = Strongest(State.Effects[Index].Dealt, Posed.Dealt);
			State.Effects[Index].Taken = Strongest(State.Effects[Index].Taken, Posed.Taken);
			State.Effects[Index].Rate = Strongest(State.Effects[Index].Rate, Posed.Rate);
			State.Effects[Index].DurationMs = FMath::Max(State.Effects[Index].DurationMs, Posed.DurationMs);
			return ApplyRefreshed;
		}
		if (State.Num >= MaxEffects)
		{
			return ApplyNoRoom;
		}
		State.Effects[State.Num] = Posed;
		State.Num++;
		return ApplyPosed;
	}

	int32 Total(const FHOLMoraleState& State, int32 Lever)
	{
		int32 Bonus = 0;
		int32 Malus = 0;
		for (int32 Index = 0; Index < State.Num; Index++)
		{
			const int32 Value = LeverValue(State.Effects[Index], Lever);
			if (Value > 0)
			{
				Bonus += Value;
			}
			else
			{
				Malus -= Value;
			}
		}
		return FMath::Min(Bonus, BonusCap(Lever)) - FMath::Min(Malus, MalusCap(Lever));
	}

	int32 TakenPercent(const FHOLMoraleState& State, int32 OtherReductionPercent)
	{
		const int32 Net = Total(State, LeverTaken) + OtherReductionPercent;
		return 100 - FMath::Min(Net, BonusCap(LeverTaken));
	}

	int32 Multiplier(const FHOLMoraleState& State, int32 Lever)
	{
		const int32 Net = Total(State, Lever);
		return Lever == LeverTaken ? 100 - Net : 100 + Net;
	}

	void Tick(FHOLMoraleState& State, int32 ElapsedMs)
	{
		for (int32 Index = State.Num - 1; Index >= 0; Index--)
		{
			if (State.Effects[Index].bAura)
			{
				continue;
			}
			State.Effects[Index].DurationMs -= ElapsedMs;
			if (State.Effects[Index].DurationMs <= 0)
			{
				RemoveAt(State, Index);
			}
		}
		State.ImmunityMs = FMath::Max(0, State.ImmunityMs - ElapsedMs);
	}

	void ClearAuras(FHOLMoraleState& State)
	{
		for (int32 Index = State.Num - 1; Index >= 0; Index--)
		{
			if (State.Effects[Index].bAura)
			{
				RemoveAt(State, Index);
			}
		}
	}

	int32 RemoveMalus(FHOLMoraleState& State)
	{
		int32 Removed = 0;
		for (int32 Index = State.Num - 1; Index >= 0; Index--)
		{
			if (IsMalus(State.Effects[Index]))
			{
				RemoveAt(State, Index);
				Removed++;
			}
		}
		return Removed;
	}

	int32 RemoveById(FHOLMoraleState& State, int32 Id)
	{
		int32 Removed = 0;
		for (int32 Index = State.Num - 1; Index >= 0; Index--)
		{
			if (State.Effects[Index].Id == Id)
			{
				RemoveAt(State, Index);
				Removed++;
			}
		}
		return Removed;
	}

	int32 ClearMalusAndProtect(FHOLMoraleState& State)
	{
		const int32 Removed = RemoveMalus(State);
		GrantImmunity(State, SacredLightImmunityMs);
		return Removed;
	}

void GrantImmunity(FHOLMoraleState& State, int32 DurationMs)
	{
		State.ImmunityMs = FMath::Max(State.ImmunityMs, DurationMs);
	}
}

namespace HOLMorale
{
	bool IsEmpty(const FHOLMoraleState& State)
	{
		return State.Num == 0 && State.ImmunityMs == 0;
	}

	uint16 EffectMask(const FHOLMoraleState& State)
	{
		uint16 Mask = 0;
		for (int32 Slot = 0; Slot < State.Num && Slot < MaxEffects; ++Slot)
		{
			const int32 Id = State.Effects[Slot].Id;
			if (Id >= 0 && Id < 16)
			{
				Mask |= static_cast<uint16>(1u << Id);
			}
		}
		return Mask;
	}

	uint64 FingerprintKey(const FHOLMoraleState& State)
	{
		if (IsEmpty(State))
		{
			return 0;
		}
		uint64 Hash = 14695981039346656037ull;
		const auto Mix = [&Hash](const int32 Value)
		{
			const uint32 Bits = static_cast<uint32>(Value);
			for (int32 Byte = 0; Byte < 4; ++Byte)
			{
				Hash = (Hash ^ ((Bits >> (Byte * 8)) & 0xFFu)) * 1099511628211ull;
			}
		};
		Mix(State.Num);
		Mix(State.ImmunityMs);
		for (int32 Slot = 0; Slot < State.Num && Slot < MaxEffects; ++Slot)
		{
			const FHOLMoraleEffect& Effect = State.Effects[Slot];
			Mix(Effect.Id);
			Mix(Effect.Dealt);
			Mix(Effect.Taken);
			Mix(Effect.Rate);
			Mix(Effect.DurationMs);
			Mix(Effect.bAura ? 1 : 0);
		}
		return Hash != 0 ? Hash : 1;
	}
}
