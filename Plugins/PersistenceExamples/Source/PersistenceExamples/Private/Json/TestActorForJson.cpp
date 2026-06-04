// Copyright Epic Games, Inc. All Rights Reserved.

#include "Json/TestActorForJson.h"
#include "Json/ExampleJsonType.h"
#include "PersistenceExamples.h"
#include "JsonObjectConverter.h"

void ATestActorForJson::BeginPlay()
{
	Super::BeginPlay();

	// Example: you can introduce a struct that you can serialize to/from JSON string at will.
	// The JSON string can be saved to a file, or be the record for an actor/component in your SaveGame.
	FExampleJsonDataStruct StructToSave;
	StructToSave.Health = GetHealth();
	StructToSave.MaxHealth = GetMaxHealth();
	StructToSave.TargetID = GetTargetID();

	// Struct to JSON
	FString JsonString;
	const bool bSaveSuccess = FJsonObjectConverter::UStructToJsonObjectString(StructToSave, JsonString);

	// JSON to struct
	FExampleJsonDataStruct LoadedStruct;
	const bool bLoadSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &LoadedStruct);

	if (!FJsonObjectConverter::UStructToJsonObjectString(StructToSave, JsonString))
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("UStructToJsonObjectString failed"));
		return;
	}
	UE_LOG(LogPersistenceExamples, Display, TEXT("Struct -> JSON:\n%s"), *JsonString);

	if (!FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &LoadedStruct))
	{
		UE_LOG(LogPersistenceExamples, Warning, TEXT("JsonObjectStringToUStruct failed"));
		return;
	}
	UE_LOG(LogPersistenceExamples, Display, TEXT("JSON -> struct: Health=%.1f MaxHealth=%.1f NPCID=%s"), LoadedStruct.Health, LoadedStruct.MaxHealth, *LoadedStruct.TargetID.ToString());
}
