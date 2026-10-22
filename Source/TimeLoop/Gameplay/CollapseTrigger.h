// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollapseDirector.h"
#include "CollapseTrigger.generated.h"

class UBoxComponent;

UCLASS()
class TIMELOOP_API ACollapseTrigger : public AActor
{
	GENERATED_BODY()
	
public:	
	ACollapseTrigger();

protected:
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collapse|Trigger")
	TObjectPtr<UBoxComponent> TriggerBox;

	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
		UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
		bool bFromWeep, const FHitResult &SweepResult);

	UFUNCTION()
	void HandleResetState(const ECollapseEndResult EndResult);

public:	

	UFUNCTION(BlueprintCallable, Category = "Collapse|Trigger")
	void ResetTrigger();

	UFUNCTION(BlueprintCallable, Category = "Collapse|Trigger")
	void OnLoopReset() { ResetTrigger(); }

	// EventId sent to ACollapseDirector:HandleTriggerEvent (e.g "Quake", "ApproachMaya", "StairsFallingBeam")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Trigger")
	FName EventId;

	// If true, trigger can only be fired once and deactivates itself
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Trigger")
	bool bTriggerOnce = true;

	// Only trigger of ACollapseDirector is in this phase. (Leave Prologue if unused)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Trigger")
	bool bCheckPhase = false;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Trigger", meta = (EditCondition = "bCheckPhase"))
	ECollapsePhase RequiredPhase = ECollapsePhase::Prologue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Trigger")
	bool bRequiredDialogueFinished = false;

private:
	bool bHasTriggered = false;

	UPROPERTY(Transient)
	TObjectPtr<ACollapseDirector> CachedDirector;

	ACollapseDirector *GetDirector();
	
};
