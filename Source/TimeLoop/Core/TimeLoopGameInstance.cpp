// Copyright (c) 2026 Team Ficus

#include "TimeLoopGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/Package.h"

UTimeLoopGameInstance::UTimeLoopGameInstance()
{
	MainMenuMapName = FName(TEXT("Lvl_MainMenu"));
	GameplayMapName = FName(TEXT("Lvl_TheCollapse"));
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
	if (!MainMenuMapName.IsNone())
	{
		return OpenLevelByName(MainMenuMapName);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: MainMenuMapName is not set!"));
	return false;
}

bool UTimeLoopGameInstance::OpenGameplayLevel()
{
	if (!GameplayMapName.IsNone())
	{
		return OpenLevelByName(GameplayMapName);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: GameplayMapName is not set!"));
	return false;
}

bool UTimeLoopGameInstance::OpenLevelByName(const FName LevelName)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::OpenLevelByName: LevelName is None!"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Opening level %s..."), *LevelName.ToString());
	
	if (OnLevelTransitionRequested.IsBound())
	{
		OnLevelTransitionRequested.Broadcast(LevelName);
	}

	UGameplayStatics::OpenLevel(this, LevelName);
	return true;
}

bool UTimeLoopGameInstance::AsyncOpenMainMenu()
{
	if (!MainMenuMapName.IsNone())
	{
		return AsyncOpenLevelByName(MainMenuMapName);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: MainMenuMapName is not set!"));
	return false;
}

bool UTimeLoopGameInstance::AsyncOpenGameplayLevel()
{
	if (!GameplayMapName.IsNone())
	{
		return AsyncOpenLevelByName(GameplayMapName);
	}

	UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: GameplayMapName is not set!"));
	return false;
}

bool UTimeLoopGameInstance::AsyncOpenLevelByName(const FName LevelName)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::AsyncOpenLevelByName: LevelName is None!"));
		return false;
	}

	if (const UWorld* World = GetWorld(); !World)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::AsyncOpenLevelByName: World is null!"));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Opening level %s asynchronously..."), *LevelName.ToString());

	if (OnLevelTransitionRequested.IsBound())
	{
		OnLevelTransitionRequested.Broadcast(LevelName);
	}

	LoadPackageAsync(LevelDirectoryName.ToString() + LevelName.ToString(),
		FLoadPackageAsyncDelegate::CreateLambda([this, LevelName](const FName& PackageName, UPackage* LoadedPackage, EAsyncLoadingResult::Type Result)
		{
			if (Result == EAsyncLoadingResult::Succeeded)
			{
				UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Level %s loaded successfully. Opening level..."), *PackageName.ToString());
				UGameplayStatics::OpenLevel(this, LevelName);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: Failed to async load level %s!"), *PackageName.ToString());
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
