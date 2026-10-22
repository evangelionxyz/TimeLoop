// Copyright (c) 2026 Team Ficus


#include "Debris.h"
#include "Components/StaticMeshComponent.h"
#include "FPSCameraEffects.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

ADebris::ADebris()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SetRootComponent(StaticMesh);
	StaticMesh->SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
	StaticMesh->SetNotifyRigidBodyCollision(true); // Enable OnComponentHit when beam hits the floor

	DustParticleComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DustParticleComponent"));
	DustParticleComponent->SetupAttachment(StaticMesh);
	DustParticleComponent->SetAutoActivate(false);
}

void ADebris::BeginPlay()
{
	Super::BeginPlay();

	InitialTransform = GetActorTransform();
	InitialCollisionProfile = StaticMesh->GetCollisionProfileName();

	StaticMesh->OnComponentHit.AddDynamic(this, &ADebris::OnMeshHit);

	if (ACollapseDirector *Director = GetCollapseDirector())
	{
		Director->OnCollapseEvent.AddDynamic(this, &ADebris::OnCollapseEvent);
		Director->OnResetState.AddDynamic(this, &ADebris::OnResetState);
	}

	ApplyInitialState();
}

void ADebris::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(TriggerDelayHandle);

	if (CachedDirector)
	{
		CachedDirector->OnResetState.RemoveDynamic(this, &ADebris::OnResetState);
		CachedDirector->OnCollapseEvent.RemoveDynamic(this, &ADebris::OnCollapseEvent);
	}

	if (SpawnedChaosActor)
	{
		SpawnedChaosActor->Destroy();
		SpawnedChaosActor = nullptr;
	}

	StaticMesh->OnComponentHit.RemoveDynamic(this, &ADebris::OnMeshHit);

	Super::EndPlay(EndPlayReason);
}

void ADebris::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADebris::OnCollapseEvent(FName EventId, AActor* EventInstigator)
{
	if (bHasTriggered || EventId.IsNone() || EventId != RequiredEventId)
	{
		return;
	}

	bHasTriggered = true;

	if (TriggerDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(TriggerDelayHandle, this, &ADebris::ActivateDebris, TriggerDelay, false);
	}
	else
	{
		ActivateDebris();
	}
}

void ADebris::ActivateDebris()
{
	if (TriggerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, TriggerSound, InitialTransform.GetLocation());
	}

	if (TriggerImpactVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, TriggerImpactVFX,
			InitialTransform.GetLocation(), InitialTransform.GetRotation().Rotator());
	}

	if (DustParticleComponent && DustParticleComponent->GetAsset())
	{
		DustParticleComponent->Activate(true);
	}

	if (ChaosFractureActorClass)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnedChaosActor = GetWorld()->SpawnActor<AActor>(ChaosFractureActorClass, InitialTransform, Params);

		// Hide our preview mesh if the Chaos actor replaces it
		StaticMesh->SetVisibility(false, false);
		StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}

	StaticMesh->SetVisibility(true, false);
	StaticMesh->SetCollisionProfileName(InitialCollisionProfile);
	StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	if (bSimulatePhysicsOnTrigger && StaticMesh->GetStaticMesh())
	{
		bIsFalling = true;
		StaticMesh->SetSimulatePhysics(true);
		StaticMesh->WakeAllRigidBodies();

		const FVector RandomSpread = FMath::VRand() * RandomImpulseStrength;
		StaticMesh->AddImpulse(InitialImpulse + RandomSpread, NAME_None, true);

		if (RandomAngularImpulse > 0.0f)
		{
			const FVector RandomTorque = FMath::VRand() * RandomAngularImpulse;
			StaticMesh->AddAngularImpulseInDegrees(RandomTorque, NAME_None, true);
		}
	}
}

void ADebris::OnMeshHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!bIsFalling || bHasPlayedFloorHit || !bSimulatePhysicsOnTrigger)
	{
		return;
	}

	if (OtherActor == this || NormalImpulse.IsNearlyZero())
	{
		return;
	}

	bHasPlayedFloorHit = true;
	if (FloorHitSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FloorHitSound, Hit.ImpactPoint);
	}

	if (FloorHitVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FloorHitVFX, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
	}

	if (bShakeCameraOnHit)
	{
		if (APawn *PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			if (UFPSCameraEffects *CameraEffects = PlayerPawn->FindComponentByClass<UFPSCameraEffects>())
			{
				CameraEffects->PlayEarthQuakeShake(HitShakeDuration, HitShakeIntensity);
			}
		}
	}
}

void ADebris::OnResetState(ECollapseEndResult EndResult)
{
	GetWorldTimerManager().ClearTimer(TriggerDelayHandle);
	ApplyInitialState();
}

void ADebris::ApplyInitialState()
{
	bHasTriggered = false;
	bIsFalling = false;
	bHasPlayedFloorHit = false;

	if (SpawnedChaosActor)
	{
		SpawnedChaosActor->Destroy();
		SpawnedChaosActor = nullptr;
	}

	if (DustParticleComponent)
	{
		DustParticleComponent->DeactivateImmediate();
	}

	// Stop physics and zero out velocities BEFORE teleporting back to InitialTransform
	if (StaticMesh)
	{
		if (StaticMesh->IsSimulatingPhysics())
		{
			StaticMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
			StaticMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
			StaticMesh->SetSimulatePhysics(false);
		}

		SetActorTransform(InitialTransform, false, nullptr, ETeleportType::TeleportPhysics);

		if (bStartHidden)
		{
			StaticMesh->SetVisibility(false, false);
			StaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			StaticMesh->SetVisibility(true, false);
			StaticMesh->SetCollisionProfileName(InitialCollisionProfile);
			StaticMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		}
	}
}

ACollapseDirector* ADebris::GetCollapseDirector()
{
	if (!CachedDirector)
	{
		CachedDirector = Cast<ACollapseDirector>(UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass()));
	}
	return CachedDirector;
}
