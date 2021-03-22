#include "AnimGraphNode_DistanceMatching.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Kismet2/CompilerResultsLog.h"
#include "GraphEditorActions.h"
#include "Animation/AnimComposite.h"

#define LOCTEXT_NAMESPACE "A3Nodes"

void UAnimGraphNode_DistanceMatching::PreloadRequiredAssets()
{
	PreloadObject(Node.Sequence);

	Super::PreloadRequiredAssets();
}

void UAnimGraphNode_DistanceMatching::BakeDataDuringCompilation(class FCompilerResultsLog& MessageLog)
{
	UAnimBlueprint* AnimBlueprint = GetAnimBlueprint();
	AnimBlueprint->FindOrAddGroup(SyncGroup.GroupName);
	Node.GroupName = SyncGroup.GroupName;
	Node.GroupRole = SyncGroup.GroupRole;
	Node.GroupScope = SyncGroup.GroupScope;
}

void UAnimGraphNode_DistanceMatching::OnProcessDuringCompilation(IAnimBlueprintCompilationContext& InCompilationContext, IAnimBlueprintGeneratedClassCompiledData& OutCompiledData)
{
}

void UAnimGraphNode_DistanceMatching::GetAllAnimationSequencesReferred(TArray<UAnimationAsset*>& AnimationAssets) const
{
	if (Node.Sequence)
	{
		HandleAnimReferenceCollection(Node.Sequence, AnimationAssets);
	}
}

void UAnimGraphNode_DistanceMatching::ReplaceReferredAnimations(const TMap<UAnimationAsset*, UAnimationAsset*>& AnimAssetReplacementMap)
{
	HandleAnimReferenceReplacement(Node.Sequence, AnimAssetReplacementMap);
}

FText UAnimGraphNode_DistanceMatching::GetTooltipText() const
{
	// FText::Format() is slow, so we utilize the cached list title
	return GetNodeTitle(ENodeTitleType::ListView);
}

FText UAnimGraphNode_DistanceMatching::GetNodeTitleForSequence(ENodeTitleType::Type TitleType, UAnimSequenceBase* InSequence) const
{
	const FText SequenceName = FText::FromString(InSequence->GetName());

	FFormatNamedArguments Args;
	Args.Add(TEXT("SequenceName"), SequenceName);

	// FText::Format() is slow, so we cache this to save on performance
	if (InSequence->IsValidAdditive())
	{
		CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("DistanceMatching_Additive", "DistanceMatching {SequenceName} (additive)"), Args), this);
	}
	else
	{
		CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("DistanceMatching", "DistanceMatching {SequenceName}"), Args), this);
	}

	return CachedNodeTitle;
}

FText UAnimGraphNode_DistanceMatching::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (Node.Sequence == nullptr)
	{
		return LOCTEXT("DistanceMatching", "DistanceMatching");
	}
	else
	{
		GetNodeTitleForSequence(TitleType, Node.Sequence);
	}

	return CachedNodeTitle;
}

FString UAnimGraphNode_DistanceMatching::GetNodeCategory() const
{
	return TEXT("Distance Matching");
}

void UAnimGraphNode_DistanceMatching::SetAnimationAsset(UAnimationAsset* Asset)
{
	if (UAnimSequenceBase* Seq = Cast<UAnimSequence>(Asset))
	{
		Node.Sequence = Seq;
	}
}

void UAnimGraphNode_DistanceMatching::ValidateAnimNodeDuringCompilation(class USkeleton* ForSkeleton, class FCompilerResultsLog& MessageLog)
{
	Super::ValidateAnimNodeDuringCompilation(ForSkeleton, MessageLog);

	UAnimSequenceBase* SequenceToCheck = Node.Sequence;
	UEdGraphPin* SequencePin = FindPin(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_DistanceMatching, Sequence));
	if (SequencePin != nullptr && SequenceToCheck == nullptr)
	{
		SequenceToCheck = Cast<UAnimSequenceBase>(SequencePin->DefaultObject);
	}

	if (SequenceToCheck == nullptr)
	{
		// we may have a connected node
		if (SequencePin == nullptr || SequencePin->LinkedTo.Num() == 0)
		{
			MessageLog.Error(TEXT("@@ references an unknown sequence"), this);
		}
	}
	else
	{
		USkeleton* SeqSkeleton = SequenceToCheck->GetSkeleton();
		if (SeqSkeleton && // if anim sequence doesn't have skeleton, it might be due to anim sequence not loaded yet, @todo: wait with anim blueprint compilation until all assets are loaded?
			!SeqSkeleton->IsCompatible(ForSkeleton))
		{
			MessageLog.Error(TEXT("@@ references sequence that uses different skeleton @@"), this, SeqSkeleton);
		}
	}
}

bool UAnimGraphNode_DistanceMatching::DoesSupportTimeForTransitionGetter() const
{
	return true;
}

UAnimationAsset* UAnimGraphNode_DistanceMatching::GetAnimationAsset() const
{
	UAnimSequenceBase* Sequence = Node.Sequence;
	UEdGraphPin* SequencePin = FindPin(GET_MEMBER_NAME_STRING_CHECKED(FAnimNode_DistanceMatching, Sequence));
	if (SequencePin != nullptr && Sequence == nullptr)
	{
		Sequence = Cast<UAnimSequenceBase>(SequencePin->DefaultObject);
	}

	return Node.Sequence;
}

const TCHAR* UAnimGraphNode_DistanceMatching::GetTimePropertyName() const
{
	return TEXT("ExplicitTime");
}

UScriptStruct* UAnimGraphNode_DistanceMatching::GetTimePropertyStruct() const
{
	return FAnimNode_DistanceMatching::StaticStruct();
}

EAnimAssetHandlerType UAnimGraphNode_DistanceMatching::SupportsAssetClass(const UClass* AssetClass) const
{
	if (AssetClass->IsChildOf(UAnimSequence::StaticClass()) || AssetClass->IsChildOf(UAnimComposite::StaticClass()))
	{
		return EAnimAssetHandlerType::Supported;
	}
	else
	{
		return EAnimAssetHandlerType::NotSupported;
	}
}

#undef LOCTEXT_NAMESPACE
