// Copyright (c) 2026 Team Ficus

#include "TimeLoopSaveGame.h"

UTimeLoopSaveGame::UTimeLoopSaveGame()
{
	SaveSlotName = TEXT("TimeLoopSaveSlot");
	UserIndex = 0;
	SaveTimestamp = FDateTime::UtcNow();
	CurrentLevelName = NAME_None;
	TotalGlobalLoops = 0;
	TotalPlayTimeSeconds = 0.0f;
}

bool UTimeLoopSaveGame::GetLevelData(FName InLevelName, FTimeLoopLevelData& OutLevelData) const
{
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(InLevelName))
	{
		OutLevelData = *Found;
		return true;
	}
	return false;
}

void UTimeLoopSaveGame::SetLevelData(FName InLevelName, const FTimeLoopLevelData& InLevelData)
{
	LevelDataMap.Add(InLevelName, InLevelData);
}

int32 UTimeLoopSaveGame::GetLoopCountForLevel(FName InLevelName) const
{
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(InLevelName))
	{
		return Found->LoopCount;
	}
	return 0;
}

bool UTimeLoopSaveGame::IsObjectiveSolvedForLevel(FName InLevelName) const
{
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(InLevelName))
	{
		return Found->bObjectiveSolved;
	}
	return false;
}

bool UTimeLoopSaveGame::IsLevelCompleted(FName InLevelName) const
{
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(InLevelName))
	{
		return Found->bLevelCompleted;
	}
	return false;
}

int32 UTimeLoopSaveGame::CalculateTotalLoops() const
{
	int32 Total = 0;
	for (const auto& Pair : LevelDataMap)
	{
		Total += Pair.Value.LoopCount;
	}
	return Total;
}
