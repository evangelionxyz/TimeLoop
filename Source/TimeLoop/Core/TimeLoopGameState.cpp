// Copyright (c) 2026 Team Ficus

#include "TimeLoopGameState.h"
#include "TimeLoopGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"

ATimeLoopGameState::ATimeLoopGameState()
{
	LevelName = NAME_None;
	CurrentLevelLoopCount = 0;
	bObjectiveSolved = false;
	bLevelCompleted = false;
}

void ATimeLoopGameState::BeginPlay()
{
	Super::BeginPlay();

	if (LevelName.IsNone())
	{
		if (const UWorld* World = GetWorld())
		{
			const FString RawMapName = World->GetOutermost()->GetName();
			const FString ShortName = FPackageName::GetShortName(RawMapName);
			const FString CleanMapName = UWorld::RemovePIEPrefix(ShortName);
			LevelName = FName(*CleanMapName);
		}
	}

	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		GI->OnLevelLoopCountChanged.AddDynamic(this, &ATimeLoopGameState::HandleGameInstanceLoopCountChanged);
		GI->OnObjectiveSolvedChanged.AddDynamic(this, &ATimeLoopGameState::HandleGameInstanceObjectiveSolvedChanged);
	}

	SyncWithGameInstance();
}

void ATimeLoopGameState::StartPrologue()
{
	// A level that has already looped is never in its prologue again.
	if (CurrentLevelLoopCount > 0 || bLevelCompleted)
	{
		bInPrologue = false;
		return;
	}

	if (!bInPrologue)
	{
		bInPrologue = true;
		OnPrologueStateChanged.Broadcast(true);
	}
}

void ATimeLoopGameState::EndPrologueAndInitiateLoop()
{
	if (bInPrologue)
	{
		bInPrologue = false;
		OnPrologueStateChanged.Broadcast(false);
	}
}

void ATimeLoopGameState::SetMayaState(EMayaState NewState)
{
	if (CurrentMayaState == NewState)
	{
		return;
	}

	const EMayaState OldState = CurrentMayaState;
	CurrentMayaState = NewState;
	OnMayaStateChanged.Broadcast(NewState, OldState);
}

void ATimeLoopGameState::SetInDialogue(bool bNewInDialogue)
{
	if (bInDialogue != bNewInDialogue)
	{
		bInDialogue = bNewInDialogue;
		OnDialogueStateChanged.Broadcast(bInDialogue);
	}
}

void ATimeLoopGameState::InitializeForLevel(FName InLevelName)
{
	LevelName = InLevelName;
	SyncWithGameInstance();
}

void ATimeLoopGameState::SyncWithGameInstance()
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		if (!LevelName.IsNone())
		{
			GI->SetCurrentActiveLevelName(LevelName);
		}
		else
		{
			LevelName = GI->GetCurrentActiveLevelName();
		}

		FTimeLoopLevelData Data;
		if (GI->GetLevelData(LevelName, Data))
		{
			CurrentLevelLoopCount = Data.LoopCount;
			bObjectiveSolved = Data.bObjectiveSolved;
			bLevelCompleted = Data.bLevelCompleted;
		}
		else
		{
			CurrentLevelLoopCount = 0;
			bObjectiveSolved = false;
			bLevelCompleted = false;
		}
	}
}

void ATimeLoopGameState::IncrementLoopCount()
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		GI->IncrementLoopCountForLevel(LevelName);
		CurrentLevelLoopCount = GI->GetLoopCountForLevel(LevelName);
	}
	else
	{
		CurrentLevelLoopCount++;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(CurrentLevelLoopCount);
		}
	}
}

void ATimeLoopGameState::ResetLoopCount()
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		GI->ResetLoopCountForLevel(LevelName);
		CurrentLevelLoopCount = 0;
	}
	else
	{
		CurrentLevelLoopCount = 0;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(0);
		}
	}
}

void ATimeLoopGameState::SetObjectiveSolved(bool bSolved)
{
	bObjectiveSolved = bSolved;
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		GI->SetObjectiveSolved(LevelName, bSolved);
	}
	else
	{
		if (OnObjectiveSolvedChanged.IsBound())
		{
			OnObjectiveSolvedChanged.Broadcast(bSolved);
		}
	}
}

void ATimeLoopGameState::SetLevelCompleted(bool bCompleted)
{
	bLevelCompleted = bCompleted;
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		GI->SetLevelCompleted(LevelName, bCompleted);
	}
}

FTimeLoopLevelData ATimeLoopGameState::GetLevelData() const
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		FTimeLoopLevelData Data;
		if (GI->GetLevelData(LevelName, Data))
		{
			return Data;
		}
	}

	FTimeLoopLevelData FallbackData;
	FallbackData.LevelName = LevelName;
	FallbackData.LoopCount = CurrentLevelLoopCount;
	FallbackData.bObjectiveSolved = bObjectiveSolved;
	FallbackData.bLevelCompleted = bLevelCompleted;
	return FallbackData;
}

void ATimeLoopGameState::AddPersistentClue(FName ClueID)
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		GI->AddPersistentClue(LevelName, ClueID);
	}
}

bool ATimeLoopGameState::HasPersistentClue(FName ClueID) const
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		return GI->HasPersistentClue(LevelName, ClueID);
	}
	return false;
}

bool ATimeLoopGameState::SaveCurrentGameState(const FString& SlotName, int32 UserIndex)
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		return GI->SaveGame(SlotName, UserIndex);
	}
	return false;
}

bool ATimeLoopGameState::LoadCurrentGameState(const FString& SlotName, int32 UserIndex)
{
	if (UTimeLoopGameInstance* GI = GetTimeLoopGameInstance())
	{
		if (GI->LoadGame(SlotName, UserIndex))
		{
			SyncWithGameInstance();
			if (OnLoopCountChanged.IsBound())
			{
				OnLoopCountChanged.Broadcast(CurrentLevelLoopCount);
			}
			if (OnObjectiveSolvedChanged.IsBound())
			{
				OnObjectiveSolvedChanged.Broadcast(bObjectiveSolved);
			}
			return true;
		}
	}
	return false;
}

UTimeLoopGameInstance* ATimeLoopGameState::GetTimeLoopGameInstance() const
{
	return Cast<UTimeLoopGameInstance>(GetGameInstance());
}

void ATimeLoopGameState::HandleGameInstanceLoopCountChanged(FName ChangedLevelName, int32 NewLoopCount)
{
	if (ChangedLevelName == LevelName || LevelName.IsNone())
	{
		CurrentLevelLoopCount = NewLoopCount;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(NewLoopCount);
		}
	}
}

void ATimeLoopGameState::HandleGameInstanceObjectiveSolvedChanged(FName ChangedLevelName, bool bSolved)
{
	if (ChangedLevelName == LevelName || LevelName.IsNone())
	{
		bObjectiveSolved = bSolved;
		if (OnObjectiveSolvedChanged.IsBound())
		{
			OnObjectiveSolvedChanged.Broadcast(bSolved);
		}
	}
}
