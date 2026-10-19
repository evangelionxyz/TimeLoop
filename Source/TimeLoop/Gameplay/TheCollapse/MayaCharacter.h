// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MayaState.h"
#include "CollapseDirector.h"
#include "MayaCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMayaStateChanged, EMayaState, NewState, EMayaState, OldState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMayaSheltered);

UCLASS()
class TIMELOOP_API AMayaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMayaCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Maya|State")
	void SetMayaState(EMayaState NewState);

	UFUNCTION(BlueprintPure, Category = "Maya|State")
	EMayaState GetMayaState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Maya|State")
	bool CanTransitionTo(EMayaState NewState) const;

	// Triggered when Ethan throws Barnaby or pushes the steel desk into place
	UFUNCTION(BlueprintCallable, Category = "Maya|State")
	void OrderGoToShelter(AActor *ShelterDestination);

	// State Queries for ABP_Maya
	UFUNCTION(BlueprintPure, Category = "Maya|Animation")
	bool IsDrawing() const { return CurrentState == EMayaState::Drawing; }

	UFUNCTION(BlueprintPure, Category = "Maya|Animation")
	bool IsPanicked() const { return CurrentState == EMayaState::Panicked; }

	UFUNCTION(BlueprintPure, Category = "Maya|Animation")
	bool IsSeekingShelter() const { return CurrentState == EMayaState::SeekingShelter; }

	UFUNCTION(BlueprintPure, Category = "Maya|Animation")
	bool IsSheltered() const { return CurrentState == EMayaState::Sheltered; }

	// Delegates
	UPROPERTY(BlueprintAssignable, Category = "Maya|Events")
	FOnMayaStateChanged OnMayaStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Maya|Events")
	FOnMayaSheltered OnMayaSheltered;

	// World Scene References (Set per instance in editor)
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Maya|References")
	TObjectPtr<AActor> TargetPoint_A;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Maya|References")
	TObjectPtr<AActor> TargetPoint_B;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Maya|References")
	TObjectPtr<AActor> DoorActor;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Maya|References")
	TObjectPtr<AActor> CrayonActor;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Maya|References")
	TObjectPtr<AActor> PlayMatActor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Setup")
	FName HandSocketName = TEXT("Hand_R_Socket");

	// Distance between Maya and target point considered "arrived".
	// Must be greater than (CapsuleRadius ~34-40cm + AcceptanceRadius 40cm = ~74-80cm).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Movement", meta = (ClampMin = "30.0"))
	float ArrivalDistance = 95.0f;

	// Interacts with the door to open it (matches BP_Maya: DisableCollision -> BPI_Interactable::Interact)
	// Can also be overridden in BP_MayaCharacter EventGraph if desired.
	UFUNCTION(BlueprintNativeEvent, Category = "Maya|Interaction")
	void InteractWithDoor(AActor* Door);
	virtual void InteractWithDoor_Implementation(AActor* Door);
protected:
	virtual void OnStateEntered(EMayaState NewState, EMayaState OldState);
	virtual void OnStateExited(EMayaState OldState, EMayaState NewState);

	// Event Handlers
	UFUNCTION()
	void HandleCollapsePhaseChanged(ECollapsePhase NewPhase);

	void MoveToTargetActor(AActor *Target, float AcceptanceRadius = 40.0f);
	void HandleMoveCompleted();

private:
	UPROPERTY(VisibleAnywhere, Category = "Maya|State")
	EMayaState CurrentState = EMayaState::Inactive;

	UPROPERTY(Transient)
	TObjectPtr<AActor> CurrentMoveGoal;

	FTimerHandle StateTimerHandle;
	
};
