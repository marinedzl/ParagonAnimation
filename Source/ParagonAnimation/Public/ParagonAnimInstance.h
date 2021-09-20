#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ParagonAnimInstance.generated.h"

UENUM(BlueprintType)
enum class EAnimCardinalDirection : uint8
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
	bool IsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	bool IsMoving;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float Lean;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float LeanFactor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float LeanInterpSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	EAnimCardinalDirection CardinalDirection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float MeshRotationInterpSpeed;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float AimYaw;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float AimPitch;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float DistanceMachingStart;

	UPROPERTY(BlueprintReadOnly, Category = Animation)
	float DistanceMachingStop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	float DistanceMachingScaling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animation)
	bool bDrawDebug;
	
public:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	FRotator ActorRotation;
	FRotator MeshRotation;
	FVector DistanceMachingStartLocation;
	FVector DistanceMachingStopLocation;
};
