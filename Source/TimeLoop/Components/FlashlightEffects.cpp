// Copyright (c) 2026 Team Ficus

#include "FlashlightEffects.h"
#include "InventoryComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UFlashlightEffects::UFlashlightEffects()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFlashlightEffects::BeginPlay()
{
	Super::BeginPlay();

	SpotLight = FindSpotLight();
	if (SpotLight)
	{
		BaseIntensity = SpotLight->Intensity;
		bIsOn = SpotLight->IsVisible();
	}
}

USpotLightComponent* UFlashlightEffects::FindSpotLight() const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return nullptr;
	}

	TArray<USpotLightComponent*> Spots;
	Owner->GetComponents<USpotLightComponent>(Spots);
	for (USpotLightComponent* Spot : Spots)
	{
		if (SpotLightComponentName.IsNone() || Spot->GetFName() == SpotLightComponentName)
		{
			return Spot;
		}
	}
	return nullptr;
}

bool UFlashlightEffects::HasFlashlight() const
{
	if (!bRequireFlashlightItem)
	{
		return true;
	}

	const AActor* Owner = GetOwner();
	const UInventoryComponent* Inventory = Owner ? Owner->FindComponentByClass<UInventoryComponent>() : nullptr;
	if (!Inventory)
	{
		return false;
	}

	if (!RequiredItemId.IsNone())
	{
		return Inventory->HasItem(RequiredItemId);
	}

	for (const FInventoryItem& Item : Inventory->GetItems())
	{
		if (Item.ItemType == EInventoryItemType::Flashlight)
		{
			return true;
		}
	}
	return false;
}

bool UFlashlightEffects::SwitchLight()
{
	return SetFlashlightEnabled(!bIsOn);
}

bool UFlashlightEffects::SetFlashlightEnabled(bool bEnabled)
{
	if (bEnabled && !HasFlashlight())
	{
		return false;
	}

	if (bEnabled == bIsOn)
	{
		return true;
	}

	ApplyState(bEnabled, true);
	return true;
}

void UFlashlightEffects::ApplyState(bool bOn, bool bPlaySound)
{
	if (!SpotLight)
	{
		SpotLight = FindSpotLight();
		if (SpotLight)
		{
			BaseIntensity = SpotLight->Intensity;
		}
	}
	if (!SpotLight)
	{
		return;
	}

	bIsOn = bOn;
	SpotLight->SetIntensity(BaseIntensity);
	SpotLight->SetVisibility(bOn);

	if (!bOn)
	{
		FlickerTimeRemaining = 0.0f;
		SetComponentTickEnabled(false);
	}

	if (bPlaySound)
	{
		if (USoundBase* Sound = bOn ? SoundSwitchOn.Get() : SoundSwitchOff.Get())
		{
			UGameplayStatics::PlaySound2D(this, Sound);
		}
	}

	OnFlashlightToggled.Broadcast(bOn);
}

void UFlashlightEffects::FlickerFlashlight(float Duration)
{
	if (!bIsOn || !SpotLight || Duration <= 0.0f)
	{
		return;
	}

	FlickerTimeRemaining = Duration;
	SetComponentTickEnabled(true);
}

void UFlashlightEffects::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!SpotLight)
	{
		SetComponentTickEnabled(false);
		return;
	}

	FlickerTimeRemaining -= DeltaTime;
	if (FlickerTimeRemaining <= 0.0f)
	{
		SpotLight->SetIntensity(BaseIntensity);
		SetComponentTickEnabled(false);
		return;
	}

	const bool bDrop = FMath::FRand() < FlickerDropChance;
	SpotLight->SetIntensity(bDrop ? BaseIntensity * FMath::FRandRange(0.0f, 0.15f) : BaseIntensity * FMath::FRandRange(0.6f, 1.0f));
}
