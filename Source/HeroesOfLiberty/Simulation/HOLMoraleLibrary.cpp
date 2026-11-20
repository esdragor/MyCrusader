// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLMoraleLibrary.h"

// STUB (red test first): compiles, changes nothing, returns 0 or an empty effect. The body comes in the next commit.

int32 UHOLMoraleLibrary::NativeAttenuationCap() { return 0; }
int32 UHOLMoraleLibrary::NativeBonusCap(const int32) { return 0; }
int32 UHOLMoraleLibrary::NativeMalusCap(const int32) { return 0; }
FHOLMoraleEffectData UHOLMoraleLibrary::NativeMakeEffect(const int32, const int32, const int32, const int32, const int32, const bool) { return FHOLMoraleEffectData(); }
FHOLMoraleEffectData UHOLMoraleLibrary::NativeMakeEffectFromRow(const int32) { return FHOLMoraleEffectData(); }
int32 UHOLMoraleLibrary::NativeLeverValue(const FHOLMoraleEffectData&, const int32) { return 0; }
bool UHOLMoraleLibrary::NativeIsMalus(const FHOLMoraleEffectData&) { return false; }
int32 UHOLMoraleLibrary::NativeScale(const int32, const int32) { return 0; }
FHOLMoraleEffectData UHOLMoraleLibrary::NativeWithIntensity(const FHOLMoraleEffectData& Effect, const int32) { return Effect; }
bool UHOLMoraleLibrary::NativeHasIntensity(const FHOLMoraleEffectData&) { return false; }
FHOLMoraleEffectData UHOLMoraleLibrary::NativeAttenuated(const FHOLMoraleEffectData& Effect, const int32) { return Effect; }
int32 UHOLMoraleLibrary::NativeCombineAttenuation(const TArray<int32>&, const TArray<int32>&) { return 0; }
bool UHOLMoraleLibrary::NativeHas(const FHOLMoraleStateData&, const int32) { return false; }
int32 UHOLMoraleLibrary::NativeApply(FHOLMoraleStateData&, const FHOLMoraleEffectData&, const int32) { return 0; }
int32 UHOLMoraleLibrary::NativeTotal(const FHOLMoraleStateData&, const int32) { return 0; }
int32 UHOLMoraleLibrary::NativeMultiplier(const FHOLMoraleStateData&, const int32) { return 0; }
void UHOLMoraleLibrary::NativeTick(FHOLMoraleStateData&, const int32) {}
void UHOLMoraleLibrary::NativeClearAuras(FHOLMoraleStateData&) {}
int32 UHOLMoraleLibrary::NativeRemoveMalus(FHOLMoraleStateData&) { return 0; }
void UHOLMoraleLibrary::NativeGrantImmunity(FHOLMoraleStateData&, const int32) {}
