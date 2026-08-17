// Copyright (c) 2026 Team Ficus

#include "FlashlightEffects.h"
#include "Components/SpotLightComponent.h"
#include "Kismet/GameplayStatics.h"

UFlashlightEffects::UFlashlightEffects()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFlashlightEffects::BeginPlay()
{
	Super::BeginPlay();

	SpotLight = GetOwner()->FindComponentByClass<USpotLightComponent>();
	if (SpotLight)
	{
		SpotLight->SetVisibility(false);
	}
}

void UFlashlightEffects::SwitchLight()
{
	if (!SpotLight)
		return;

	const bool IsVisible = SpotLight->IsVisible();
	SpotLight->SetVisibility(!IsVisible);

	if (SoundSwitchOn && SoundSwitchOff)
	{
		UGameplayStatics::PlaySound2D(this, IsVisible ? SoundSwitchOff : SoundSwitchOn);
	}
}

// Called every frame
void UFlashlightEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

