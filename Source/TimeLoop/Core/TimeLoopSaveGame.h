// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "TimeLoopSaveGame.generated.h"

// FTimeLoopLevelData
// Stores persistent progression, loop counts, and puzzle completion states per level.
USTRUCT(BlueprintType)
struct TIMELOOP_API FTimeLoopLevelData
{
	GENERATED_BODY()

	// Identifier / Name of the level (e.g. Lvl_TheCollapse, Lvl_TheCrosshairs, Lvl_ToxinsInTheGlass)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	FName LevelName = NAME_None;

	// Loop iteration count for this specific level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	int32 LoopCount = 0;

	// Whether the level's core objective / target sacrifice has been solved
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	bool bObjectiveSolved = false;

	// Whether this level has been cleared / completed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	bool bLevelCompleted = false;

	// Whether this level is unlocked for selection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	bool bUnlocked = true;

	// Fewest loop attempts to solve/clear this level (0 = unrecorded)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	int32 BestLoopCount = 0;

	// Fastest completion time in seconds (0.0 = unrecorded)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	float BestCompletionTime = 0.0f;

	// Persistent clues and knowledge discovered across loops (e.g. "NoticedBathtub", "FoundSafeGlass")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	TArray<FName> DiscoveredClues;

	// Arbitrary custom key-value pairs for level-specific persistent state
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeLoop|LevelData")
	TMap<FName, FString> CustomState;
};

// UTimeLoopSaveGame
// Save game object storing user progression, per-level loop counters, objectives, and persistent knowledge.
UCLASS()
class TIMELOOP_API UTimeLoopSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UTimeLoopSaveGame();

	// Default slot name identifier
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	FString SaveSlotName;

	// User index for local player
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	int32 UserIndex;

	// Timestamp when the save was recorded
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	FDateTime SaveTimestamp;

	// Last active level name when saved
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	FName CurrentLevelName;

	// Per-level data map (Key = LevelName)
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	TMap<FName, FTimeLoopLevelData> LevelDataMap;

	// Global persistent knowledge / clues across all levels
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	TArray<FName> GlobalPersistentKnowledge;

	// Cumulative loop count across all levels
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	int32 TotalGlobalLoops;

	// Total play time in seconds
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TimeLoop|SaveGame")
	float TotalPlayTimeSeconds;

	// --- Helper Queries & Mutators ---

	UFUNCTION(BlueprintPure, Category = "TimeLoop|SaveGame")
	bool GetLevelData(FName InLevelName, FTimeLoopLevelData& OutLevelData) const;

	UFUNCTION(BlueprintCallable, Category = "TimeLoop|SaveGame")
	void SetLevelData(FName InLevelName, const FTimeLoopLevelData& InLevelData);

	UFUNCTION(BlueprintPure, Category = "TimeLoop|SaveGame")
	int32 GetLoopCountForLevel(FName InLevelName) const;

	UFUNCTION(BlueprintPure, Category = "TimeLoop|SaveGame")
	bool IsObjectiveSolvedForLevel(FName InLevelName) const;

	UFUNCTION(BlueprintPure, Category = "TimeLoop|SaveGame")
	bool IsLevelCompleted(FName InLevelName) const;

	UFUNCTION(BlueprintPure, Category = "TimeLoop|SaveGame")
	int32 CalculateTotalLoops() const;
};
