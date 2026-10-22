// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "CollapseDirector.h"
#include "GameFramework/Actor.h"
#include "Debris.generated.h"

class UStaticMeshComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class USoundBase;
class UGeometryCollectionComponent;
class UFieldSystemComponent;

UCLASS()
class TIMELOOP_API ADebris : public AActor {
	GENERATED_BODY()

public:
	ADebris();

	// Matches ACollapseTrigger::EventId (e.g "StairsFallingBeam", "CeilingCollapse")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Trigger")
	FName RequiredEventId;

	// Stagger multiple debris actors on the same EventId so the don't fall on the same frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Trigger", meta = (ClampMin = "0.0"))
	float TriggerDelay = 0.0f;


	// If true, this debris is invisible and has no collision until the trigger fires
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Behavior")
	bool bStartHidden = false;

	// If true, enables physics on the mesh when triggered (for falling beams/rocks).
	// If false, the mesh stays static where you placed it in the level (for floor ruble piles).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Behavior")
	bool bSimulatePhysicsOnTrigger = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Physics", meta = (EditCondition = "bSimulatePhysicsOnTrigger"))
	FVector InitialImpulse = FVector(0.0f, 0.0f, -200.0f);

	// Random chaos spread added to InitialImpulse
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Physics", meta = (EditCondition = "bSimulatePhysicsOnTrigger"))
	float RandomImpulseStrength = 150.0f;

	// Random rotation spin (degrees / sec) so falling beams twist naturally
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Physics", meta = (EditCondition = "bSimulatePhysicsOnTrigger"))
	float RandomAngularImpulse = 90.0f;

	// Optional Chaos Destruction fracture actor class to spawn at this actor's transform on trigger (destroyed on reset)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Chaos")
	TSubclassOf<AActor> ChaosFractureActorClass;

	// Spawned when the debris triggers (ceiling dust / sparks)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Effects")
	TObjectPtr<UNiagaraSystem> TriggerImpactVFX;

	// Spawned when the impact point when a falling physics beam slams into the floor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Effects")
	TObjectPtr<UNiagaraSystem> FloorHitVFX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Audio")
	TObjectPtr<USoundBase> TriggerSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|Audio")
	TObjectPtr<USoundBase> FloorHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|CameraShake")
	bool bShakeCameraOnHit = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|CameraShake", meta = (EditCondition = "bShakeCameraOnHit", ClampMin = "0.0"))
	float HitShakeIntensity = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debris|CameraShake", meta = (EditCondition = "bShakeCameraOnHit", ClampMin = "0.0"	))
	float HitShakeDuration = 0.6f;



	// Components (Editable in Level Viewport)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debris|Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debris|Components")
	TObjectPtr<UNiagaraComponent> DustParticleComponent;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void Tick(float DeltaTime) override;


private:
	UFUNCTION()
	void OnCollapseEvent(FName EventId, AActor* EventInstigator);

	UFUNCTION()
	void OnResetState(ECollapseEndResult EndResult);

	UFUNCTION()
	void OnMeshHit(UPrimitiveComponent *HitComp, AActor *OtherActor,
		UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit);

	void ActivateDebris();
	void ApplyInitialState();
	ACollapseDirector *GetCollapseDirector();

	UPROPERTY(Transient)
	TObjectPtr<ACollapseDirector> CachedDirector = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<AActor> SpawnedChaosActor = nullptr;

	FTransform InitialTransform;
	FName InitialCollisionProfile;

	FTimerHandle TriggerDelayHandle;
	bool bHasTriggered = false;
	bool bHasPlayedFloorHit = false;
	bool bIsFalling = false;
};
