// Copyright Epic Games, Inc. All Rights Reserved.

#include "InstancedActors/ExampleInstancedActorVersion.h"
#include "Serialization/CustomVersion.h"

const FGuid FExampleInstancedActorVersion::GUID(0x2F8B6D14, 0xA47C42E9, 0x91D5C30B, 0x6E1A4F87);

// Registers LatestVersion once per module in the process-global custom-version registry. A saving
// archive reads the number from here; a loading archive uses whatever was stored in the blob instead
// (the transport persists the FCustomVersionContainer alongside the payload).
static FCustomVersionRegistration GRegisterExampleInstancedActorVersion(
	FExampleInstancedActorVersion::GUID,
	FExampleInstancedActorVersion::LatestVersion,
	TEXT("ExampleInstancedActorVersion"));
