// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassPersistence/PersistableEntityConfigTrait.h"
#include "MassEntityTemplateRegistry.h"

void UPersistableEntityConfigTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FPersistableEntityConfigFragment>();
}
