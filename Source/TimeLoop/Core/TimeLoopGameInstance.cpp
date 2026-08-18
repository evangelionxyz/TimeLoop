// Copyright (c) 2026 Team Ficus

#include "TimeLoopGameInstance.h"
#include "Kismet/GameplayStatics.h"

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

void UTimeLoopGameInstance::OpenMainMenu()
{
	if (!MainMenuMapName.IsNone())
	{
		OpenLevelByName(MainMenuMapName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: MainMenuMapName is not set!"));
	}
}

void UTimeLoopGameInstance::OpenGameplayLevel()
{
	if (!GameplayMapName.IsNone())
	{
		OpenLevelByName(GameplayMapName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance: GameplayMapName is not set!"));
	}
}

void UTimeLoopGameInstance::OpenLevelByName(FName LevelName)
{
	if (LevelName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopGameInstance::OpenLevelByName: LevelName is None!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("TimeLoopGameInstance: Opening level %s..."), *LevelName.ToString());
	
	if (OnLevelTransitionRequested.IsBound())
	{
		OnLevelTransitionRequested.Broadcast(LevelName);
	}

	UGameplayStatics::OpenLevel(this, LevelName);
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
