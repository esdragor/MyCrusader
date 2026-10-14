// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

// Dedicated server (D231): needs an engine built with the Server target (UE 5.8 AS 3).
public class HeroesOfLibertyServerTarget : TargetRules
{
	public HeroesOfLibertyServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.AddRange( new string[] { "HeroesOfLiberty" } );
	}
}
