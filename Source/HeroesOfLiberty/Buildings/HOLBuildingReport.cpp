// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerColors.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	/** True when a piece of the building is painted with its colour index's colour (the tower's roof, tone 1, D261). */
	bool ShowsItsColor(const AHOLBuilding& Building)
	{
		const FLinearColor Expected(HOLPlayerColors::GetLinearColor(Building.GetColorIndex()));
		TArray<UStaticMeshComponent*> Pieces;
		Building.GetComponents(Pieces);
		for (const UStaticMeshComponent* Piece : Pieces)
		{
			const UMaterialInstanceDynamic* Material = Cast<UMaterialInstanceDynamic>(Piece->GetMaterial(0));
			FLinearColor Shown;
			if (Material && Material->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Color")), Shown)
				&& Shown.Equals(Expected, 0.001f))
			{
				return true;
			}
		}
		return false;
	}

	/** Scripted checks of the two-client match (D273, M2.5.6): the buildings a machine has received, each with its
	 *  owner's slot, its colour index and whether that colour is shown. */
	FAutoConsoleCommand BuildingsReportCommand(
		TEXT("hol.Buildings.Report"),
		TEXT("hol.Buildings.Report: logs the buildings of the game worlds, their slot, colour and whether it is shown"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				if (!World || !World->IsGameWorld())
				{
					continue;
				}
				int32 Count = 0;
				for (TActorIterator<AHOLBuilding> It(World); It; ++It, ++Count)
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Building report: %s of slot %d, colour %d %s"), *It->GetTypeName().ToString(),
						It->GetOwnerPlayer(), It->GetColorIndex(), ShowsItsColor(**It) ? TEXT("shown") : TEXT("not shown"));
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Building report: %d buildings"), Count);
			}
		}));
}
