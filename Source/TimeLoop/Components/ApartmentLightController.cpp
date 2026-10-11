// Copyright (c) 2026 Team Ficus

#include "ApartmentLightController.h"
#include "Components/LightComponent.h"
#include "Components/MeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

UApartmentLightController::UApartmentLightController()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UApartmentLightController::BeginPlay()
{
	Super::BeginPlay();

	CacheLights();
	SetMode(InitialMode);
}

void UApartmentLightController::CacheLights()
{
	if (bCached)
	{
		return;
	}
	bCached = true;

	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	TArray<ULightComponent*> FoundLights;
	Owner->GetComponents<ULightComponent>(FoundLights);
	for (ULightComponent* Light : FoundLights)
	{
		if (Light->Mobility == EComponentMobility::Static)
		{
			UE_LOG(LogTemp, Warning, TEXT("ApartmentLightController: light '%s' on '%s' is Static and cannot be changed at runtime. Set it to Stationary or Movable."), *Light->GetName(), *Owner->GetName());
		}

		FCachedLight& Entry = Lights.AddDefaulted_GetRef();
		Entry.Light = Light;
		Entry.BaseIntensity = Light->Intensity;
	}

	if (!EmissiveParameterName.IsNone())
	{
		TArray<UMeshComponent*> Meshes;
		Owner->GetComponents<UMeshComponent>(Meshes);
		for (UMeshComponent* Mesh : Meshes)
		{
			for (int32 Slot = 0; Slot < Mesh->GetNumMaterials(); ++Slot)
			{
				if (UMaterialInstanceDynamic* MID = Mesh->CreateAndSetMaterialInstanceDynamic(Slot))
				{
					EmissiveMaterials.Add(MID);
				}
			}
		}
	}
}

void UApartmentLightController::SetLightScale(float Scale)
{
	CurrentScale = Scale;
	const bool bVisible = Scale > KINDA_SMALL_NUMBER;

	for (const FCachedLight& Entry : Lights)
	{
		if (ULightComponent* Light = Entry.Light.Get())
		{
			Light->SetIntensity(Entry.BaseIntensity * Scale);
			Light->SetVisibility(bVisible);
		}
	}

	for (UMaterialInstanceDynamic* MID : EmissiveMaterials)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(EmissiveParameterName, EmissiveOnValue * Scale);
		}
	}
}

void UApartmentLightController::SetMode(EApartmentLightMode NewMode)
{
	CacheLights();

	Mode = NewMode;
	bPendingDoubleBlink = false;

	switch (Mode)
	{
	case EApartmentLightMode::Normal:
		SetLightScale(1.0f);
		SetComponentTickEnabled(false);
		break;

	case EApartmentLightMode::Off:
		SetLightScale(0.0f);
		SetComponentTickEnabled(false);
		break;

	case EApartmentLightMode::Buzz:
		bPhaseOn = true;
		PhaseTimeRemaining = FMath::FRandRange(0.5f, 3.0f);
		SetComponentTickEnabled(true);
		break;

	case EApartmentLightMode::Flicker:
		bPhaseOn = true;
		PhaseTimeRemaining = 0.0f;
		SetComponentTickEnabled(true);
		break;

	case EApartmentLightMode::Stutter:
		// Start dark so the lamp visibly "dies" before its first unreliable burst.
		SetLightScale(0.0f);
		bPhaseOn = false;
		PhaseTimeRemaining = FMath::FRandRange(0.05f, 0.5f);
		SetComponentTickEnabled(true);
		break;
	}
}

void UApartmentLightController::PopOff()
{
	EmitSparks();

	if (PopSound)
	{
		const AActor* Owner = GetOwner();
		UGameplayStatics::PlaySoundAtLocation(this, PopSound, Owner ? Owner->GetActorLocation() : FVector::ZeroVector);
	}

	SetMode(EApartmentLightMode::Off);
}

void UApartmentLightController::EmitSparks()
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector Location = Owner->GetActorLocation();
	for (const FCachedLight& Entry : Lights)
	{
		if (const ULightComponent* Light = Entry.Light.Get())
		{
			Location = Light->GetComponentLocation();
			break;
		}
	}
	Location += SparkOffset;

	if (SparkSystem)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, SparkSystem, Location);
	}

	if (SparkSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SparkSound, Location);
	}

	OnSpark.Broadcast(GetOwner(), Location);
}

void UApartmentLightController::ScheduleNextStutterPhase(bool bNowOn)
{
	bPhaseOn = bNowOn;

	if (bNowOn)
	{
		SetLightScale(FMath::FRandRange(StutterBurstBrightness.X, StutterBurstBrightness.Y));
		PhaseTimeRemaining = FMath::FRandRange(StutterOnTime.X, StutterOnTime.Y);
		bPendingDoubleBlink = FMath::FRand() < StutterDoubleBlinkChance;

		if (FMath::FRand() < StutterSparkChance)
		{
			EmitSparks();
		}
	}
	else
	{
		SetLightScale(0.0f);

		// A double blink is a burst followed by a very short gap and another burst.
		if (bPendingDoubleBlink)
		{
			bPendingDoubleBlink = false;
			PhaseTimeRemaining = FMath::FRandRange(0.04f, 0.1f);
		}
		else
		{
			PhaseTimeRemaining = FMath::FRandRange(StutterOffTime.X, StutterOffTime.Y);
		}
	}
}

void UApartmentLightController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PhaseTimeRemaining -= DeltaTime;

	switch (Mode)
	{
	case EApartmentLightMode::Buzz:
		if (PhaseTimeRemaining <= 0.0f)
		{
			// Flip between "wobbling normally" and a very short dip
			bPhaseOn = !bPhaseOn;
			PhaseTimeRemaining = bPhaseOn ? FMath::FRandRange(1.5f, 5.0f) : FMath::FRandRange(0.04f, 0.1f);
		}
		if (bPhaseOn)
		{
			const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
			SetLightScale(1.0f - 0.08f * (FMath::PerlinNoise1D(Time * 23.0f) * 0.5f + 0.5f));
		}
		else
		{
			SetLightScale(0.25f);
		}
		break;

	case EApartmentLightMode::Flicker:
		if (PhaseTimeRemaining <= 0.0f)
		{
			PhaseTimeRemaining = FMath::FRandRange(0.02f, 0.09f);
			SetLightScale(FMath::FRand() < 0.35f ? 0.0f : FMath::FRandRange(0.3f, 1.2f));
		}
		break;

	case EApartmentLightMode::Stutter:
		if (PhaseTimeRemaining <= 0.0f)
		{
			ScheduleNextStutterPhase(!bPhaseOn);
		}
		break;

	default:
		SetComponentTickEnabled(false);
		break;
	}
}
