// Copyright (c) 2026 Team Ficus

#include "FPSCameraEffects.h"
#include "Camera/CameraComponent.h"

UFPSCameraEffects::UFPSCameraEffects()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFPSCameraEffects::SetShakingParameters(const float NewFrequency, const float NewPitchIntensity, const float NewRollIntensity, const float NewVerticalIntensity)
{
	ShakingFrequency = NewFrequency;
	PitchIntensity = NewPitchIntensity;
	RollIntensity = NewRollIntensity;
	VerticalOffsetIntensity = NewVerticalIntensity;
}

void UFPSCameraEffects::SetLeaningParameters(const float NewTargetLeanAngle)
{
	M_TargetLeanAngle = NewTargetLeanAngle;
}

void UFPSCameraEffects::SetFOV(float NewFov)
{
	TargetFOV = NewFov;
}

void UFPSCameraEffects::SetVignetteParameters(float MaxIntensity, float MinIntensity)
{
	VignetteMaxIntensity = MaxIntensity;
	VignetteMinIntensity = MinIntensity;
	if (FMath::IsNearlyEqual(MaxIntensity, MinIntensity))
	{
		bIsRecovering = false;
		TargetVignetteIntensity = MaxIntensity;
		DefaultVignetteIntensity = MaxIntensity;
		TargetVignetteSize = DefaultVignetteSize;
		M_VignetteRunningTime = 0.0f;
	}
	else
	{
		bIsRecovering = true;
	}
}

void UFPSCameraEffects::SetVignetteIntensity(float NewIntensity)
{
	TargetVignetteIntensity = NewIntensity;
	VignetteIntensity = NewIntensity;
}

void UFPSCameraEffects::SetDefaultVignetteIntensity(float NewDefaultIntensity)
{
	DefaultVignetteIntensity = NewDefaultIntensity;
	TargetVignetteIntensity = NewDefaultIntensity;
	VignetteIntensity = NewDefaultIntensity;
}

void UFPSCameraEffects::SetVignetteSize(FVector2D NewSize)
{
	TargetVignetteSize = NewSize;
	VignetteSize = NewSize;
}

void UFPSCameraEffects::SetDefaultVignetteSize(FVector2D NewDefaultSize)
{
	DefaultVignetteSize = NewDefaultSize;
	TargetVignetteSize = NewDefaultSize;
	VignetteSize = NewDefaultSize;
}

void UFPSCameraEffects::SetVignetteSizePulseParameters(FVector2D MaxSize, FVector2D MinSize)
{
	VignettePulseMaxSize = MaxSize;
	VignettePulseMinSize = MinSize;
}

void UFPSCameraEffects::SetRecovering(bool bNewRecovering)
{
	bIsRecovering = bNewRecovering;
	if (!bIsRecovering)
	{
		M_VignetteRunningTime = 0.0f;
		TargetVignetteIntensity = DefaultVignetteIntensity;
		VignetteMaxIntensity = DefaultVignetteIntensity;
		VignetteMinIntensity = DefaultVignetteIntensity;
		TargetVignetteSize = DefaultVignetteSize;
	}
	else
	{
		VignetteMinIntensity = 0.2f;
		VignetteMaxIntensity = 1.2f;
	}
}

void UFPSCameraEffects::SetVignettePulse(bool bEnablePulse, float PulseFrequency, float MinIntensity, float MaxIntensity, FVector2D MinSize, FVector2D MaxSize)
{
	bIsRecovering = bEnablePulse;
	VignettePulseFrequency = PulseFrequency;
	VignetteMinIntensity = MinIntensity;
	VignetteMaxIntensity = MaxIntensity;
	VignettePulseMinSize = MinSize;
	VignettePulseMaxSize = MaxSize;
	if (!bEnablePulse)
	{
		M_VignetteRunningTime = 0.0f;
		TargetVignetteIntensity = DefaultVignetteIntensity;
		TargetVignetteSize = DefaultVignetteSize;
	}
}

void UFPSCameraEffects::BeginPlay()
{
	Super::BeginPlay();
	Camera = GetOwner()->FindComponentByClass<UCameraComponent>();

	M_LeanAngle = 0.0f;

	M_ShakingPitch = 0.0f;
	M_ShakingRoll = 0.0f;
	M_ShakingVerticalOffset = 0.0f;
	M_ShakingRunningTime = 0.0f;

	M_VignetteRunningTime = 0.0f;
	M_CurrentVignetteIntensity = DefaultVignetteIntensity;
	TargetVignetteIntensity = DefaultVignetteIntensity;
	VignetteMaxIntensity = DefaultVignetteIntensity;
	VignetteMinIntensity = DefaultVignetteIntensity;

	M_CurrentVignetteSize = DefaultVignetteSize;
	TargetVignetteSize = DefaultVignetteSize;

	if (Camera)
	{
		TargetFOV = Camera->FieldOfView;
		Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
		Camera->PostProcessSettings.VignetteIntensity = M_CurrentVignetteIntensity;
		Camera->PostProcessSettings.bOverride_VignetteSize = true;
		Camera->PostProcessSettings.VignetteSize = FVector2f(M_CurrentVignetteSize.X, M_CurrentVignetteSize.Y);
	}
}

void UFPSCameraEffects::PerformShaking(const float DeltaTime)
{
	if (!bShaking)
	{
		M_ShakingRunningTime = 0.0f;
		return;
	}

	M_ShakingRunningTime += ShakingFrequency * DeltaTime;
	const float ShakingValue = M_ShakingRunningTime * ShakingFrequency;

	M_ShakingPitch = FMath::Sin(ShakingValue) * PitchIntensity;
	M_ShakingRoll = (FMath::Cos(ShakingValue) * 0.5f) * RollIntensity;
	M_ShakingVerticalOffset = (FMath::Sin(ShakingValue) * 2.0f) * VerticalOffsetIntensity;
}

void UFPSCameraEffects::PerformLeaning(const float DeltaTime)
{
	if (!bLeaning)
	{
		M_LeanAngle = 0.0f;
		return;
	}

	M_LeanAngle = FMath::FInterpTo(
		M_LeanAngle,
		M_TargetLeanAngle * MaxLeanAngle,
		DeltaTime,
		LeanSpeed
	);
}

void UFPSCameraEffects::ApplyVignette(float DeltaTime)
{
	if (!Camera)
		return;

	if (!bVignette)
	{
		M_CurrentVignetteIntensity = FMath::FInterpTo(M_CurrentVignetteIntensity, 0.0f, DeltaTime, VignetteInterpSpeed);
		Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
		Camera->PostProcessSettings.VignetteIntensity = M_CurrentVignetteIntensity;
		return;
	}

	if (bIsRecovering || (VignetteMaxIntensity > VignetteMinIntensity + UE_KINDA_SMALL_NUMBER))
	{
		M_VignetteRunningTime += DeltaTime * VignettePulseFrequency;
		const float SineVal = (FMath::Sin(M_VignetteRunningTime) + 1.0f) * 0.5f; // [0.0, 1.0]

		// Animate Vignette Intensity
		const float PulseTargetIntensity = FMath::Lerp(VignetteMinIntensity, VignetteMaxIntensity, SineVal);
		M_CurrentVignetteIntensity = FMath::FInterpTo(M_CurrentVignetteIntensity, PulseTargetIntensity, DeltaTime, VignetteInterpSpeed);

		// Animate Vignette Size to give the recovery breathing / heartbeat feeling
		const FVector2D PulseTargetSize = FMath::Lerp(VignettePulseMinSize, VignettePulseMaxSize, SineVal);
		M_CurrentVignetteSize.X = FMath::FInterpTo(M_CurrentVignetteSize.X, PulseTargetSize.X, DeltaTime, VignetteInterpSpeed);
		M_CurrentVignetteSize.Y = FMath::FInterpTo(M_CurrentVignetteSize.Y, PulseTargetSize.Y, DeltaTime, VignetteInterpSpeed);
	}
	else
	{
		M_VignetteRunningTime = 0.0f;
		M_CurrentVignetteIntensity = FMath::FInterpTo(M_CurrentVignetteIntensity, TargetVignetteIntensity, DeltaTime, VignetteInterpSpeed);
		M_CurrentVignetteSize.X = FMath::FInterpTo(M_CurrentVignetteSize.X, TargetVignetteSize.X, DeltaTime, VignetteInterpSpeed);
		M_CurrentVignetteSize.Y = FMath::FInterpTo(M_CurrentVignetteSize.Y, TargetVignetteSize.Y, DeltaTime, VignetteInterpSpeed);
	}

	Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
	Camera->PostProcessSettings.VignetteIntensity = M_CurrentVignetteIntensity;
	Camera->PostProcessSettings.bOverride_VignetteSize = true;
	Camera->PostProcessSettings.VignetteSize = FVector2f(M_CurrentVignetteSize.X, M_CurrentVignetteSize.Y);
}

// Called every frame
void UFPSCameraEffects::TickComponent(const float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Camera)
		return;

	PerformShaking(DeltaTime);
	PerformLeaning(DeltaTime);
	ApplyVignette(DeltaTime);

	Camera->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, M_ShakingVerticalOffset),
		FRotator(M_ShakingPitch, 0.0f, M_ShakingRoll + M_LeanAngle) // Added Lean angle to the Shaking Roll
	);
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, FOVChangeSpeed);
}
