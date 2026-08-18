// Copyright (c) 2026 Team Ficus

#include "TimeLoopUIManagerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

void UTimeLoopUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UTimeLoopUIManagerSubsystem::HandlePostLoadMap);
	UE_LOG(LogTemp, Log, TEXT("TimeLoopUIManagerSubsystem initialized."));
}

void UTimeLoopUIManagerSubsystem::Deinitialize()
{
	if (PostLoadMapHandle.IsValid())
	{
		FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
		PostLoadMapHandle.Reset();
	}

	ClearAllWidgets();
	UE_LOG(LogTemp, Log, TEXT("TimeLoopUIManagerSubsystem deinitialized."));
	Super::Deinitialize();
}

UUserWidget* UTimeLoopUIManagerSubsystem::GetOrCreateWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* OwningPlayer)
{
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopUIManagerSubsystem::GetOrCreateWidget: WidgetClass is null!"));
		return nullptr;
	}

	// Check if already created and valid
	if (TObjectPtr<UUserWidget>* FoundWidget = CachedWidgets.Find(WidgetClass))
	{
		if (FoundWidget->Get() && IsValid(FoundWidget->Get()))
		{
			return FoundWidget->Get();
		}
	}

	// Create widget parented to GameInstance so it survives level travel
	UUserWidget* NewWidget = nullptr;
	if (OwningPlayer)
	{
		NewWidget = CreateWidget<UUserWidget>(OwningPlayer, WidgetClass);
	}
	else
	{
		NewWidget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
	}

	if (NewWidget)
	{
		CachedWidgets.Add(WidgetClass, NewWidget);
		UE_LOG(LogTemp, Log, TEXT("TimeLoopUIManagerSubsystem: Created widget instance of %s"), *WidgetClass->GetName());
	}

	return NewWidget;
}

UUserWidget* UTimeLoopUIManagerSubsystem::ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder, APlayerController* OwningPlayer)
{
	if (!WidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TimeLoopUIManagerSubsystem::ShowWidget: WidgetClass is null!"));
		return nullptr;
	}

	UUserWidget* Widget = GetOrCreateWidget(WidgetClass, OwningPlayer);
	if (Widget)
	{
		if (!Widget->IsInViewport())
		{
			Widget->AddToViewport(ZOrder);
		}
		Widget->SetVisibility(ESlateVisibility::Visible);

		CurrentActiveWidget = Widget;

		if (OnActiveWidgetChanged.IsBound())
		{
			OnActiveWidgetChanged.Broadcast(Widget);
		}
	}

	return Widget;
}

void UTimeLoopUIManagerSubsystem::HideWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return;
	}

	if (TObjectPtr<UUserWidget>* FoundWidget = CachedWidgets.Find(WidgetClass))
	{
		if (FoundWidget->Get() && IsValid(FoundWidget->Get()))
		{
			FoundWidget->Get()->SetVisibility(ESlateVisibility::Collapsed);
			
			if (CurrentActiveWidget == FoundWidget->Get())
			{
				CurrentActiveWidget = nullptr;
				if (OnActiveWidgetChanged.IsBound())
				{
					OnActiveWidgetChanged.Broadcast(nullptr);
				}
			}
		}
	}
}

void UTimeLoopUIManagerSubsystem::RemoveWidget(TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		return;
	}

	if (TObjectPtr<UUserWidget>* FoundWidget = CachedWidgets.Find(WidgetClass))
	{
		if (FoundWidget->Get() && IsValid(FoundWidget->Get()))
		{
			FoundWidget->Get()->RemoveFromParent();
			
			if (CurrentActiveWidget == FoundWidget->Get())
			{
				CurrentActiveWidget = nullptr;
				if (OnActiveWidgetChanged.IsBound())
				{
					OnActiveWidgetChanged.Broadcast(nullptr);
				}
			}
		}
		CachedWidgets.Remove(WidgetClass);
	}
}

void UTimeLoopUIManagerSubsystem::ClearAllWidgets()
{
	for (auto& Pair : CachedWidgets)
	{
		if (Pair.Value && IsValid(Pair.Value))
		{
			Pair.Value->RemoveFromParent();
		}
	}
	CachedWidgets.Empty();
	CurrentActiveWidget = nullptr;

	if (OnActiveWidgetChanged.IsBound())
	{
		OnActiveWidgetChanged.Broadcast(nullptr);
	}
}

void UTimeLoopUIManagerSubsystem::SetInputModeUIOnly(APlayerController* PlayerController, UUserWidget* WidgetToFocus)
{
	if (!PlayerController)
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (WidgetToFocus)
	{
		InputModeData.SetWidgetToFocus(WidgetToFocus->TakeWidget());
	}
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = true;
}

void UTimeLoopUIManagerSubsystem::SetInputModeGameOnly(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = false;
}

void UTimeLoopUIManagerSubsystem::SetInputModeGameAndUI(APlayerController* PlayerController, UUserWidget* WidgetToFocus)
{
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (WidgetToFocus)
	{
		InputModeData.SetWidgetToFocus(WidgetToFocus->TakeWidget());
	}
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = true;
}

void UTimeLoopUIManagerSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	UE_LOG(LogTemp, Log, TEXT("TimeLoopUIManagerSubsystem: Level loaded: %s"), LoadedWorld ? *LoadedWorld->GetName() : TEXT("Unknown"));
	
	// If there is an active widget that was removed from viewport on world teardown, re-add it
	if (CurrentActiveWidget && IsValid(CurrentActiveWidget))
	{
		if (!CurrentActiveWidget->IsInViewport() && CurrentActiveWidget->GetVisibility() == ESlateVisibility::Visible)
		{
			CurrentActiveWidget->AddToViewport();
		}
	}
}
