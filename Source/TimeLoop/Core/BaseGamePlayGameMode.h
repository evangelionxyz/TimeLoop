// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TimeLoopGameState.h"
#include "CollapseDirector.h"
#include "BaseGamePlayGameMode.generated.h"

class ACollapseDirector;

UENUM(BlueprintType)
enum class ELoopState : uint8
{
	Inactive    UMETA(DisplayName = "Inactive"),
	Active      UMETA(DisplayName = "Active"),
	Failed      UMETA(DisplayName = "Failed"),
	Succeeded   UMETA(DisplayName = "Succeeded")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoopFailed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoopSucceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateTimer, float, RemainingTime);

/**
 * Base GameMode for TimeLoop gameplay levels.
 * Manages loop countdown timer, victory/failure states, actor resets, and director notifications.
 */
UCLASS()
class TIMELOOP_API ABaseGamePlayGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ABaseGamePlayGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	// --- Loop Properties ---

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|State")
	bool bPaused = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|Loop")
	float MaxLoopDuration = 30.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|Loop")
	float TimeRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|Loop")
	float FailureResetDelay = 0.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|Loop")
	ELoopState CurrentLoopState = ELoopState::Inactive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|Loop")
	ECollapseEndResult EndResult = ECollapseEndResult::Timeout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|Level")
	FName LevelName = TEXT("Please fill the Level Name !!!!!!");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|Level")
	FText LevelUIDisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|References")
	TObjectPtr<ATimeLoopGameState> TLGameState = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|References")
	TObjectPtr<AHUD> GameHUD = nullptr;

	// --- Event Dispatchers ---

	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLoopFailed OnLoopFailed;

	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLoopSucceeded OnLoopSucceeded;

	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnUpdateTimer OnUpdateTimer;

	// --- Flow Control Functions ---

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void StartPlayGame();

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void PreStartGame();

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void StartLoop();

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void UpdateTimer(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void TriggerLoopFailure(ECollapseEndResult InEndResult = ECollapseEndResult::Timeout);

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void TriggerLoopSuccess();

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void ResetLoop();

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void StartNewGame();

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	virtual void ReturnToMenu();

	UFUNCTION(BlueprintPure, Category = "TimeLoop|Flow")
	ELoopState GetCurrentLoopState() const { return CurrentLoopState; }

	UFUNCTION(BlueprintPure, Category = "TimeLoop|Flow")
	ECollapseEndResult GetEndResult() const { return EndResult; }

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Flow")
	void SetCurrentLoopState(ELoopState NewState);

protected:
	FTimerHandle LoopTimerHandle;

	UFUNCTION(BlueprintImplementableEvent, Category = "TimeLoop|Events")
	void K2_OnLoopReset();
};
