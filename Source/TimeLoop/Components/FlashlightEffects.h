// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightEffects.generated.h"

class USpotLightComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlashlightToggled, bool, bIsOn);

// Drives the owner's flashlight SpotLightComponent: toggling, inventory gating, sound and a
// "dying battery" flicker used by the earthquake sequence.
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TIMELOOP_API UFlashlightEffects : public UActorComponent
{
	GENERATED_BODY()

public:
	UFlashlightEffects();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Toggles the flashlight. Returns false if the player does not own a flashlight yet.
	UFUNCTION(BlueprintCallable, Category = "Spot Light")
	bool SwitchLight();

	// Forces the flashlight on/off (still respects the inventory requirement when turning on).
	UFUNCTION(BlueprintCallable, Category = "Spot Light")
	bool SetFlashlightEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Spot Light")
	bool IsFlashlightOn() const { return bIsOn; }

	// True when the owner's InventoryComponent contains an item of type Flashlight (or the item named RequiredItemId).
	UFUNCTION(BlueprintPure, Category = "Spot Light")
	bool HasFlashlight() const;

	// Makes the beam stutter for Duration seconds (dying battery / electrical interference), then restores it.
	UFUNCTION(BlueprintCallable, Category = "Spot Light")
	void FlickerFlashlight(float Duration = 1.5f);

	UPROPERTY(BlueprintAssignable, Category = "Spot Light")
	FOnFlashlightToggled OnFlashlightToggled;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Spot Light")
	TObjectPtr<USoundBase> SoundSwitchOn = nullptr;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Spot Light")
	TObjectPtr<USoundBase> SoundSwitchOff = nullptr;

	// Name of the SpotLightComponent on the owner. Leave empty to use the first one found.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spot Light")
	FName SpotLightComponentName = NAME_None;

	// If true the flashlight only turns on when the owner's inventory holds a flashlight.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spot Light")
	bool bRequireFlashlightItem = true;

	// Optional exact item id. Leave None to accept any inventory item of type Flashlight.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spot Light")
	FName RequiredItemId = NAME_None;

	// Probability per frame that the beam is cut while flickering.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spot Light|Flicker", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FlickerDropChance = 0.45f;

private:
	USpotLightComponent* FindSpotLight() const;
	void ApplyState(bool bOn, bool bPlaySound);

	UPROPERTY(Transient)
	TObjectPtr<USpotLightComponent> SpotLight = nullptr;

	bool bIsOn = false;
	float BaseIntensity = 0.0f;
	float FlickerTimeRemaining = 0.0f;
};
