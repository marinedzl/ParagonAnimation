#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ParagonAnimInstance.generated.h"

UCLASS()
class PARAGONANIMATION_API UParagonAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float YawDelta;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float InverseYawDelta;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsLanding;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	FVector DistanceMachingLocation;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float MatchingDistance;
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;
private:
	void UpdateActorLean(float DeltaTimeX);
private:
	void UpdateDistanceMatching(float DeltaTimeX);
	void EvalDistanceMatching(float DeltaTimeX);
private:
	FRotator RotationLastTick;
};
