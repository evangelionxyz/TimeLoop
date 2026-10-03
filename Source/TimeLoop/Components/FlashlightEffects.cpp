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
}

void UFlashlightEffects::SwitchLight()
{
}

// Called every frame
void UFlashlightEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

