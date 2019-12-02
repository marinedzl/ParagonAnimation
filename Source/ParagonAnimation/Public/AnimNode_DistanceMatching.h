#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Animation/AnimNode_AssetPlayerBase.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimSequenceDecompressionContext.h"
#include "AnimNode_DistanceMatching.generated.h"

USTRUCT()
struct PARAGONANIMATION_API FAnimNode_DistanceMatching : public FAnimNode_AssetPlayerBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	UAnimSequenceBase* Sequence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinHiddenByDefault))
	FName CurveName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings, meta = (PinShownByDefault))
	float Distance;

public:
	FAnimNode_DistanceMatching();

	// FAnimNode_AssetPlayerBase interface
	virtual float GetCurrentAssetTime();
	virtual float GetCurrentAssetLength();
	// End of FAnimNode_AssetPlayerBase interface

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void UpdateAssetPlayer(const FAnimationUpdateContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void OverrideAsset(UAnimationAsset* NewAsset) override;
	// End of FAnimNode_Base interface

	// FAnimNode_AssetPlayerBase Interface
	virtual UAnimationAsset* GetAnimAsset() { return Sequence; }
	// End of FAnimNode_AssetPlayerBase Interface
};
