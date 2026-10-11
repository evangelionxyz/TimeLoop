// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "CollapseTrigger.generated.h"

class ACollapseDirector;
class UBoxComponent;

// Box trigger that tells the ACollapseDirector that something happened (EventId).
// Examples:
//   EventId "Quake"           + RequiredItemId <gift/drink>  -> entryway: quake fires once the player holds the gift
//   EventId "SecondaryTremor"                                  -> 4th floor hallway: beam drop / stairwell collapse
//   Any other id                                               -> forwarded to the director's OnCollapseEvent for Blueprints
UCLASS()
class TIMELOOP_API ACollapseTrigger : public AActor
{
	GENERATED_BODY()

public:
	ACollapseTrigger();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Event sent to the director
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FName EventId = TEXT("Quake");

	// If set, the player's InventoryComponent must contain this item id before the trigger fires.
	// While the player stands inside without it, the trigger keeps re-checking so picking it up in the volume works.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	FName RequiredItemId = NAME_None;

	// Fire only once, then disable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
	bool bOneShot = true;

	// Director to notify. If empty, the first ACollapseDirector in the level is used.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Trigger")
	TObjectPtr<ACollapseDirector> Director;

	UFUNCTION(BlueprintCallable, Category = "Trigger")
	void SetTriggerEnabled(bool bEnabled);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Trigger")
	TObjectPtr<UBoxComponent> Box;

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	bool PlayerMeetsRequirements(const AActor* Player) const;
	void TryFire();

	TWeakObjectPtr<AActor> OverlappingPlayer;
	FTimerHandle RecheckHandle;
	bool bFired = false;
};
