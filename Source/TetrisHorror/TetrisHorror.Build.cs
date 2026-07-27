// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class TetrisHorror : ModuleRules
{
	public TetrisHorror(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"TetrisHorror",
			"TetrisHorror/Variant_Horror",
			"TetrisHorror/Variant_Horror/UI",
			"TetrisHorror/Variant_Shooter",
			"TetrisHorror/Variant_Shooter/AI",
			"TetrisHorror/Variant_Shooter/UI",
			"TetrisHorror/Variant_Shooter/Weapons"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
