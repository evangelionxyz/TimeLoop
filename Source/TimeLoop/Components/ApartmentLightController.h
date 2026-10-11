// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ApartmentLightController.generated.h"

class ULightComponent;
class UMaterialInstanceDynamic;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class EApartmentLightMode : uint8
{
	// Steady, original intensity
	Normal UMETA(DisplayName = "Normal"),
	// Mild electrical hum: tiny brightness wobble with rare short dips (pre-quake foreshadowing)
	Buzz UMETA(DisplayName = "Buzz"),
	// Violent rapid flicker (during the quake)
	Flicker UMETA(DisplayName = "Flicker"),
	// Dying lamp: mostly dark, short unreliable bursts of weak light, sparks on re-ignition
	Stutter UMETA(DisplayName = "Stutter"),
	// Completely dark
	Off UMETA(DisplayName = "Off")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnApartmentLightSpark, AActor*, LightActor, FVector, Location);

// Attach to any actor that owns light components (e.g. BP_ArpartmentLight). It caches the owner's
// light intensities and then animates them according to the current mode. ACollapseDirector adds
// this component automatically at runtime, so existing light Blueprints don't need to change.
UCLASS(ClassGroup = (TimeLoop), meta = (BlueprintSpawnableComponent))
class TIMELOOP_API UApartmentLightController : public UActorComponent
{
	GENERATED_BODY()

public:
	UApartmentLightController();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Apartment Light")
	void SetMode(EApartmentLightMode NewMode);

	UFUNCTION(BlueprintPure, Category = "Apartment Light")
	EApartmentLightMode GetMode() const { return Mode; }

	// Electrical pop: kills the light instantly with a spark burst and sound.
	UFUNCTION(BlueprintCallable, Category = "Apartment Light")
	void PopOff();

	// Spawns the spark Niagara system (and spark sound) at the light's position.
	UFUNCTION(BlueprintCallable, Category = "Apartment Light")
	void EmitSparks();

	UPROPERTY(BlueprintAssignable, Category = "Apartment Light")
	FOnApartmentLightSpark OnSpark;

	// --- Setup (set by ACollapseDirector, or per-instance if you add the component by hand) ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light")
	EApartmentLightMode InitialMode = EApartmentLightMode::Normal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Effects")
	TObjectPtr<UNiagaraSystem> SparkSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Effects")
	TObjectPtr<USoundBase> SparkSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Effects")
	TObjectPtr<USoundBase> PopSound = nullptr;

	// Offset from the light component to spawn sparks at (world space Z is usually enough)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Effects")
	FVector SparkOffset = FVector(0.0f, 0.0f, -20.0f);

	// Optional scalar material parameter on the owner's meshes driven with the light (bulb / shade emissive)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Effects")
	FName EmissiveParameterName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Effects")
	float EmissiveOnValue = 10.0f;

	// --- Stutter tuning ---

	// Burst duration range (seconds) of the lamp being lit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Stutter")
	FVector2D StutterOnTime = FVector2D(0.03f, 0.22f);

	// Gap range (seconds) of darkness between bursts
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Stutter")
	FVector2D StutterOffTime = FVector2D(0.15f, 2.2f);

	// Brightness range of a burst, as a fraction of the original intensity (a dying lamp never reaches full power)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Stutter")
	FVector2D StutterBurstBrightness = FVector2D(0.15f, 0.6f);

	// Chance a burst is immediately followed by a second quick blink
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Stutter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StutterDoubleBlinkChance = 0.35f;

	// Chance a burst emits sparks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Apartment Light|Stutter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StutterSparkChance = 0.5f;

protected:
	virtual void BeginPlay() override;

private:
	struct FCachedLight
	{
		TWeakObjectPtr<ULightComponent> Light;
		float BaseIntensity = 0.0f;
	};

	void CacheLights();
	void SetLightScale(float Scale);
	void ScheduleNextStutterPhase(bool bNowOn);

	TArray<FCachedLight> Lights;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> EmissiveMaterials;

	EApartmentLightMode Mode = EApartmentLightMode::Normal;
	bool bCached = false;

	// Generic state machine timers
	float PhaseTimeRemaining = 0.0f;
	bool bPhaseOn = true;
	bool bPendingDoubleBlink = false;
	float CurrentScale = 1.0f;
};
