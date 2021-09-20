#include "ParagonAnimInstance.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma optimize( "", off )
namespace
{
	// Copy from CharacterMovementComponent
	bool PredictStopLocation(
		FVector& OutStopLocation,
		const FVector& CurrentLocation,
		const FVector& Velocity,
		const FVector& Acceleration,
		float Friction,
		float BrakingDeceleration,
		const float TimeStep,
		const int MaxSimulationIterations /*= 10*/)
	{
		const float MIN_TICK_TIME = 1e-6;
		if (TimeStep < MIN_TICK_TIME)
		{
			return false;
		}
		// Apply braking or deceleration
		const bool bZeroAcceleration = Acceleration.IsZero();

		if ((Acceleration | Velocity) > 0.0f)
		{
			return false;
		}

		BrakingDeceleration = FMath::Max(BrakingDeceleration, 0.f);
		Friction = FMath::Max(Friction, 0.f);
		const bool bZeroFriction = (Friction == 0.f);
		const bool bZeroBraking = (BrakingDeceleration == 0.f);

		if (bZeroAcceleration && bZeroFriction)
		{
			return false;
		}

		FVector LastVelocity = bZeroAcceleration ? Velocity : Velocity.ProjectOnToNormal(Acceleration.GetSafeNormal());
		LastVelocity.Z = 0;

		FVector LastLocation = CurrentLocation;

		int Iterations = 0;
		while (Iterations < MaxSimulationIterations)
		{
			Iterations++;

			const FVector OldVel = LastVelocity;

			// Only apply braking if there is no acceleration, or we are over our max speed and need to slow down to it.
			if (bZeroAcceleration)
			{
				// subdivide braking to get reasonably consistent results at lower frame rates
				// (important for packet loss situations w/ networking)
				float RemainingTime = TimeStep;
				const float MaxTimeStep = (1.0f / 33.0f);

				// Decelerate to brake to a stop
				const FVector RevAccel = (bZeroBraking ? FVector::ZeroVector : (-BrakingDeceleration * LastVelocity.GetSafeNormal()));
				while (RemainingTime >= MIN_TICK_TIME)
				{
					// Zero friction uses constant deceleration, so no need for iteration.
					const float dt = ((RemainingTime > MaxTimeStep && !bZeroFriction) ? FMath::Min(MaxTimeStep, RemainingTime * 0.5f) : RemainingTime);
					RemainingTime -= dt;

					// apply friction and braking
					LastVelocity = LastVelocity + ((-Friction) * LastVelocity + RevAccel) * dt;

					// Don't reverse direction
					if ((LastVelocity | OldVel) <= 0.f)
					{
						LastVelocity = FVector::ZeroVector;
						break;
					}
				}

				// Clamp to zero if nearly zero, or if below min threshold and braking.
				const float VSizeSq = LastVelocity.SizeSquared();
				if (VSizeSq <= 1.f || (!bZeroBraking && VSizeSq <= FMath::Square(10)))
				{
					LastVelocity = FVector::ZeroVector;
				}
			}
			else
			{
				FVector TotalAcceleration = Acceleration;
				TotalAcceleration.Z = 0;

				// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
				const FVector AccelDir = TotalAcceleration.GetSafeNormal();
				const float VelSize = LastVelocity.Size();
				TotalAcceleration += -(LastVelocity - AccelDir * VelSize) * Friction;
				// Apply acceleration
				LastVelocity += TotalAcceleration * TimeStep;
			}

			LastLocation += LastVelocity * TimeStep;

			// Clamp to zero if nearly zero, or if below min threshold and braking.
			const float VSizeSq = LastVelocity.SizeSquared();
			if (VSizeSq <= 1.f
				|| (LastVelocity | OldVel) <= 0.f)
			{
				OutStopLocation = LastLocation;
				return true;
			}
		}

		return false;
	}
}

UParagonAnimInstance::UParagonAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	IsAccelerating = false;
	IsMoving = false;
	Lean = 0.f;
	LeanFactor = 0.3f;
	LeanInterpSpeed = 10.f;
	CardinalDirection = EAnimCardinalDirection::North;
	MeshRotationInterpSpeed = 10.f;
	AimYaw = 0.f;
	AimPitch = 0.f;
	DistanceMachingStart = 0.f;
	DistanceMachingStop = 0.f;
	DistanceMachingScaling = 1.f;

	ActorRotation = FRotator::ZeroRotator;
	MeshRotation = FRotator::ZeroRotator;
	DistanceMachingStartLocation = FVector::ZeroVector;
	DistanceMachingStopLocation = FVector::ZeroVector;

	bDrawDebug = false;
}

void UParagonAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (Character)
	{
		ActorRotation = Character->GetActorRotation();
		MeshRotation = Character->GetBaseRotationOffsetRotator() + ActorRotation;
	}
}

void UParagonAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	ACharacter* Character = Cast<ACharacter>(TryGetPawnOwner());
	if (!Character)
		return;

	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
	if (!ensure(CharacterMovement))
		return;

	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!ensure(Mesh))
		return;

	const FVector CurrentActorLoaction = Character->GetActorLocation();
	const FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
	const FVector CurrentVelocity = CharacterMovement->Velocity;
	const FRotator NewActorRotation = Character->GetActorRotation();
	const FRotator BaseMeshRotationOffset = Character->GetBaseRotationOffsetRotator();

	const bool IsAcceleratingNow = !CurrentAcceleration.IsNearlyZero();

	if (IsAcceleratingNow != IsAccelerating)
	{
		if (IsAcceleratingNow)
		{
			DistanceMachingStartLocation = CurrentActorLoaction;
		}
		else
		{
			PredictStopLocation(
				DistanceMachingStopLocation,
				CurrentActorLoaction,
				CurrentVelocity,
				CurrentAcceleration,
				CharacterMovement->BrakingFriction * CharacterMovement->BrakingFrictionFactor,
				CharacterMovement->GetMaxBrakingDeceleration(),
				CharacterMovement->MaxSimulationTimeStep,
				100);
		}

#if ENABLE_DRAW_DEBUG
		if (bDrawDebug)
		{
			if (IsAcceleratingNow)
				DrawDebugSphere(GetWorld(), DistanceMachingStartLocation, 10, 8, FColor::Green, false, 3.f);
			else
				DrawDebugSphere(GetWorld(), DistanceMachingStopLocation, 10, 8, FColor::Red, false, 3.f);
		}
#endif // ENABLE_DRAW_DEBUG
	}

	DistanceMachingStart = FVector::Dist2D(CurrentActorLoaction, DistanceMachingStartLocation) * DistanceMachingScaling;
	DistanceMachingStop = -FVector::Dist2D(CurrentActorLoaction, DistanceMachingStopLocation) * DistanceMachingScaling;

	IsAccelerating = IsAcceleratingNow;
	IsMoving = !CurrentVelocity.IsNearlyZero();

	float YawDelta = FMath::FindDeltaAngleDegrees(ActorRotation.Yaw, NewActorRotation.Yaw);
	Lean = FMath::FInterpTo(Lean, YawDelta / DeltaSeconds * LeanFactor, DeltaSeconds, LeanInterpSpeed);

	ActorRotation = NewActorRotation;

	if (IsAccelerating)
	{
		float CardinalDirectionAngle = 0.f;

		const FRotator InputRotation = CurrentAcceleration.ToOrientationRotator();

		const float InputDelta = FMath::FindDeltaAngleDegrees(InputRotation.Yaw, ActorRotation.Yaw);
		if (InputDelta > 0.f)
		{
			if (InputDelta < 70.f)
			{
				CardinalDirection = EAnimCardinalDirection::North;
				CardinalDirectionAngle = InputDelta;
			}
			else if (InputDelta > 110.f)
			{
				CardinalDirection = EAnimCardinalDirection::South;
				CardinalDirectionAngle = InputDelta + 180;
			}
			else
			{
				CardinalDirection = EAnimCardinalDirection::West;
				CardinalDirectionAngle = InputDelta - 90;
			}
		}
		else
		{
			if (InputDelta > -70.f)
			{
				CardinalDirection = EAnimCardinalDirection::North;
				CardinalDirectionAngle = InputDelta;
			}
			else if (InputDelta < -110.f)
			{
				CardinalDirection = EAnimCardinalDirection::South;
				CardinalDirectionAngle = InputDelta + 180;
			}
			else
			{
				CardinalDirection = EAnimCardinalDirection::East;
				CardinalDirectionAngle = InputDelta + 90;
			}
		}

		CardinalDirectionAngle = -CardinalDirectionAngle;

		const FRotator CardinalDirectionRotation(0.f, CardinalDirectionAngle, 0.f);
		const FRotator TargetMeshRotation = BaseMeshRotationOffset + CardinalDirectionRotation + ActorRotation;
		MeshRotation = FMath::RInterpTo(MeshRotation, TargetMeshRotation, DeltaSeconds, MeshRotationInterpSpeed);
		// 记得关闭移动组件的NetworkSmoothing，否则MeshRotation的修改会被SmoothClientPosition覆盖
		Mesh->SetWorldRotation(MeshRotation);
	}

	const FRotator BaseAimRotation = Character->GetBaseAimRotation();
	FRotator AimDelta = BaseAimRotation - (MeshRotation - BaseMeshRotationOffset);
	AimDelta.Normalize();
	AimYaw = AimDelta.Yaw;
	AimPitch = AimDelta.Pitch;
}
#pragma optimize( "", on )
