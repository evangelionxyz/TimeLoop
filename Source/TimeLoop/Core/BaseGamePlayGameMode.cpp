// Copyright (c) 2026 Team Ficus

#include "BaseGamePlayGameMode.h"
#include "CollapseDirector.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"

ABaseGamePlayGameMode::ABaseGamePlayGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	MaxLoopDuration = 30.0f;
	TimeRemaining = 0.0f;
	FailureResetDelay = 0.5f;
	CurrentLoopState = ELoopState::Inactive;
	EndResult = ECollapseEndResult::Timeout;
	LevelName = FName(TEXT("Please fill the Level Name !!!!!!"));
	LevelUIDisplayName = FText::FromString(TEXT("Please fill the Level Name !!!!!!"));
}

void ABaseGamePlayGameMode::BeginPlay()
{
	Super::BeginPlay();

	TLGameState = Cast<ATimeLoopGameState>(GameState);
	if (TLGameState)
	{
		TLGameState->InitializeForLevel(LevelName);
	}

	StartPlayGame();
}

void ABaseGamePlayGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Do not update when paused
	if (!bPaused && CurrentLoopState == ELoopState::Active)
	{
		UpdateTimer(DeltaSeconds);
	}
}

void ABaseGamePlayGameMode::StartPlayGame()
{
	PreStartGame();

	const int32 LoopCount = TLGameState ? TLGameState->GetLoopCount() : 0;
	if (LoopCount == 0)
	{
		if (TLGameState)
		{
			TLGameState->StartPrologue();
		}
		SetCurrentLoopState(ELoopState::Inactive);
	}
	else
	{
		StartLoop();
	}
}

void ABaseGamePlayGameMode::PreStartGame()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = false;
		GameHUD = PC->GetHUD();
	}
}

void ABaseGamePlayGameMode::StartLoop()
{
	if (TLGameState)
	{
		TLGameState->EndPrologueAndInitiateLoop();
	}

	SetCurrentLoopState(ELoopState::Active);
	TimeRemaining = MaxLoopDuration;
	OnUpdateTimer.Broadcast(TimeRemaining);
}

void ABaseGamePlayGameMode::UpdateTimer(float DeltaSeconds)
{
	TimeRemaining -= DeltaSeconds;
	OnUpdateTimer.Broadcast(FMath::Max(TimeRemaining, 0.0f));

	if (TimeRemaining <= 0.0f)
	{
		TimeRemaining = 0.0f;
		TriggerLoopFailure(ECollapseEndResult::Timeout);
	}
}

void ABaseGamePlayGameMode::TriggerLoopFailure(ECollapseEndResult InEndResult)
{
	if (CurrentLoopState != ELoopState::Active)
	{
		return;
	}

	EndResult = InEndResult;
	SetCurrentLoopState(ELoopState::Failed);

	GetWorldTimerManager().ClearTimer(LoopTimerHandle);
	OnLoopFailed.Broadcast();

	GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ABaseGamePlayGameMode::ResetLoop, FMath::Max(FailureResetDelay, 0.01f), false);
}

void ABaseGamePlayGameMode::TriggerLoopSuccess()
{
	EndResult = ECollapseEndResult::Success;
	SetCurrentLoopState(ELoopState::Succeeded);
	GetWorldTimerManager().ClearTimer(LoopTimerHandle);

	if (TLGameState)
	{
		TLGameState->SetLevelCompleted(true);
	}

	OnLoopSucceeded.Broadcast();
}

void ABaseGamePlayGameMode::ResetLoop()
{
	UE_LOG(LogTemp, Warning, TEXT("[GameMode] ResetLoop started. Resetting loop timer and notifying resetables..."));

	TimeRemaining = MaxLoopDuration;
	OnUpdateTimer.Broadcast(TimeRemaining);

	if (TLGameState)
	{
		TLGameState->IncrementLoopCount();
	}

	// 1. Reset all actors implementing BPI_LoopResetable
	if (UClass* InterfaceClass = StaticLoadClass(UInterface::StaticClass(), nullptr, TEXT("/Game/Core/Interfaces/BPI_LoopResetable.BPI_LoopResetable_C")))
	{
		TArray<AActor*> ResetableActors;
		UGameplayStatics::GetAllActorsWithInterface(this, InterfaceClass, ResetableActors);
		for (AActor* ResetableActor : ResetableActors)
		{
			if (ResetableActor)
			{
				if (UFunction* ResetFunc = ResetableActor->FindFunction(TEXT("OnLoopReset")))
				{
					ResetableActor->ProcessEvent(ResetFunc, nullptr);
				}
			}
		}
	}

	// 2. Reset registered resetables
	for (UObject* Resetable : RegisteredResetables)
	{
		if (Resetable)
		{
			if (UFunction* ResetFunc = Resetable->FindFunction(TEXT("OnLoopReset")))
			{
				Resetable->ProcessEvent(ResetFunc, nullptr);
			}
		}
	}

	// 3. Notify Level Director if present (sets phase to Prologue, resets Maya to drawing, retriggers quake/blackout)
	if (ACollapseDirector* Director = Cast<ACollapseDirector>(UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass())))
	{
		UE_LOG(LogTemp, Warning, TEXT("[GameMode] Notifying CollapseDirector::ResetState with Result: %d"), static_cast<int32>(EndResult));
		Director->ResetState(EndResult);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[GameMode] Could not find ACollapseDirector in world!"));
	}

	K2_OnLoopReset();
}

void ABaseGamePlayGameMode::RegisterResetable(UObject* Resetable)
{
	if (Resetable && !RegisteredResetables.Contains(Resetable))
	{
		RegisteredResetables.Add(Resetable);
	}
}

void ABaseGamePlayGameMode::StartNewGame()
{
	PreStartGame();
	StartLoop();
}

void ABaseGamePlayGameMode::ReturnToMenu()
{
	UGameplayStatics::OpenLevel(this, TEXT("Lvl_MainMenu"));
}

void ABaseGamePlayGameMode::SetCurrentLoopState(ELoopState NewState)
{
	CurrentLoopState = NewState;
}
