// Copyright Epic Games, Inc. All Rights Reserved.

#include "Data/PersistableEntityConfigFragment.h"
#include "MassEntityTemplateRegistry.h"

void UPersistableEntityConfigTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FPersistableEntityConfigFragment>();
}
