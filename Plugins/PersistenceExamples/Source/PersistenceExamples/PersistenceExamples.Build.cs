// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PersistenceExamples : ModuleRules
{
	public PersistenceExamples(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"Chaos",
				"GameplayAbilities",
				"GeometryCollectionEngine",
				"PersistenceUtils",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"AIModule",
				"GameplayTags",
				"GameplayTasks",
				"StateTreeModule",
				"GameplayStateTreeModule",
				"Json",
				"JsonUtilities",
				// InstancedActors / Mass dependencies
				"InstancedActors",
				"MassEntity",
				"MassCommon",
				"MassRepresentation",
				"MassActors",
				"MassSpawner",
				"MassSimulation",
                "MassCore",
				// Mass StateTree integration (NPCNavTargetEvaluator)
				"StateTreeModule",
				"MassAIBehavior",
				"MassNavigation",
				"MassSignals",
			}
			);
	}
}
