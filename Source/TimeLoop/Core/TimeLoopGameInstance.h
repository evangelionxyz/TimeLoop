// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimeLoopSaveGame.h"
#include "TimeLoopGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoopCountChanged, int32, NewLoopCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelLoopCountChanged, FName, LevelName, int32, NewLoopCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveSolvedChanged, FName, LevelName, bool, bSolved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelTransitionRequested, FName, TargetLevelName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveGameOperationResult, const FString&, SlotName, bool, bSuccess);

// UTimeLoopGameInstance
// Persistent GameInstance that stays in memory across all level transitions.
// Manages global level transitions, per-level loop counters, objective states, and persistent save data.

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

	// --- Active Level Identification ---

	// Name of the currently active level
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|State")
	FName CurrentActiveLevelName;

	// Gets the active level name
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	FName GetCurrentActiveLevelName() const;

	// Sets the active level name manually
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void SetCurrentActiveLevelName(FName InLevelName);

	// --- Time Loop State (Per-Level and Active Level) ---

	// Loop count for the currently active level (maintained for backward compatibility)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|State")
	int32 CurrentLoopCount = 0;

	// Increments the loop count for the current active level and fires OnLoopCountChanged
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void IncrementLoopCount();

	// Increments the loop count for a specific level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void IncrementLoopCountForLevel(FName LevelName);

	// Resets the loop count for the current active level back to 0
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void ResetLoopCount();

	// Resets the loop count for a specific level back to 0
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void ResetLoopCountForLevel(FName LevelName);

	// Returns the loop count for the currently active level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	int32 GetCurrentLoopCount() const;

	// Returns the loop count for a specific level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	int32 GetLoopCountForLevel(FName LevelName) const;

	// Returns the cumulative loop count across all levels
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	int32 GetTotalGlobalLoopCount() const;

	// --- Level Data & Objectives ---

	// Persistent map of level data per level
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|State")
	TMap<FName, FTimeLoopLevelData> LevelDataMap;

	// Global persistent knowledge / clues across all levels
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|State")
	TArray<FName> GlobalPersistentKnowledge;

	// Retrieves level data for a specific level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	bool GetLevelData(FName LevelName, FTimeLoopLevelData& OutLevelData) const;

	// Retrieves level data for the current active level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	FTimeLoopLevelData GetCurrentLevelData() const;

	// Sets or updates level data for a specific level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void SetLevelData(FName LevelName, const FTimeLoopLevelData& InLevelData);

	// Returns true if the objective is solved for the specified level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	bool IsObjectiveSolved(FName LevelName) const;

	// Returns true if the objective is solved for the current level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	bool IsCurrentLevelObjectiveSolved() const;

	// Sets whether the objective is solved for a specific level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void SetObjectiveSolved(FName LevelName, bool bSolved = true);

	// Sets whether the objective is solved for the current active level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void SetCurrentLevelObjectiveSolved(bool bSolved = true);

	// Returns true if the level is marked completed
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	bool IsLevelCompleted(FName LevelName) const;

	// Marks a level as completed
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void SetLevelCompleted(FName LevelName, bool bCompleted = true);

	// Adds a discovered clue for a specific level
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void AddPersistentClue(FName LevelName, FName ClueID);

	// Checks if a clue has been discovered for a level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|State")
	bool HasPersistentClue(FName LevelName, FName ClueID) const;

	// Resets all level progression and loop counts
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|State")
	void ResetAllLevelsData();

	// --- Save / Load Game ---

	// Default save slot name
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	FString DefaultSaveSlotName = TEXT("TimeLoopSaveSlot");

	// Default user index
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	int32 DefaultUserIndex = 0;

	// Saves the game state to slot
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|SaveGame")
	bool SaveGame(const FString& SlotName = TEXT(""), int32 UserIndex = -1);

	// Loads the game state from slot
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|SaveGame")
	bool LoadGame(const FString& SlotName = TEXT(""), int32 UserIndex = -1);

	// Checks if a save game exists in slot
	UFUNCTION(BlueprintPure, Category = "TimeLoop|SaveGame")
	bool DoesSaveGameExist(const FString& SlotName = TEXT(""), int32 UserIndex = -1) const;

	// Deletes a save game file
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|SaveGame")
	bool DeleteSaveGame(const FString& SlotName = TEXT(""), int32 UserIndex = -1);

	// Creates a save game object populated with current game instance state
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|SaveGame")
	UTimeLoopSaveGame* CreateSaveGameObject() const;

	// Applies state from a save game object to this game instance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|SaveGame")
	void ApplySaveGameObject(const UTimeLoopSaveGame* SaveGameObject);

	// --- Event Delegates ---

	// Broadcast when the current level's loop count changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLoopCountChanged OnLoopCountChanged;

	// Broadcast when any level's loop count changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLevelLoopCountChanged OnLevelLoopCountChanged;

	// Broadcast when an objective solved status changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnObjectiveSolvedChanged OnObjectiveSolvedChanged;

	// Broadcast when a level transition is requested
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnLevelTransitionRequested OnLevelTransitionRequested;

	// Broadcast when a save game operation completes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnSaveGameOperationResult OnSaveGameSaved;

	// Broadcast when a load game operation completes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|Events")
	FOnSaveGameOperationResult OnSaveGameLoaded;

	// Helper to get or create level data reference
	FTimeLoopLevelData& GetOrCreateLevelData(FName LevelName);

	// Resolves a level name, falling back to CurrentActiveLevelName or world name if None
	FName ResolveLevelName(FName LevelName) const;

protected:
	// Handle for PostLoadMap delegate
	FDelegateHandle PostLoadMapHandle;

	// Called automatically when a level loads
	void HandlePostLoadMap(UWorld* LoadedWorld);
};
