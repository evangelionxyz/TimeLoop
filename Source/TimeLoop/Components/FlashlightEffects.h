// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FlashlightEffects.generated.h"

class USpotLightComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TIMELOOP_API UFlashlightEffects : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UFlashlightEffects();

protected:
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Spot Light")
	USoundBase *SoundSwitchOn;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Spot Light")
	USoundBase *SoundSwitchOff;

	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SwitchLight();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
