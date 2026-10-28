// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MayaState.h"
#include "CollapseDirector.h"
#include "Navigation/PathFollowingComponent.h"
#include "MayaCharacter.generated.h"

class ACollapseDirector;
class ATimeLoopGameState;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMayaStateChangedNative, EMayaState, NewState, EMayaState, OldState);

UCLASS()
class TIMELOOP_API AMayaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMayaCharacter();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;

	// --- State System ---

	UFUNCTION(BlueprintCallable, Category = "Maya|State")
	void SetState(EMayaState NewState);

	UFUNCTION(BlueprintPure, Category = "Maya|State")
	EMayaState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Maya|State")
	void RunStateLogic();

	UPROPERTY(BlueprintAssignable, Category = "Maya|Events")
	FOnMayaStateChangedNative OnMayaStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Maya|Interaction")
	void HandleInteraction();

	// --- Navigation ---
	UFUNCTION(BlueprintCallable, Category = "Maya|Movement")
	void MoveToTarget(AActor* TargetActor);

	UFUNCTION(BlueprintCallable, Category = "Maya|Movement")
	void StopMovement();

	// --- Drawing Items ---
	UFUNCTION(BlueprintCallable, Category = "Maya|Drawing")
	void AttachDrawingActors();

	UFUNCTION(BlueprintCallable, Category = "Maya|Drawing")
	void DetachDrawingActors();

	// --- Navigation Target Points ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Targets")
	TObjectPtr<AActor> Ethan_TargetPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Targets")
	TObjectPtr<AActor> Drawing_TargetPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Targets")
	TObjectPtr<AActor> SitHide_TargetPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Targets")
	TObjectPtr<AActor> Shelter_TargetPoint;

	// --- Drawing Setup ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Drawing")
	TObjectPtr<AActor> Crayon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Drawing")
	TObjectPtr<AActor> DrawingPaper;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maya|Drawing")
	bool bIsDrawing = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Drawing")
	FName CrayonSocketName = TEXT("Hand_R_Socket");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Drawing")
	FName PaperSocketName = TEXT("Hand_L_Socket");

	// --- Acceptance Radius ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Movement")
	float MoveAcceptanceRadius = 50.0f;

	// Dialogue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Maya|Dialogue")
	FName ApproachEthanId = TEXT("ApproachEthan");

	// Sound
	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent> MayaCryingAudioComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio|Maya")
	TObjectPtr<USoundBase> MayaCryingSound;

	// "ETHAN?! THE LIGHTS WENT OUT!" through the intercom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio|Maya")
	TObjectPtr<USoundBase> MayaScreamSound;

	void PlayScreamSound();

protected:
	void HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);

	UFUNCTION()
	void HandleCollapseEvent(FName EventId, AActor* EventInstigator);

	UFUNCTION()
	void HandleDialogueFinished();

	UFUNCTION()
	void HandleCollapseResetState(const ECollapseEndResult EndResult);

	UFUNCTION()
	void HandleCollapsePhaseChanged(ECollapsePhase NewPhase);

	ACollapseDirector* GetCollapseDirector();
	ATimeLoopGameState* GetTimeLoopGameState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Maya|State")
	EMayaState CurrentState = EMayaState::Inactive;

	UPROPERTY(Transient)
	TObjectPtr<ACollapseDirector> CachedDirector;

	UPROPERTY(Transient)
	TObjectPtr<ATimeLoopGameState> CachedGameState;

	FDelegateHandle MoveCompleteHandle;

	FTransform CrayonInitialTransform;
	FTransform DrawingPaperInitialTransform;
};
