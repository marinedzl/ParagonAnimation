#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_DistanceMatching.h"
#include "AnimGraphNode_DistanceMatching.generated.h"

class FBlueprintActionDatabaseRegistrar;

UCLASS()
class UAnimGraphNode_DistanceMatching : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_DistanceMatching Node;

	//~ Begin UEdGraphNode Interface.
	virtual FLinearColor GetNodeTitleColor() const override;
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	//~ End UEdGraphNode Interface.

	//~ Begin UAnimGraphNode_Base Interface
	virtual FString GetNodeCategory() const override;
	//~ End UAnimGraphNode_Base Interface

	UAnimGraphNode_DistanceMatching(const FObjectInitializer& ObjectInitializer);

private:
	/** Constructing FText strings can be costly, so we cache the node's title */
	FNodeTextCache CachedNodeTitle;
};
