#include "ParagonAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#pragma optimize( "", off )
namespace
{
	inline FVector CalcVelocity(const FVector& Velocity, const FVector& Acceleration, float Friction, float TimeStep)
	{
		FVector TotalAcceleration = Acceleration;
		TotalAcceleration.Z = 0;

		// Friction affects our ability to change direction. This is only done for input acceleration, not path following.
		const FVector AccelDir = TotalAcceleration.GetSafeNormal();
		const float VelSize = Velocity.Size();
		TotalAcceleration += -(Velocity - AccelDir * VelSize) * Friction;
		// Apply acceleration
		return Velocity + TotalAcceleration * TimeStep;
	}

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

	bool PredictFallingLocation(
		UCharacterMovementComponent* CharacterMovementComponent,
		FVector& OutStopLocation,
		const FVector& _CurrentLocation,
		const FVector& _Velocity,
		const FVector& Acceleration,
		float FallingLateralFriction,
		float Gravity,
		const float TimeStep,
		const int MaxSimulationIterations /*= 10*/)
	{
		const float MIN_TICK_TIME = 1e-6;
		if (TimeStep < MIN_TICK_TIME)
		{
			return false;
		}

		FVector Velocity = _Velocity;
		FVector LastLocation = _CurrentLocation;

		int Iterations = 0;
		while (Iterations < MaxSimulationIterations)
		{
			Iterations++;

			FVector OldVelocity = Velocity;

			// Apply input
			{
				Velocity.Z = 0.f;
				Velocity = CalcVelocity(Velocity, Acceleration, FallingLateralFriction, TimeStep);
				Velocity.Z = OldVelocity.Z;
			}

			// Apply gravity
			{
				float GravityTime = TimeStep;
				Velocity += FVector(0.f, 0.f, Gravity) * GravityTime;
			}

			LastLocation += Velocity * TimeStep;

			const FVector PawnLocation = LastLocation;
			FFindFloorResult FloorResult;
			CharacterMovementComponent->FindFloor(PawnLocation, FloorResult, false);
			if (FloorResult.IsWalkableFloor())
			{
				FVector TestLocation = FloorResult.HitResult.ImpactPoint;
				FNavLocation NavLocation;
				CharacterMovementComponent->FindNavFloor(TestLocation, NavLocation);
				OutStopLocation = NavLocation;
				OutStopLocation += FVector(0, 0, CharacterMovementComponent->UpdatedComponent->Bounds.BoxExtent.Z);
				return true;
			}
		}

		return false;
	}
}

UParagonAnimInstance::UParagonAnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, YawDelta(0)
	, InverseYawDelta(0)
	, CardinalDirection(ECardinalDirection::North)
	, IsMoving(false)
	, IsAccelerating(false)
	, IsFalling(false)
	, DistanceMachingLocation(FVector::ZeroVector)
	, MatchingDistance(0)
	, RotationLastTick(FRotator::ZeroRotator)
{
}

void UParagonAnimInstance::NativeInitializeAnimation()
{
	//Very Important Line
	Super::NativeInitializeAnimation();
}

void UParagonAnimInstance::NativeBeginPlay()
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
		return;

	FRotator ActorRotation = Pawn->GetActorRotation();
	RotationLastTick = ActorRotation;
	YawDelta = 0;
}

void UParagonAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
	//Very Important Line
	Super::NativeUpdateAnimation(DeltaTimeX);

	UpdateActorLean(DeltaTimeX);
	UpdateCardinalDirection(DeltaTimeX);
	UpdateDistanceMatching(DeltaTimeX);
	EvalDistanceMatching(DeltaTimeX);
}

void UParagonAnimInstance::UpdateDistanceMatching(float DeltaTimeX)
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
		return;

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!ensure(Character))
		return;

	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
	if (!ensure(CharacterMovement))
		return;

	FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
	bool IsAcceleratingNow = FVector::DistSquared(CurrentAcceleration, FVector::ZeroVector) > 0;

	IsFalling = CharacterMovement->IsFalling();

	if (!IsFalling)
	{
		if (IsAcceleratingNow != IsAccelerating)
		{
			IsAccelerating = IsAcceleratingNow;

			if (IsAccelerating)
			{
				DistanceMachingLocation = Pawn->GetActorLocation();
			}
			else
			{
				PredictStopLocation(
					DistanceMachingLocation,
					Pawn->GetActorLocation(),
					CharacterMovement->Velocity,
					CurrentAcceleration,
					CharacterMovement->BrakingFriction,
					CharacterMovement->GetMaxBrakingDeceleration(),
					CharacterMovement->MaxSimulationTimeStep,
					100);
			}
		}
	}
	else
	{
		IsAccelerating = IsAcceleratingNow;
	}
}

void UParagonAnimInstance::EvalDistanceMatching(float DeltaTimeX)
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
		return;
	
	IsMoving = FVector::Dist(Pawn->GetVelocity(), FVector::ZeroVector) > 0;

	FVector Location = Pawn->GetActorLocation();

	if (!IsFalling)
	{
		MatchingDistance = FVector::Dist(Location, DistanceMachingLocation);
		if (!IsAccelerating)
			MatchingDistance = -MatchingDistance;
	}
}

void UParagonAnimInstance::UpdateActorLean(float DeltaTimeX)
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
		return;

	FRotator ActorRotation = Pawn->GetActorRotation();
	float Delta = FMath::FindDeltaAngleDegrees(ActorRotation.Yaw, RotationLastTick.Yaw);
	YawDelta = FMath::FInterpTo(YawDelta, Delta / DeltaTimeX, DeltaTimeX, 6);
	InverseYawDelta = -YawDelta;

	RotationLastTick = ActorRotation;
}

void UParagonAnimInstance::UpdateCardinalDirection(float DeltaTimeX)
{
	APawn* Pawn = TryGetPawnOwner();
	if (!Pawn)
		return;

	ACharacter* Character = Cast<ACharacter>(Pawn);
	if (!ensure(Character))
		return;

	UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
	if (!ensure(CharacterMovement))
		return;

	FVector CurrentAcceleration = CharacterMovement->GetCurrentAcceleration();
	bool IsAcceleratingNow = FVector::DistSquared(CurrentAcceleration, FVector::ZeroVector) > 0;

	if (!IsAcceleratingNow)
		return;

	FRotator InputRotation = CurrentAcceleration.ToOrientationRotator();
	FRotator ActorRotation = Pawn->GetActorRotation();

	float Delta = FMath::FindDeltaAngleDegrees(InputRotation.Yaw, ActorRotation.Yaw);
	if (Delta > 0.f)
	{
		if (Delta < 50.f)
		{
			CardinalDirection = ECardinalDirection::North;
			CardinalDirectionAngle = Delta;
		}
		else if (Delta > 130.f)
		{
			CardinalDirection = ECardinalDirection::South;
			CardinalDirectionAngle = Delta + 180;
		}
		else
		{
			CardinalDirection = ECardinalDirection::West;
			CardinalDirectionAngle = Delta - 90;
		}
	}
	else
	{
		if (Delta > -50.f)
		{
			CardinalDirection = ECardinalDirection::North;
			CardinalDirectionAngle = Delta;
		}
		else if (Delta < -130.f)
		{
			CardinalDirection = ECardinalDirection::South;
			CardinalDirectionAngle = Delta + 180;
		}
		else
		{
			CardinalDirection = ECardinalDirection::East;
			CardinalDirectionAngle = Delta + 90;
		}
	}

	CardinalDirectionAngle = -CardinalDirectionAngle;
}
#pragma optimize( "", on )
