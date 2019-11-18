#include "MyAnimNode.h"
#include "Animation/AnimInstanceProxy.h"


FMyAnimNode::FMyAnimNode()
{
}

void FMyAnimNode::Initialize_AnyThread(const FAnimationInitializeContext & Context)
{
	FAnimNode_Base::Initialize_AnyThread(Context);

	BasePose.Initialize(Context);
}

void FMyAnimNode::CacheBones_AnyThread(const FAnimationCacheBonesContext & Context)
{
	BasePose.CacheBones(Context);
}

void FMyAnimNode::Update_AnyThread(const FAnimationUpdateContext & Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);
	BasePose.Update(Context);
}


void FMyAnimNode::Evaluate_AnyThread(FPoseContext & Output)
{
	// Evaluate the input
	BasePose.Evaluate(Output);
}

void FMyAnimNode::GatherDebugData(FNodeDebugData & DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);


	DebugData.AddDebugItem(DebugLine);

	BasePose.GatherDebugData(DebugData);
}

