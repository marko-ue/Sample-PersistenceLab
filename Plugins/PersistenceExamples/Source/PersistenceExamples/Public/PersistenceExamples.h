// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Logging/LogMacros.h"

PERSISTENCEEXAMPLES_API DECLARE_LOG_CATEGORY_EXTERN(LogPersistenceExamples, Log, All);

class FPersistenceExamplesModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
