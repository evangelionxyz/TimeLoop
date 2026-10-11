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

void UFPSCameraEffects::GetShakingParameters(float& OutFrequency, float& OutPitchIntensity, float& OutRollIntensity, float& OutVerticalIntensity) const
{
	OutFrequency = ShakingFrequency;
	OutPitchIntensity = PitchIntensity;
	OutRollIntensity = RollIntensity;
	OutVerticalIntensity = VerticalOffsetIntensity;
}

void UFPSCameraEffects::SetLeaningParameters(const float NewTargetLeanAngle)
{
	TargetLeanAngle = NewTargetLeanAngle;
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
		TargetVignetteIntensity = DefaultVignetteIntensity;
		TargetVignetteSize = DefaultVignetteSize;
	}
}

void UFPSCameraEffects::PlayEarthQuakeShake(float Duration, float Intensity)
{
	bEarthQuakeShaking = true;
	EarthQuakeShakeDuration = Duration;
	EarthQuakeShakeIntensity = Intensity;
}

void UFPSCameraEffects::BeginPlay()
{
	Super::BeginPlay();
	Camera = GetOwner()->FindComponentByClass<UCameraComponent>();

	LeanAngle = 0.0f;

	ShakingPitch = 0.0f;
	ShakingRoll = 0.0f;
	ShakingVerticalOffset = 0.0f;

	CurrentVignetteIntensity = DefaultVignetteIntensity;
	TargetVignetteIntensity = DefaultVignetteIntensity;
	VignetteMaxIntensity = DefaultVignetteIntensity;
	VignetteMinIntensity = DefaultVignetteIntensity;

	CurrentVignetteSize = DefaultVignetteSize;
	TargetVignetteSize = DefaultVignetteSize;

	if (Camera)
	{
		TargetFOV = Camera->FieldOfView;
		Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
		Camera->PostProcessSettings.VignetteIntensity = CurrentVignetteIntensity;
		Camera->PostProcessSettings.bOverride_VignetteSize = true;
		Camera->PostProcessSettings.VignetteSize = FVector2f(CurrentVignetteSize.X, CurrentVignetteSize.Y);
	}
}
void UFPSCameraEffects::PerformShaking(const float DeltaTime)
{
	static float RunningTime = 0.0f;

	if (!bShaking || bEarthQuakeShaking)
	{
		RunningTime = 0.0f;
		return;
	}

	RunningTime += ShakingFrequency * DeltaTime;
	const float ShakingValue = RunningTime * ShakingFrequency;

	ShakingPitch = FMath::Sin(ShakingValue) * PitchIntensity;
	ShakingRoll = (FMath::Cos(ShakingValue) * 0.5f) * RollIntensity;
	ShakingVerticalOffset = (FMath::Sin(ShakingValue) * 2.0f) * VerticalOffsetIntensity;
}


void UFPSCameraEffects::PerformLeaning(const float DeltaTime)
{
	if (!bLeaning)
	{
		LeanAngle = 0.0f;
		return;
	}

	LeanAngle = FMath::FInterpTo(
		LeanAngle,
		TargetLeanAngle * MaxLeanAngle,
		DeltaTime,
		LeanSpeed
	);
}

void UFPSCameraEffects::ApplyVignette(const float DeltaTime)
{
	if (!Camera)
		return;

	static float RunningTime = 0.0f;

	if (!bVignette)
	{
		RunningTime = 0.0f;
		CurrentVignetteIntensity = FMath::FInterpTo(CurrentVignetteIntensity, 0.0f, DeltaTime, VignetteInterpSpeed);
		Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
		Camera->PostProcessSettings.VignetteIntensity = CurrentVignetteIntensity;

		return;
	}

	// Animating the Recovering Heartbeat Pulse
	if (bIsRecovering || (VignetteMaxIntensity > VignetteMinIntensity + UE_KINDA_SMALL_NUMBER))
	{
		RunningTime += DeltaTime * VignettePulseFrequency;
		const float SineVal = (FMath::Sin(RunningTime) + 1.0f) * 0.5f; // [0.0, 1.0]

		// Animate Vignette Intensity
		const float PulseTargetIntensity = FMath::Lerp(VignetteMinIntensity, VignetteMaxIntensity, SineVal);
		CurrentVignetteIntensity = FMath::FInterpTo(CurrentVignetteIntensity, PulseTargetIntensity, DeltaTime, VignetteInterpSpeed);

		// Animate Vignette Size to give the recovery breathing / heartbeat feeling
		const FVector2D PulseTargetSize = FMath::Lerp(VignettePulseMinSize, VignettePulseMaxSize, SineVal);
		CurrentVignetteSize.X = FMath::FInterpTo(CurrentVignetteSize.X, PulseTargetSize.X, DeltaTime, VignetteInterpSpeed);
		CurrentVignetteSize.Y = FMath::FInterpTo(CurrentVignetteSize.Y, PulseTargetSize.Y, DeltaTime, VignetteInterpSpeed);
	}
	else
	{
		RunningTime = 0.0f;
		CurrentVignetteIntensity = FMath::FInterpTo(CurrentVignetteIntensity, TargetVignetteIntensity, DeltaTime, VignetteInterpSpeed);
		CurrentVignetteSize.X = FMath::FInterpTo(CurrentVignetteSize.X, TargetVignetteSize.X, DeltaTime, VignetteInterpSpeed);
		CurrentVignetteSize.Y = FMath::FInterpTo(CurrentVignetteSize.Y, TargetVignetteSize.Y, DeltaTime, VignetteInterpSpeed);
	}

	Camera->PostProcessSettings.bOverride_VignetteIntensity = true;
	Camera->PostProcessSettings.VignetteIntensity = CurrentVignetteIntensity;
	Camera->PostProcessSettings.bOverride_VignetteSize = true;
	Camera->PostProcessSettings.VignetteSize = FVector2f(CurrentVignetteSize.X, CurrentVignetteSize.Y);
}

void UFPSCameraEffects::PerformEarthQuakeShaking(float DeltaTime)
{
	static float RunningTime = 0.0f;

	if (!bEarthQuakeShaking)
	{
		RunningTime = 0.0f;
		return;
	}

	RunningTime += ShakingFrequency * DeltaTime;
	const float ShakingValue = RunningTime * ShakingFrequency;

	EarthQuakeShakeDuration -= DeltaTime;
	if (EarthQuakeShakeDuration <= 0.0f)
	{
		bEarthQuakeShaking = false;
		EarthQuakeShakeDuration = 0.0f;
	}

	ShakingPitch = FMath::Sin(ShakingValue) * PitchIntensity * EarthQuakeShakeIntensity;
	ShakingRoll = (FMath::Cos(ShakingValue) * 0.5f) * RollIntensity * EarthQuakeShakeIntensity;
	ShakingVerticalOffset = (FMath::Sin(ShakingValue) * 2.0f) * VerticalOffsetIntensity * EarthQuakeShakeIntensity;
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
	PerformEarthQuakeShaking(DeltaTime);

	Camera->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, ShakingVerticalOffset),
		FRotator(ShakingPitch, 0.0f, ShakingRoll + LeanAngle) // Added Lean angle to the Shaking Roll
	);
	
	Camera->FieldOfView = FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaTime, FOVChangeSpeed);
}
