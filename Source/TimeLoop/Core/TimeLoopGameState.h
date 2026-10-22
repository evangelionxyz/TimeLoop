// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TimeLoopSaveGame.h"
#include "TimeLoopGameState.generated.h"

class UTimeLoopGameInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameStateLoopCountChanged, int32, NewLoopCount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveStateChanged, bool, bSolved);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrologueStateChanged, bool, bInPrologue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStateChanged, bool, bInDialogue);

UCLASS()
class TIMELOOP_API ATimeLoopGameState : public AGameState
{
	GENERATED_BODY()

public:
	ATimeLoopGameState();

	virtual void BeginPlay() override;

	// --- Level State ---

	// The identifier/name of the current level managed by this GameState
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|GameState")
	FName LevelName;

	// Current loop count for this level
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|GameState")
	int32 CurrentLevelLoopCount = 0;

	// Whether the current level's objective (e.g. child saved, sniper blocked) is solved
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|GameState")
	bool bObjectiveSolved = false;

	// Whether the current level is marked completed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|GameState")
	bool bLevelCompleted = false;

	// --- Prologue ---

	// True while the level is in its calm pre-loop prologue. The loop timer must NOT run in this state.
	// Off by default so levels without a prologue are unaffected; ACollapseDirector turns it on.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|GameState")
	bool bInPrologue = false;

	// Enters the prologue (no loop timer, no loop HUD). Can be re-entered when resetting the loop.
	// Does not touch the loop count: your GameMode's existing IncrementLoopCount stays the single owner of it.
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void StartPrologue();

	// Leaves the prologue. Call this when the player dies for the first time, right before StartLoop().
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void EndPrologueAndInitiateLoop();

	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState")
	bool IsInPrologue() const { return bInPrologue; }

	// Broadcast when the prologue starts (true) or ends (false)
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|GameState|Events")
	FOnPrologueStateChanged OnPrologueStateChanged;

	// Dialogue

	// True while a conversation is on screen (player is advancing lines with the key). Other systems can check this.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TimeLoop|GameState|Dialogue")
	bool bInDialogue = false;

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState|Dialogue")
	void SetInDialogue(bool bNewInDialogue);

	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState|Dialogue")
	bool IsInDialogue() const { return bInDialogue; }


	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|GameState|Events")
	FOnDialogueStateChanged OnDialogueStateChanged;

	// --- Initialization & Sync ---

	// Initializes the GameState for a specific level name and syncs with GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void InitializeForLevel(FName InLevelName);

	// Synchronizes local state with persistent GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void SyncWithGameInstance();

	// --- Loop Counter Operations ---

	// Returns the loop count for this level
	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState")
	int32 GetLoopCount() const { return CurrentLevelLoopCount; }

	// Increments the loop count for this level and updates GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void IncrementLoopCount();

	// Resets the loop count for this level to 0 and updates GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void ResetLoopCount();

	// --- Objective & Level Completion ---

	// Returns true if the objective is solved
	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState")
	bool IsObjectiveSolved() const { return bObjectiveSolved; }

	// Sets the objective solved state and notifies GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void SetObjectiveSolved(bool bSolved = true);

	// Returns true if the level is completed
	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState")
	bool IsLevelCompleted() const { return bLevelCompleted; }

	// Sets the level completed state and notifies GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void SetLevelCompleted(bool bCompleted = true);

	// Retrieves the full level data structure from GameInstance
	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState")
	FTimeLoopLevelData GetLevelData() const;

	// --- Persistent Clues ---

	// Adds a persistent clue discovered during the loop
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	void AddPersistentClue(FName ClueID);

	// Checks if a persistent clue has been discovered
	UFUNCTION(BlueprintPure, Category = "TimeLoop|GameState")
	bool HasPersistentClue(FName ClueID) const;

	// --- Save / Load Shortcuts ---

	// Saves the current game state via GameInstance
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	bool SaveCurrentGameState(const FString& SlotName = TEXT(""), int32 UserIndex = -1);

	// Loads the game state via GameInstance and refreshes GameState
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|GameState")
	bool LoadCurrentGameState(const FString& SlotName = TEXT(""), int32 UserIndex = -1);

	// --- Event Delegates ---

	// Broadcast when the loop count for this level changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|GameState|Events")
	FOnGameStateLoopCountChanged OnLoopCountChanged;

	// Broadcast when the objective solved state changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|GameState|Events")
	FOnObjectiveStateChanged OnObjectiveSolvedChanged;

protected:
	// Helper to get typed GameInstance
	UTimeLoopGameInstance* GetTimeLoopGameInstance() const;

	// Handler for GameInstance loop count changes
	UFUNCTION()
	void HandleGameInstanceLoopCountChanged(FName ChangedLevelName, int32 NewLoopCount);

	// Handler for GameInstance objective solved changes
	UFUNCTION()
	void HandleGameInstanceObjectiveSolvedChanged(FName ChangedLevelName, bool bSolved);
};
