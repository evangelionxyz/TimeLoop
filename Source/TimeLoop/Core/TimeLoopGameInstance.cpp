// Copyright (c) 2026 Team Ficus

#include "TimeLoopGameInstance.h"
#include "TimeLoopSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"
#include "Misc/PackageName.h"

UTimeLoopGameInstance::UTimeLoopGameInstance()
{
	CurrentLoopCount = 0;
	CurrentActiveLevelName = NAME_None;
	DefaultSaveSlotName = TEXT("TimeLoopSaveSlot");
	DefaultUserIndex = 0;
}

void UTimeLoopGameInstance::Init()
{
	Super::Init();

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTimeLoopGameInstance::HandlePostLoadMap);
	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance initialized."));
}

void UTimeLoopGameInstance::Shutdown()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance shutting down."));
	Super::Shutdown();
}

void UTimeLoopGameInstance::HandlePostLoadMap(UWorld* LoadedWorld)
{
	if (!LoadedWorld || !LoadedWorld->IsGameWorld())
	{
		return;
	}

	const FString RawMapName = LoadedWorld->GetOutermost()->GetName();
	const FString ShortName = FPackageName::GetShortName(RawMapName);
	const FString CleanMapName = UWorld::RemovePIEPrefix(ShortName);
	const FName MapName(*CleanMapName);

	if (CurrentActiveLevelName.IsNone() || CurrentActiveLevelName != MapName)
	{
		CurrentActiveLevelName = MapName;
	}

	CurrentLoopCount = GetLoopCountForLevel(CurrentActiveLevelName);
	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Active level set to '%s'. Loop Count: %d"), *CurrentActiveLevelName.ToString(), CurrentLoopCount);

	if (OnLoopCountChanged.IsBound())
	{
		OnLoopCountChanged.Broadcast(CurrentLoopCount);
	}
}

FName UTimeLoopGameInstance::GetCurrentActiveLevelName() const
{
	return CurrentActiveLevelName;
}

void UTimeLoopGameInstance::SetCurrentActiveLevelName(FName InLevelName)
{
	CurrentActiveLevelName = InLevelName;
	CurrentLoopCount = GetLoopCountForLevel(CurrentActiveLevelName);

	if (OnLoopCountChanged.IsBound())
	{
		OnLoopCountChanged.Broadcast(CurrentLoopCount);
	}
}

FName UTimeLoopGameInstance::ResolveLevelName(FName LevelName) const
{
	if (!LevelName.IsNone())
	{
		return LevelName;
	}

	if (!CurrentActiveLevelName.IsNone())
	{
		return CurrentActiveLevelName;
	}

	if (const UWorld* World = GetWorld())
	{
		const FString RawMapName = World->GetOutermost()->GetName();
		const FString ShortName = FPackageName::GetShortName(RawMapName);
		return FName(*UWorld::RemovePIEPrefix(ShortName));
	}

	return NAME_None;
}

FTimeLoopLevelData& UTimeLoopGameInstance::GetOrCreateLevelData(FName LevelName)
{
	if (FTimeLoopLevelData* Found = LevelDataMap.Find(LevelName))
	{
		return *Found;
	}

	FTimeLoopLevelData NewData;
	NewData.LevelName = LevelName;
	NewData.LoopCount = 0;
	NewData.bObjectiveSolved = false;
	NewData.bLevelCompleted = false;
	NewData.bUnlocked = true;
	NewData.BestLoopCount = 0;
	NewData.BestCompletionTime = 0.0f;

	return LevelDataMap.Add(LevelName, NewData);
}

bool UTimeLoopGameInstance::OpenMainMenu()
{
	if (!MainMenuLevel.IsNull())
	{
		return OpenLevelByReference(MainMenuLevel);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: MainMenuLevel is not set!"));
	return false;
}

bool UTimeLoopGameInstance::OpenGameplayLevel()
{
	if (!GameplayLevel.IsNull())
	{
		return OpenLevelByReference(GameplayLevel);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: GameplayLevel is not set!"));
	return false;
}

bool UTimeLoopGameInstance::OpenLevelByReference(const TSoftObjectPtr<UWorld> Level)
{
	if (Level.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::OpenLevelByReference: Level is null!"));
		return false;
	}

	const FSoftObjectPath LevelPath = Level.ToSoftObjectPath();
	const FName TargetLevelName(*LevelPath.GetAssetName());

	CurrentActiveLevelName = TargetLevelName;
	CurrentLoopCount = GetLoopCountForLevel(TargetLevelName);

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Opening level %s..."), *LevelPath.ToString());
	
	if (OnLevelTransitionRequested.IsBound())
	{
		OnLevelTransitionRequested.Broadcast(TargetLevelName);
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, Level);
	return true;
}

bool UTimeLoopGameInstance::AsyncOpenMainMenu()
{
	if (!MainMenuLevel.IsNull())
	{
		return AsyncOpenLevelByReference(MainMenuLevel);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: MainMenuLevel is not set!"));
	return false;
}

bool UTimeLoopGameInstance::AsyncOpenGameplayLevel()
{
	if (!GameplayLevel.IsNull())
	{
		return AsyncOpenLevelByReference(GameplayLevel);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: GameplayLevel is not set!"));
	return false;
}

bool UTimeLoopGameInstance::AsyncOpenLevelByReference(const TSoftObjectPtr<UWorld> Level)
{
	if (Level.IsNull())
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::AsyncOpenLevelByReference: Level is null!"));
		return false;
	}

	if (const UWorld* World = GetWorld(); !World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::AsyncOpenLevelByReference: World is null!"));
		return false;
	}

	const FSoftObjectPath LevelPath = Level.ToSoftObjectPath();
	const FName TargetLevelName(*LevelPath.GetAssetName());
	const FString PackageName = LevelPath.GetLongPackageName();

	if (PackageName.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::AsyncOpenLevelByReference: Invalid level path '%s'"), *LevelPath.ToString());
		return false;
	}

	CurrentActiveLevelName = TargetLevelName;
	CurrentLoopCount = GetLoopCountForLevel(TargetLevelName);

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Opening level %s asynchronously..."), *LevelPath.ToString());

	if (OnLevelTransitionRequested.IsBound())
	{
		OnLevelTransitionRequested.Broadcast(TargetLevelName);
	}

	LoadPackageAsync(PackageName,
		FLoadPackageAsyncDelegate::CreateLambda([this, Level, TargetLevelName](const FName& LoadedPackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
		{
			if (Result == EAsyncLoadingResult::Succeeded)
			{
				UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Level %s loaded successfully. Opening level..."), *LoadedPackageName.ToString());
				UGameplayStatics::OpenLevelBySoftObjectPtr(this, Level);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: Failed to async load level %s!"), *TargetLevelName.ToString());
			}
		}), 0, PKG_ContainsMap
	);

	return true;
}

void UTimeLoopGameInstance::IncrementLoopCount()
{
	const FName Level = ResolveLevelName(NAME_None);
	IncrementLoopCountForLevel(Level);
}

void UTimeLoopGameInstance::IncrementLoopCountForLevel(FName LevelName)
{
	const FName Level = ResolveLevelName(LevelName);
	if (Level.IsNone())
	{
		CurrentLoopCount++;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(CurrentLoopCount);
		}
		return;
	}

	FTimeLoopLevelData& LevelData = GetOrCreateLevelData(Level);
	LevelData.LoopCount++;

	if (Level == CurrentActiveLevelName || CurrentActiveLevelName.IsNone())
	{
		CurrentLoopCount = LevelData.LoopCount;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(CurrentLoopCount);
		}
	}

	if (OnLevelLoopCountChanged.IsBound())
	{
		OnLevelLoopCountChanged.Broadcast(Level, LevelData.LoopCount);
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Loop count for level '%s' incremented to %d"), *Level.ToString(), LevelData.LoopCount);
}

void UTimeLoopGameInstance::ResetLoopCount()
{
	const FName Level = ResolveLevelName(NAME_None);
	ResetLoopCountForLevel(Level);
}

void UTimeLoopGameInstance::ResetLoopCountForLevel(FName LevelName)
{
	const FName Level = ResolveLevelName(LevelName);
	if (Level.IsNone())
	{
		CurrentLoopCount = 0;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(0);
		}
		return;
	}

	FTimeLoopLevelData& LevelData = GetOrCreateLevelData(Level);
	LevelData.LoopCount = 0;

	if (Level == CurrentActiveLevelName || CurrentActiveLevelName.IsNone())
	{
		CurrentLoopCount = 0;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(0);
		}
	}

	if (OnLevelLoopCountChanged.IsBound())
	{
		OnLevelLoopCountChanged.Broadcast(Level, 0);
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Loop count for level '%s' reset to 0"), *Level.ToString());
}

int32 UTimeLoopGameInstance::GetCurrentLoopCount() const
{
	const FName Level = ResolveLevelName(NAME_None);
	if (!Level.IsNone())
	{
		return GetLoopCountForLevel(Level);
	}
	return CurrentLoopCount;
}

int32 UTimeLoopGameInstance::GetLoopCountForLevel(FName LevelName) const
{
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(LevelName))
	{
		return Found->LoopCount;
	}
	return 0;
}

int32 UTimeLoopGameInstance::GetTotalGlobalLoopCount() const
{
	int32 Total = 0;
	for (const auto& Pair : LevelDataMap)
	{
		Total += Pair.Value.LoopCount;
	}
	return Total;
}

bool UTimeLoopGameInstance::GetLevelData(FName LevelName, FTimeLoopLevelData& OutLevelData) const
{
	const FName Level = ResolveLevelName(LevelName);
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(Level))
	{
		OutLevelData = *Found;
		return true;
	}
	return false;
}

FTimeLoopLevelData UTimeLoopGameInstance::GetCurrentLevelData() const
{
	FTimeLoopLevelData OutData;
	GetLevelData(CurrentActiveLevelName, OutData);
	return OutData;
}

void UTimeLoopGameInstance::SetLevelData(FName LevelName, const FTimeLoopLevelData& InLevelData)
{
	const FName Level = ResolveLevelName(LevelName);
	LevelDataMap.Add(Level, InLevelData);

	if (Level == CurrentActiveLevelName || CurrentActiveLevelName.IsNone())
	{
		CurrentLoopCount = InLevelData.LoopCount;
		if (OnLoopCountChanged.IsBound())
		{
			OnLoopCountChanged.Broadcast(CurrentLoopCount);
		}
	}

	if (OnLevelLoopCountChanged.IsBound())
	{
		OnLevelLoopCountChanged.Broadcast(Level, InLevelData.LoopCount);
	}
}

bool UTimeLoopGameInstance::IsObjectiveSolved(FName LevelName) const
{
	const FName Level = ResolveLevelName(LevelName);
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(Level))
	{
		return Found->bObjectiveSolved;
	}
	return false;
}

bool UTimeLoopGameInstance::IsCurrentLevelObjectiveSolved() const
{
	return IsObjectiveSolved(CurrentActiveLevelName);
}

void UTimeLoopGameInstance::SetObjectiveSolved(FName LevelName, bool bSolved)
{
	const FName Level = ResolveLevelName(LevelName);
	FTimeLoopLevelData& LevelData = GetOrCreateLevelData(Level);
	LevelData.bObjectiveSolved = bSolved;

	if (OnObjectiveSolvedChanged.IsBound())
	{
		OnObjectiveSolvedChanged.Broadcast(Level, bSolved);
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Objective for level '%s' set to %s"), *Level.ToString(), bSolved ? TEXT("SOLVED") : TEXT("UNSOLVED"));
}

void UTimeLoopGameInstance::SetCurrentLevelObjectiveSolved(bool bSolved)
{
	SetObjectiveSolved(CurrentActiveLevelName, bSolved);
}

bool UTimeLoopGameInstance::IsLevelCompleted(FName LevelName) const
{
	const FName Level = ResolveLevelName(LevelName);
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(Level))
	{
		return Found->bLevelCompleted;
	}
	return false;
}

void UTimeLoopGameInstance::SetLevelCompleted(FName LevelName, bool bCompleted)
{
	const FName Level = ResolveLevelName(LevelName);
	FTimeLoopLevelData& LevelData = GetOrCreateLevelData(Level);
	LevelData.bLevelCompleted = bCompleted;

	if (bCompleted)
	{
		if (LevelData.BestLoopCount == 0 || LevelData.LoopCount < LevelData.BestLoopCount)
		{
			LevelData.BestLoopCount = LevelData.LoopCount;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Level '%s' completed state set to %s (Loop: %d, Best: %d)"),
		*Level.ToString(), bCompleted ? TEXT("TRUE") : TEXT("FALSE"), LevelData.LoopCount, LevelData.BestLoopCount);
}

void UTimeLoopGameInstance::AddPersistentClue(FName LevelName, FName ClueID)
{
	const FName Level = ResolveLevelName(LevelName);
	FTimeLoopLevelData& LevelData = GetOrCreateLevelData(Level);
	LevelData.DiscoveredClues.AddUnique(ClueID);
	GlobalPersistentKnowledge.AddUnique(ClueID);

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Added persistent clue '%s' for level '%s'"), *ClueID.ToString(), *Level.ToString());
}

bool UTimeLoopGameInstance::HasPersistentClue(FName LevelName, FName ClueID) const
{
	const FName Level = ResolveLevelName(LevelName);
	if (const FTimeLoopLevelData* Found = LevelDataMap.Find(Level))
	{
		if (Found->DiscoveredClues.Contains(ClueID))
		{
			return true;
		}
	}

	return GlobalPersistentKnowledge.Contains(ClueID);
}

void UTimeLoopGameInstance::ResetAllLevelsData()
{
	LevelDataMap.Empty();
	GlobalPersistentKnowledge.Empty();
	CurrentLoopCount = 0;

	if (OnLoopCountChanged.IsBound())
	{
		OnLoopCountChanged.Broadcast(0);
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: All level data and loop counters reset."));
}

bool UTimeLoopGameInstance::SaveGame(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? DefaultSaveSlotName : SlotName;
	const int32 FinalIndex = UserIndex < 0 ? DefaultUserIndex : UserIndex;

	UTimeLoopSaveGame* SaveObj = CreateSaveGameObject();
	if (!SaveObj)
	{
		if (OnSaveGameSaved.IsBound())
		{
			OnSaveGameSaved.Broadcast(FinalSlot, false);
		}
		return false;
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveObj, FinalSlot, FinalIndex);
	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Game saved successfully to slot '%s'"), *FinalSlot);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: Failed to save game to slot '%s'"), *FinalSlot);
	}

	if (OnSaveGameSaved.IsBound())
	{
		OnSaveGameSaved.Broadcast(FinalSlot, bSuccess);
	}

	return bSuccess;
}

bool UTimeLoopGameInstance::LoadGame(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? DefaultSaveSlotName : SlotName;
	const int32 FinalIndex = UserIndex < 0 ? DefaultUserIndex : UserIndex;

	if (!UGameplayStatics::DoesSaveGameExist(FinalSlot, FinalIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: Save slot '%s' does not exist!"), *FinalSlot);
		if (OnSaveGameLoaded.IsBound())
		{
			OnSaveGameLoaded.Broadcast(FinalSlot, false);
		}
		return false;
	}

	USaveGame* LoadedGame = UGameplayStatics::LoadGameFromSlot(FinalSlot, FinalIndex);
	const UTimeLoopSaveGame* SaveObj = Cast<UTimeLoopSaveGame>(LoadedGame);
	if (!SaveObj)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: Failed to cast save game from slot '%s'"), *FinalSlot);
		if (OnSaveGameLoaded.IsBound())
		{
			OnSaveGameLoaded.Broadcast(FinalSlot, false);
		}
		return false;
	}

	ApplySaveGameObject(SaveObj);

	if (OnSaveGameLoaded.IsBound())
	{
		OnSaveGameLoaded.Broadcast(FinalSlot, true);
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Game loaded successfully from slot '%s'"), *FinalSlot);
	return true;
}

bool UTimeLoopGameInstance::DoesSaveGameExist(const FString& SlotName, int32 UserIndex) const
{
	const FString FinalSlot = SlotName.IsEmpty() ? DefaultSaveSlotName : SlotName;
	const int32 FinalIndex = UserIndex < 0 ? DefaultUserIndex : UserIndex;
	return UGameplayStatics::DoesSaveGameExist(FinalSlot, FinalIndex);
}

bool UTimeLoopGameInstance::DeleteSaveGame(const FString& SlotName, int32 UserIndex)
{
	const FString FinalSlot = SlotName.IsEmpty() ? DefaultSaveSlotName : SlotName;
	const int32 FinalIndex = UserIndex < 0 ? DefaultUserIndex : UserIndex;
	return UGameplayStatics::DeleteGameInSlot(FinalSlot, FinalIndex);
}

UTimeLoopSaveGame* UTimeLoopGameInstance::CreateSaveGameObject() const
{
	UTimeLoopSaveGame* SaveObj = Cast<UTimeLoopSaveGame>(UGameplayStatics::CreateSaveGameObject(UTimeLoopSaveGame::StaticClass()));
	if (SaveObj)
	{
		SaveObj->SaveSlotName = DefaultSaveSlotName;
		SaveObj->UserIndex = DefaultUserIndex;
		SaveObj->SaveTimestamp = FDateTime::UtcNow();
		SaveObj->CurrentLevelName = CurrentActiveLevelName;
		SaveObj->LevelDataMap = LevelDataMap;
		SaveObj->GlobalPersistentKnowledge = GlobalPersistentKnowledge;
		SaveObj->TotalGlobalLoops = GetTotalGlobalLoopCount();
	}
	return SaveObj;
}

void UTimeLoopGameInstance::ApplySaveGameObject(const UTimeLoopSaveGame* SaveGameObject)
{
	if (!SaveGameObject)
	{
		return;
	}

	LevelDataMap = SaveGameObject->LevelDataMap;
	GlobalPersistentKnowledge = SaveGameObject->GlobalPersistentKnowledge;

	if (!SaveGameObject->CurrentLevelName.IsNone())
	{
		CurrentActiveLevelName = SaveGameObject->CurrentLevelName;
	}

	CurrentLoopCount = GetLoopCountForLevel(CurrentActiveLevelName);

	if (OnLoopCountChanged.IsBound())
	{
		OnLoopCountChanged.Broadcast(CurrentLoopCount);
	}
}
