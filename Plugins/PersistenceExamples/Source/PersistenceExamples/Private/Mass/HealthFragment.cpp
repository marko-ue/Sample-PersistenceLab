// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mass/HealthFragment.h"
#include "MassEntityTemplateRegistry.h"

void UHealthTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FHealthFragment>();
}
