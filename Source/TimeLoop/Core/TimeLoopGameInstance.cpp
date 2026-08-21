// Copyright (c) 2026 Team Ficus

#include "TimeLoopGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UTimeLoopGameInstance::UTimeLoopGameInstance()
{
	CurrentLoopCount = 0;
}

void UTimeLoopGameInstance::Init()
{
	Super::Init();
	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance initialized. Current Loop: %d"), CurrentLoopCount);
}

void UTimeLoopGameInstance::Shutdown()
{
	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance shutting down."));
	Super::Shutdown();
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
	CurrentLoopCount++;
	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Loop count incremented to %d"), CurrentLoopCount);
	
	if (OnLoopCountChanged.IsBound())
	{
		OnLoopCountChanged.Broadcast(CurrentLoopCount);
	}
}

void UTimeLoopGameInstance::ResetLoopCount()
{
	CurrentLoopCount = 0;
	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Loop count reset to 0"));
	
	if (OnLoopCountChanged.IsBound())
	{
		OnLoopCountChanged.Broadcast(CurrentLoopCount);
	}
}
