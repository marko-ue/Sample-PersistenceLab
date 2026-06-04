// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/PersistedStateTreeComponent.h"

#include "Algo/Reverse.h"
#include "StateTree.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeInstanceData.h"
#include "StateTreeTypes.h"
#include "StructUtils/PropertyBag.h"

FGuid UPersistedStateTreeComponent::GetActiveLeafStateId()
{
	if (GetStateTreeRunStatus() != EStateTreeRunStatus::Running)
	{
		return FGuid();
	}

	// Resolve dependencies (asset and runtime owner)
	const UStateTree* StateTree = StateTreeRef.GetStateTree();
	AActor* Owner = GetOwner();
	if (!StateTree || !Owner)
	{
		return FGuid();
	}

	FStateTreeReadOnlyExecutionContext Context(Owner, StateTree, InstanceData);
	const TConstArrayView<FStateTreeExecutionFrame> Frames = Context.GetActiveFrames();
	if (!Frames.IsEmpty())
	{
		const FStateTreeExecutionFrame& LeafFrame = Frames.Last();
		const FStateTreeStateHandle LeafState = LeafFrame.ActiveStates.Last();
		if (LeafFrame.StateTree && LeafState.IsValid())
		{
			// Get GUID of leaf state. This is enough to force transition.
			return LeafFrame.StateTree->GetStateIdFromHandle(LeafState);
		}
	}

	return FGuid();
}

EStateTreeRunStatus UPersistedStateTreeComponent::ForceTransitionToState(const FGuid StateId)
{
	if (!StateId.IsValid() || GetStateTreeRunStatus() != EStateTreeRunStatus::Running)
	{
		return EStateTreeRunStatus::Failed;
	}

	const UStateTree* StateTree = StateTreeRef.GetStateTree();
	AActor* Owner = GetOwner();
	if (!StateTree || !Owner)
	{
		return EStateTreeRunStatus::Failed;
	}

	const FStateTreeStateHandle Handle = StateTree->GetStateHandleFromId(/*FGuid=*/StateId);
	if (Handle.IsValid())
	{
		FStateTreeExecutionContext Context(*Owner, *StateTree, InstanceData,
			FOnCollectStateTreeExternalData::CreateUObject(this, &UPersistedStateTreeComponent::CollectExternalData));
		if (SetContextRequirements(Context))
		{
			// Construct a FRecordedStateTreeTransitionResult by reconstructing a path to the targeted leaf state
			// represented by StateId.
			TConstArrayView<FCompactStateTreeState> CompactStates = StateTree->GetStates();
			TArray<FStateTreeStateHandle> Path;
			Path.Reserve(8);
			for (FStateTreeStateHandle Current = Handle; Current.IsValid(); Current = CompactStates[Current.Index].Parent)
			{
				Path.Add(Current);
			}
			Algo::Reverse(Path);

			FRecordedStateTreeTransitionResult Recorded;
			Recorded.Priority = EStateTreeTransitionPriority::Normal;
			Recorded.States.Reserve(Path.Num());
			for (const FStateTreeStateHandle& PathState : Path)
			{
				FRecordedActiveState& Entry = Recorded.States.AddDefaulted_GetRef();
				Entry.StateTree = StateTree;
				Entry.State = PathState;
			}

			return Context.ForceTransition(Recorded);
		}
	}

	return EStateTreeRunStatus::Failed;
}

UObject* UPersistedStateTreeComponent::GetGlobalObject(FName Name) const
{
	// The storage holds the property bag's dynamically-generated *value* struct, not an
	// FInstancedPropertyBag wrapper. Read named fields via property reflection on the value struct.
	const FConstStructView View = InstanceData.GetStorage().GetGlobalParameters();
	const UScriptStruct* Struct = View.GetScriptStruct();
	if (!Struct)
	{
		return nullptr;
	}

	const FObjectProperty* ObjProp = CastField<const FObjectProperty>(Struct->FindPropertyByName(Name));
	if (!ObjProp)
	{
		return nullptr;
	}

	return ObjProp->GetObjectPropertyValue_InContainer(View.GetMemory());
}

EPropertyBagResult UPersistedStateTreeComponent::SetGlobalObject(FName Name, UObject* Object)
{
	const FStructView View = InstanceData.GetMutableStorage().GetMutableGlobalParameters();
	const UScriptStruct* Struct = View.GetScriptStruct();
	if (!Struct)
	{
		return EPropertyBagResult::PropertyNotFound;
	}

	const FProperty* Prop = Struct->FindPropertyByName(Name);
	if (!Prop)
	{
		return EPropertyBagResult::PropertyNotFound;
	}

	const FObjectProperty* ObjProp = CastField<const FObjectProperty>(Prop);
	if (!ObjProp)
	{
		return EPropertyBagResult::TypeMismatch;
	}

	if (Object && ObjProp->PropertyClass && !Object->IsA(ObjProp->PropertyClass))
	{
		return EPropertyBagResult::TypeMismatch;
	}

	ObjProp->SetObjectPropertyValue_InContainer(View.GetMemory(), Object);
	return EPropertyBagResult::Success;
}
