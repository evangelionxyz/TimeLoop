// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FPSCameraEffects.generated.h"

class UCameraComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TIMELOOP_API UFPSCameraEffects : public UActorComponent {
	GENERATED_BODY()

public:
	UFPSCameraEffects();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Functions ---
	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Shaking")
	void SetShakingParameters(float NewFrequency, float NewPitchIntensity, float NewRollIntensity, float NewVerticalIntensity);

	// Reads the current shaking parameters so a temporary override (e.g. the earthquake) can restore them afterwards.
	UFUNCTION(BlueprintPure, Category = "Camera Effects|Shaking")
	void GetShakingParameters(float& OutFrequency, float& OutPitchIntensity, float& OutRollIntensity, float& OutVerticalIntensity) const;

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Leaning")
	void SetLeaningParameters(float NewTargetLeanAngle);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|FOV")
	void SetFOV(float NewFov);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetVignetteParameters(float MaxIntensity, float MinIntensity);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetVignetteIntensity(float NewIntensity);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetDefaultVignetteIntensity(float NewDefaultIntensity);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetVignetteSize(FVector2D NewSize);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetDefaultVignetteSize(FVector2D NewDefaultSize);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetVignetteSizePulseParameters(FVector2D MaxSize, FVector2D MinSize);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetRecovering(bool bNewRecovering);

	UFUNCTION(BlueprintCallable, Category = "Camera Effects|Vignette")
	void SetVignettePulse(bool bEnablePulse, float PulseFrequency = 6.0f, float MinIntensity = 0.2f, float MaxIntensity = 1.2f, FVector2D MinSize = FVector2D(6.0f, 6.0f), FVector2D MaxSize = FVector2D(14.0f, 14.0f));


	UFUNCTION(BlueprintCallable, Category = "Play")
	void PlayEarthQuakeShake(float Duration, float Intensity);

protected:
	virtual void BeginPlay() override;

	// --- Variables ---
	// Camera Reference
	UPROPERTY(BlueprintReadOnly, Category = "Camera Effects")
	UCameraComponent *Camera;

	// FOV
	UPROPERTY(BlueprintReadOnly, Category = "Camera Effects|FOV")
	float FOVChangeSpeed = 5.0f;
	float TargetFOV = 65.0f;

	// Vignette
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	float DefaultVignetteIntensity = 0.4f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	float VignetteIntensity = 0.4f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	float VignetteMinIntensity = 0.2f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	float VignetteMaxIntensity = 1.2f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	FVector2D DefaultVignetteSize = FVector2D(10.0f, 10.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	FVector2D VignetteSize = FVector2D(10.0f, 10.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	FVector2D VignettePulseMinSize = FVector2D(6.0f, 6.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	FVector2D VignettePulseMaxSize = FVector2D(14.0f, 14.0f);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	float VignettePulseFrequency = 6.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
	float VignetteInterpSpeed = 6.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Camera Effects|Vignette")
	bool bIsRecovering = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Vignette")
    bool bVignette = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Camera Effects|Leaning")
	bool bLeaning = true;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "Camera Effects|Shaking")
	bool bShaking = true;

	// Leaning Properties
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Leaning")
	float MaxLeanAngle = 5.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Leaning")
	float LeanSpeed = 3.0f;

	float TargetLeanAngle = 0.0f;
	float LeanAngle = 0.0f;

	// Shaking Properties
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Shaking")
	float ShakingFrequency = 2.0f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Shaking")
	float PitchIntensity = 0.5f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Shaking")
	float RollIntensity = 0.8f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Camera Effects|Shaking")
	float VerticalOffsetIntensity = 2.3f;

	float ShakingPitch = 0.0f;
	float ShakingRoll = 0.0f;
	float ShakingVerticalOffset = 0.0f;

	float CurrentVignetteIntensity = 0.4f;
	float TargetVignetteIntensity = 0.4f;

	bool bEarthQuakeShaking = false;
	float EarthQuakeShakeDuration = 0.0f;
	float EarthQuakeShakeIntensity = 0.0f;

	FVector2D CurrentVignetteSize = FVector2D(10.0f, 10.0f);
	FVector2D TargetVignetteSize = FVector2D(10.0f, 10.0f);

private:
	void PerformShaking(float DeltaTime);
	void PerformLeaning(float DeltaTime);
	void ApplyVignette(float DeltaTime);
	void PerformEarthQuakeShaking(float DeltaTime);
};
