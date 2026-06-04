// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PersistenceUtils : ModuleRules
{
	public PersistenceUtils(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"MassEntity",    // FMassEntityHandle and FMassArchetypeHandle are exposed in MassPersistenceUtils.h
				"MassCore"
            }
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"DeveloperSettings",
				"Engine",
				"EngineSettings",
				"Slate",
				"SlateCore",
				"LevelStreamingPersistence",
				"InstancedActors",
				"MassActors",
				"MassSpawner",
				"MassSimulation",
				"MassCore",
				"GameplayDebugger"
				// ... add private dependencies that you statically link with here ...
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
