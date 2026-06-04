// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/NPCMassFragments.h"
#include "MassEntityTemplateRegistry.h"

void UNPCNavTargetTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FNPCNavTargetFragment>();
}

void UNPCStateTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FNPCStateFragment>();
}
