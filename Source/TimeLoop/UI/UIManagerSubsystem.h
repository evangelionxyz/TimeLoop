// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "UIManagerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveWidgetChanged, UUserWidget*, NewActiveWidget);

// UTimeLoopUIManagerSubsystem
// Persistent GameInstance Subsystem responsible for managing UI lifecycle,
// widget persistence across levels, input mode switching, and menu transitions.
UCLASS()
class TIMELOOP_API UUIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// --- Widget Creation & Retrieval ---

	// Gets an existing widget instance of the specified class or creates one if it doesn't exist.
	// The widget is parented to the GameInstance so it survives level travel.
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI", meta = (DeterminesOutputType = "WidgetClass"))
	UUserWidget* GetOrCreateWidget(TSubclassOf<UUserWidget> WidgetClass, APlayerController* OwningPlayer = nullptr);

	// Shows a widget by adding it to the viewport (or making it visible) and setting it as active.
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI", meta = (DeterminesOutputType = "WidgetClass"))
	UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass, int32 ZOrder = 0, APlayerController* OwningPlayer = nullptr);

	// Hides a widget (sets its visibility to Collapsed).
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI")
	void HideWidget(TSubclassOf<UUserWidget> WidgetClass);

	// Removes a widget from the viewport completely.
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI")
	void RemoveWidget(TSubclassOf<UUserWidget> WidgetClass);

	// Removes all managed widgets from the viewport and clears the cache.
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI")
	void ClearAllWidgets();

	// Returns the currently active widget.
	UFUNCTION(BlueprintPure, Category = "TimeLoop|UI")
	UUserWidget* GetActiveWidget() const { return CurrentActiveWidget; }

	// --- Input Mode Helpers ---

	// Sets input mode to UI only and shows mouse cursor
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI|Input")
	void SetInputModeUIOnly(APlayerController* PlayerController, UUserWidget* WidgetToFocus = nullptr);

	// Sets input mode to Game only and hides mouse cursor
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI|Input")
	void SetInputModeGameOnly(APlayerController* PlayerController);

	// Sets input mode to Game and UI
	UFUNCTION(BlueprintCallable, Category = "TimeLoop|UI|Input")
	void SetInputModeGameAndUI(APlayerController* PlayerController, UUserWidget* WidgetToFocus = nullptr);

	// --- Delegates ---

	// Event fired whenever the active widget changes
	UPROPERTY(BlueprintAssignable, Category = "TimeLoop|UI|Events")
	FOnActiveWidgetChanged OnActiveWidgetChanged;

protected:
	// Map of cached widgets surviving level travel
	UPROPERTY(Transient)
	TMap<TSubclassOf<UUserWidget>, TObjectPtr<UUserWidget>> CachedWidgets;

	// Reference to the currently active top-level widget
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CurrentActiveWidget;

	// Delegate handle for PostLoadMap
	FDelegateHandle PostLoadMapHandle;

	// Called when a new level finishes loading
	void HandlePostLoadMap(UWorld* LoadedWorld);
};
