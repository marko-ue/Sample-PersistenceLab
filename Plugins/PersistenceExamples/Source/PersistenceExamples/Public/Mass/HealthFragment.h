// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "HealthFragment.generated.h"

USTRUCT()
struct FHealthFragment : public FMassFragment
{
	GENERATED_BODY()

	float Current = 100.f;
	float Max = 100.f;
};

UCLASS(meta = (DisplayName = "Health"))
class UHealthTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
