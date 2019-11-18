#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "Animation/AnimSequenceBase.h"
#include "AnimNode_DistanceMatching.generated.h"

USTRUCT(BlueprintType)
struct PARAGONANIMATION_API FAnimNode_DistanceMatching : public FAnimNode_Base
{
	GENERATED_BODY()
public:
	// The animation sequence asset to evaluate
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	UAnimSequenceBase* Sequence;

	/** Accumulated time used to reference the asset in this node */
	UPROPERTY(BlueprintReadWrite, Transient, Category = DoNotEdit)
	float InternalTimeAccumulator;

public:
	FAnimNode_DistanceMatching()
		: Sequence(nullptr)
		, InternalTimeAccumulator(0.0f)
	{
	}

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void OverrideAsset(UAnimationAsset* NewAsset) override;
	virtual void GatherDebugData(FNodeDebugData& DebugData) override;
	// End of FAnimNode_Base interface
};
