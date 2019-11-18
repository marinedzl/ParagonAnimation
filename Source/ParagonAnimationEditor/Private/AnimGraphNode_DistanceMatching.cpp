#include "AnimGraphNode_DistanceMatching.h"

#define LOCTEXT_NAMESPACE "A3Nodes"

UAnimGraphNode_DistanceMatching::UAnimGraphNode_DistanceMatching(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

}

FLinearColor UAnimGraphNode_DistanceMatching::GetNodeTitleColor() const
{
	return FLinearColor::Green;
}

FText UAnimGraphNode_DistanceMatching::GetTooltipText() const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

FText UAnimGraphNode_DistanceMatching::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (Node.Sequence == nullptr)
	{
		return LOCTEXT("DistanceMatching_TitleNONE", "Matching (None)");
	}
	else
	{
		const FText SequenceName = FText::FromString(Node.Sequence->GetName());

		FFormatNamedArguments Args;
		Args.Add(TEXT("SequenceName"), SequenceName);

		CachedNodeTitle.SetCachedText(FText::Format(LOCTEXT("DistanceMatching", "Matching {SequenceName}"), Args), this);

		return CachedNodeTitle;
	}
}

FString UAnimGraphNode_DistanceMatching::GetNodeCategory() const
{
	return TEXT("DistanceMatching");
}

#undef LOCTEXT_NAMESPACE
