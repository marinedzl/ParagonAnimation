#include "AnimNode_DistanceMatching.h"
#include "Animation/AnimInstanceProxy.h"

void FAnimNode_DistanceMatching::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);
}

void FAnimNode_DistanceMatching::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) 
{
}

void FAnimNode_DistanceMatching::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (Sequence)
	{
		InternalTimeAccumulator = FMath::Clamp(InternalTimeAccumulator, 0.f, Sequence->SequenceLength);
	}
}

void FAnimNode_DistanceMatching::Evaluate_AnyThread(FPoseContext& Output)
{
	check(Output.AnimInstanceProxy != nullptr);
	if ((Sequence != nullptr) && (Output.AnimInstanceProxy->IsSkeletonCompatible(Sequence->GetSkeleton())))
	{
		Sequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(InternalTimeAccumulator, Output.AnimInstanceProxy->ShouldExtractRootMotion()));
	}
	else
	{
		Output.ResetToRefPose();
	}
}

void FAnimNode_DistanceMatching::OverrideAsset(UAnimationAsset* NewAsset)
{
	if(UAnimSequenceBase* NewSequence = Cast<UAnimSequenceBase>(NewAsset))
	{
		Sequence = NewSequence;
	}
}

void FAnimNode_DistanceMatching::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += FString::Printf(TEXT("('%s' Play Time: %.3f)"), Sequence ? *Sequence->GetName() : TEXT("NULL"), InternalTimeAccumulator);
	DebugData.AddDebugItem(DebugLine, true);
}
