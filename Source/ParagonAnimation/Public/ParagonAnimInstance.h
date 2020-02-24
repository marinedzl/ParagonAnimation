#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ParagonAnimInstance.generated.h"

UENUM(BlueprintType)
enum class ECardinalDirection : uint8
{
	North UMETA(DisplayName = "North"),
	East UMETA(DisplayName = "East"),
	South UMETA(DisplayName = "South"),
	West UMETA(DisplayName = "West"),
};

UCLASS()
class PARAGONANIMATION_API UParagonAnimInstance : public UAnimInstance
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float YawDelta;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float InverseYawDelta;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float CardinalDirectionAngle;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	ECardinalDirection CardinalDirection;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	FVector DistanceMachingLocation;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float MatchingDistance;
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTimeX) override;
private:
	void UpdateAim(float DeltaTimeX);
	void UpdateActorLean(float DeltaTimeX);
	void UpdateCardinalDirection(float DeltaTimeX);
private:
	void UpdateDistanceMatching(float DeltaTimeX);
	void EvalDistanceMatching(float DeltaTimeX);
private:
	FRotator RotationLastTick;
	FVector AccelerationLastTick;
};
