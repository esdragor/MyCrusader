// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLBiomeReadLibrary.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/Engine.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

namespace
{
	/** The key of the biome of Game's match: its configuration's, "default" when it has none (an empty key from a corrupt file included). */
	FString GetBiomeKeyOf(const AHOLGameState* GameState)
	{
		const FString Key = GameState ? GameState->GetLaunchConfig().Biome.TrimStartAndEnd() : FString();
		return Key.IsEmpty() ? FString(TEXT("default")) : Key;
	}

	/**
	 * hol.Biome.Report: "Biome: snow (server)" for each game world of this process (an editor's server and a client may share one),
	 * what the net tests read in the server's and in each client's log to see that everyone holds the lobby's biome.
	 */
	FAutoConsoleCommand CommandBiomeReport(TEXT("hol.Biome.Report"),
		TEXT("The biome key of the match this process plays, one line per game world: 'Biome: snow (client)'"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				const UWorld* World = Context.World();
				if (!World || !World->IsGameWorld())
				{
					continue;
				}
				const TCHAR* Role = World->GetNetMode() == NM_Client ? TEXT("client") : (World->GetNetMode() == NM_Standalone ? TEXT("standalone") : TEXT("server"));
				UE_LOG(LogHOLMatch, Display, TEXT("Biome: %s (%s)"), *GetBiomeKeyOf(World->GetGameState<AHOLGameState>()), Role);
			}
		}));
	/**
	 * hol.Biome.World (B1 visual): what the LOADED WORLD holds of the biome's look, read on the lights, the haze and the ground of each
	 * game world of this process: "Biome world: sun=7.5 sunPitch=-40 sky=1 fog=0.012 groundA=0.1,0.2,0.1". Independent of the script that
	 * sets it (UHOLBiomeApplier), so a test reads the effect, not the script's claim; Tools/Tests/biome_apply_check.ps1 compares it with
	 * the "Biome expect:" line the applier writes.
	 */
	FAutoConsoleCommand CommandBiomeWorld(TEXT("hol.Biome.World"),
		TEXT("The look the loaded world holds: 'Biome world: sun=... sunPitch=... sky=... fog=... groundA=...'"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (!World || !World->IsGameWorld())
				{
					continue;
				}
				FString Line = TEXT("Biome world:");
				for (TActorIterator<ADirectionalLight> It(World); It; ++It)
				{
					if (const ULightComponent* Light = It->FindComponentByClass<ULightComponent>())
					{
						Line += FString::Printf(TEXT(" sun=%.4f sunPitch=%.2f"), Light->Intensity, It->GetActorRotation().Pitch);
					}
					break;
				}
				for (TActorIterator<ASkyLight> It(World); It; ++It)
				{
					if (const USkyLightComponent* Sky = It->FindComponentByClass<USkyLightComponent>())
					{
						Line += FString::Printf(TEXT(" sky=%.4f"), Sky->Intensity);
					}
					break;
				}
				for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
				{
					if (const UExponentialHeightFogComponent* Fog = It->FindComponentByClass<UExponentialHeightFogComponent>())
					{
						Line += FString::Printf(TEXT(" fog=%.4f"), Fog->FogDensity);
					}
					break;
				}
				for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
				{
					const UStaticMeshComponent* Mesh = It->GetStaticMeshComponent();
					UMaterialInterface* Material = Mesh ? Mesh->GetMaterial(0) : nullptr;
					const UMaterial* Base = Material ? Material->GetBaseMaterial() : nullptr;
					FLinearColor GroundA;
					if (Base && Base->GetName() == TEXT("M_GroundVaried") && Material->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("ColorA")), GroundA))
					{
						Line += FString::Printf(TEXT(" groundA=%.4f,%.4f,%.4f"), GroundA.R, GroundA.G, GroundA.B);
						break;
					}
				}
				UE_LOG(LogHOLMatch, Display, TEXT("%s"), *Line);
			}
		}));
}

FString UHOLBiomeReadLibrary::GetMatchBiomeKey(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return GetBiomeKeyOf(World ? World->GetGameState<AHOLGameState>() : nullptr);
}
