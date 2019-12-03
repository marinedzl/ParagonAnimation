#include "AnimNode_DistanceMatching.h"
#include "Animation/AnimInstanceProxy.h"

#pragma optimize("", off)

namespace
{
	float FindPositionFromDistanceCurve(const FFloatCurve& DistanceCurve, const float Distance, UAnimSequenceBase* InAnimSequence)
	{
		const TArray<FRichCurveKey>& Keys = DistanceCurve.FloatCurve.GetConstRefOfKeys();

		const int32 NumKeys = Keys.Num();
		if (NumKeys < 2)
		{
			return 0.f;
		}

		// Some assumptions: 
		// - keys have unique values, so for a given value, it maps to a single position on the timeline of the animation.
		// - key values are sorted in increasing order.

#if ENABLE_ANIM_DEBUG
		// verify assumptions in DEBUG
		bool bIsSortedInIncreasingOrder = true;
		bool bHasUniqueValues = true;
		TMap<float, float> UniquenessMap;
		UniquenessMap.Add(Keys[0].Value, Keys[0].Time);
		for (int32 KeyIndex = 1; KeyIndex < Keys.Num(); KeyIndex++)
		{
			if (UniquenessMap.Find(Keys[KeyIndex].Value) != nullptr)
			{
				bHasUniqueValues = false;
			}

			UniquenessMap.Add(Keys[KeyIndex].Value, Keys[KeyIndex].Time);

			if (Keys[KeyIndex].Value < Keys[KeyIndex - 1].Value)
			{
				bIsSortedInIncreasingOrder = false;
			}
		}

		if (!bIsSortedInIncreasingOrder || !bHasUniqueValues)
		{
			UE_LOG(LogAnimation, Warning, TEXT("ERROR: BAD DISTANCE CURVE: %s, bIsSortedInIncreasingOrder: %d, bHasUniqueValues: %d"),
				*GetNameSafe(InAnimSequence), bIsSortedInIncreasingOrder, bHasUniqueValues);
		}
#endif

		int32 first = 1;
		int32 last = NumKeys - 1;
		int32 count = last - first;

		while (count > 0)
		{
			int32 step = count / 2;
			int32 middle = first + step;

			if (Distance > Keys[middle].Value)
			{
				first = middle + 1;
				count -= step + 1;
			}
			else
			{
				count = step;
			}
		}

		const FRichCurveKey& KeyA = Keys[first - 1];
		const FRichCurveKey& KeyB = Keys[first];
		const float Diff = KeyB.Value - KeyA.Value;
		const float Alpha = !FMath::IsNearlyZero(Diff) ? ((Distance - KeyA.Value) / Diff) : 0.f;
		return FMath::Lerp(KeyA.Time, KeyB.Time, Alpha);
	}

	float GetDistanceCurveTime(UAnimSequenceBase* Sequence, const FName& CurveName, float Distance)
	{
		auto& Curves = Sequence->GetCurveData().FloatCurves;

		for (int i = 0; i < Curves.Num(); i++)
		{
			if (Curves[i].Name.DisplayName == CurveName)
			{
				return FindPositionFromDistanceCurve(Curves[i], Distance, Sequence);
			}
		}

		return 0;
	}
}

FAnimNode_DistanceMatching::FAnimNode_DistanceMatching()
	: Sequence(nullptr)
	, Distance(0.0f)
	, EnableMatching(true)
{
}

float FAnimNode_DistanceMatching::GetCurrentAssetTime()
{
	return 0;
}

float FAnimNode_DistanceMatching::GetCurrentAssetLength()
{
	return Sequence ? Sequence->SequenceLength : 0.0f;
}

void FAnimNode_DistanceMatching::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	FAnimNode_AssetPlayerBase::Initialize_AnyThread(Context);
	InternalTimeAccumulator = 0;
}

void FAnimNode_DistanceMatching::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
}

void FAnimNode_DistanceMatching::UpdateAssetPlayer(const FAnimationUpdateContext& Context)
{
	GetEvaluateGraphExposedInputs().Execute(Context);

	if (Sequence && EnableMatching)
	{
		float Time = InternalTimeAccumulator;
		float MoveDelta = Context.GetDeltaTime();

		float Target = GetDistanceCurveTime(Sequence, CurveName, Distance);
		if (Target > Time)
			Time = Target;
		else
			Time += MoveDelta;

		Time = FMath::Min(Time, Sequence->SequenceLength);

		InternalTimeAccumulator = Time;
	}
}

void FAnimNode_DistanceMatching::Evaluate_AnyThread(FPoseContext& Output)
{
	check(Output.AnimInstanceProxy != nullptr);
	if ((Sequence != nullptr) && (Output.AnimInstanceProxy->IsSkeletonCompatible(Sequence->GetSkeleton())))
	{
		Sequence->GetAnimationPose(Output.Pose, Output.Curve, FAnimExtractContext(InternalTimeAccumulator, Output.AnimInstanceProxy->ShouldExtractRootMotion()));
	}
	else
	{
		Output.ResetToRefPose();
	}
}

void FAnimNode_DistanceMatching::OverrideAsset(UAnimationAsset* NewAsset)
{
	if (UAnimSequenceBase* NewSequence = Cast<UAnimSequenceBase>(NewAsset))
	{
		Sequence = NewSequence;
	}
}

void FAnimNode_DistanceMatching::GatherDebugData(FNodeDebugData& DebugData)
{
	FString DebugLine = DebugData.GetNodeName(this);

	DebugLine += FString::Printf(TEXT("('%s' Distance: %.3f, Time: %.3f)"), *GetNameSafe(Sequence), Distance, InternalTimeAccumulator);
	DebugData.AddDebugItem(DebugLine, true);
}
#pragma optimize("", on)
