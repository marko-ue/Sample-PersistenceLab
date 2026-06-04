// Copyright Epic Games, Inc. All Rights Reserved.

#include "PersistenceUtilsSettings.h"
#include "Mass/EntityElementTypes.h"
#include "UObject/UObjectIterator.h"

TArray<FString> UPersistenceUtilsSettings::GetMassFragmentOptions() const
{
	TArray<FString> Options;
	const UScriptStruct* MassFragmentBase = FMassFragment::StaticStruct();

	for (TObjectIterator<UScriptStruct> It; It; ++It)
	{
		UScriptStruct* S = *It;
		if (S != MassFragmentBase && S->IsChildOf(MassFragmentBase))
		{
			Options.Add(S->GetPathName());
		}
	}

	Options.Sort();
	return Options;
}
