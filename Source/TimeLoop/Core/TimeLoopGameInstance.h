// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimeLoopGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoopCountChanged, int32, NewLoopCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTransitionRequested, FName, TargetLevelName);

// UTimeLoopGameInstance
// Persistent GameInstance that stays in memory across all level transitions.
// Manages global level transitions, time loop iteration counts, and persistent state.

UCLASS()
class TIMELOOP_API UTimeLoopGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UTimeLoopGameInstance();

	virtual void Init() override;
	virtual void Shutdown() override;

	// --- Level Navigation ---

	// Level reference for the Main Menu map
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TimeLoop|Maps")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	// Level reference for the primary Gameplay map
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TimeLoop|Maps")
	TSoftObjectPtr<UWorld> GameplayLevel;

	// Opens the Main Menu level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Game Flow")
	bool OpenMainMenu();

	// Opens the Gameplay level and starts a new run
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Game Flow")
	bool OpenGameplayLevel();

	// Opens any level by object reference, broadcasting level transition events
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Game Flow")
	bool OpenLevelByReference(TSoftObjectPtr<UWorld> Level);

	// --- Async Loader ---

	// ASYNC - Opens the Main Menu level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Game Flow")
	bool AsyncOpenMainMenu();

	// ASYNC - Open any level and Start a new Run
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Game Flow")
	bool AsyncOpenGameplayLevel();

	// ASYNC - Opens any level by object reference, broadcasting level transition events
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|Game Flow")
	bool AsyncOpenLevelByReference(TSoftObjectPtr<UWorld> Level);

	// --- Time Loop State ---

	// Current loop / cycle count persistent across levels
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|State")
	int32 CurrentLoopCount = 0;

	// Increments the loop count and fires OnLoopCountChanged
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void IncrementLoopCount();

	// Resets the loop count back to 0
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void ResetLoopCount();

	// Returns the current loop count
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	int32 GetCurrentLoopCount() const { return CurrentLoopCount; }

	// --- Event Delegates ---

	// Broadcast when the loop count changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLoopCountChanged OnLoopCountChanged;

	// Broadcast when a level transition is requested
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLevelTransitionRequested OnLevelTransitionRequested;
};
