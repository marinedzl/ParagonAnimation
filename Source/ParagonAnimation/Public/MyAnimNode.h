// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "Animation/AnimNodeBase.h"
#include "MyAnimNode.generated.h"
/**
 * 
 */

USTRUCT(BlueprintType)
struct PARAGONANIMATION_API FMyAnimNode : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links)
	FPoseLink BasePose;
	
public:
	FMyAnimNode();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext & Output) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface
};
