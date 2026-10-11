// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ApartmentLightController.h"
#include "CollapseDirector.generated.h"

class ATimeLoopGameState;
class UFPSCameraEffects;
class UFlashlightEffects;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class ECollapsePhase : uint8
{
	// Calm exploration, no loop timer, lamp in Maya's room buzzes
	Prologue UMETA(DisplayName = "Prologue"),
	// t = 0-3s: sub-audible rumble, props jitter, light shake
	InitialTremor UMETA(DisplayName = "Initial Tremor"),
	// t = 3-6s: violent shake, dust, lights flicker with sparks
	StructuralRupture UMETA(DisplayName = "Structural Rupture"),
	// After the pop: lights dead (one or more lamps stutter), player must find the flashlight
	Emergency UMETA(DisplayName = "Emergency")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollapsePhaseChanged, ECollapsePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollapseMessage, FText, Message, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollapseEvent, FName, EventId, AActor*, EventInstigator);

// Level-placed director for "The Collapse". Owns the prologue -> earthquake -> blackout flow:
//  * puts the GameState in prologue (no loop timer) and optionally auto-fires the quake after a timeout
//  * receives trigger events from ACollapseTrigger volumes
//  * drives camera shake, props, Niagara dust/sparks, sounds and the apartment lights
//  * on blackout kills every apartment light and leaves chosen lamps stuttering
//
// It works with the existing Blueprints: lights are found by class/tag and given a
// UApartmentLightController at runtime, so BP_ArpartmentLight doesn't need to change.
UCLASS()
class TIMELOOP_API ACollapseDirector : public AActor
{
	GENERATED_BODY()

public:
	ACollapseDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Flow control ---

	// Starts the flow: enters the prologue on a fresh level, or applies the post-blackout world on loop >= 1.
	// Called automatically from BeginPlay when bAutoStartPrologue is true.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void StartFlow();

	// Starts the earthquake -> blackout sequence. Ignored unless currently in the prologue.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void TriggerEarthquakeSequence();

	// A shorter standalone tremor (e.g. when the player steps into the hallway: "secondary tremor").
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void PlaySecondaryTremor();

	// Entry point for ACollapseTrigger. Maps QuakeEventId / SecondaryTremorEventId to flow actions
	// and always re-broadcasts OnCollapseEvent so Blueprints can react to custom ids.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void HandleTriggerEvent(FName EventId, AActor* EventInstigator);

	UFUNCTION(BlueprintPure, Category = "Collapse|Flow")
	ECollapsePhase GetPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Collapse|Flow")
	bool IsPrologue() const { return CurrentPhase == ECollapsePhase::Prologue; }

	// Immediately applies the post-blackout world (lights dead, stuttering lamps, items unlocked).
	// Used automatically for loops >= 1; callable if you reset the level state yourself.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void ApplyPostBlackoutState();

	// Shows a line on the HUD: calls ShowMessage on HUD_TimeLoop / its HUDController if present, and broadcasts OnCollapseMessage.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void ShowMessage(const FText& Message, float Duration = 4.0f);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapsePhaseChanged OnCollapsePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseMessage OnCollapseMessage;

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseEvent OnCollapseEvent;

	// --- Prologue ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Prologue")
	bool bAutoStartPrologue = true;

	// Seconds of unhurried exploration before the quake fires on its own. 0 disables the timeout.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Prologue", meta = (ClampMin = "0.0"))
	float PrologueTimeout = 60.0f;

	// Event id that starts the quake when sent by an ACollapseTrigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Prologue")
	FName QuakeEventId = TEXT("Quake");

	// Event id that plays the short secondary tremor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Prologue")
	FName SecondaryTremorEventId = TEXT("SecondaryTremor");

	// Actors (e.g. Pickup_Flashlight) whose collision is disabled until the blackout, so they can be seen but not taken in the prologue.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Prologue")
	TArray<TObjectPtr<AActor>> LockedUntilBlackoutActors;

	// --- Lights ---

	// Class of the apartment ceiling lights (BP_ArpartmentLight). Every actor of this class is controlled.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Lights")
	TSoftClassPtr<AActor> ApartmentLightClass;

	// Additional actors with this tag are also treated as apartment lights
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Lights")
	FName ApartmentLightTag = TEXT("ApartmentLight");

	// Lights with this tag buzz during the prologue (Maya's room: "the light keeps buzzing funny")
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Lights")
	FName BuzzLampTag = TEXT("BuzzLamp");

	// Lights with this tag keep stuttering after the blackout instead of staying dead
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Lights")
	FName StutterLampTag = TEXT("StutterLamp");

	// If no light carries StutterLampTag, this many lights closest to the player become the stuttering lamps
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Lights", meta = (ClampMin = "0"))
	int32 FallbackStutterLampCount = 1;

	// Seconds of total darkness after the pop before the stuttering lamps come alive
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Lights", meta = (ClampMin = "0.0"))
	float StutterStartDelay = 1.2f;

	// --- Timing ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Timing", meta = (ClampMin = "0.1"))
	float InitialTremorDuration = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Timing", meta = (ClampMin = "0.1"))
	float RuptureDuration = 3.0f;

	// Seconds after the blackout before the "grab the flashlight" monologue
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Timing", meta = (ClampMin = "0.0"))
	float EmergencyDelay = 2.5f;

	// How long the camera keeps shaking after the blackout before settling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Timing", meta = (ClampMin = "0.0"))
	float AftershockDuration = 2.0f;

	// --- Camera ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Camera")
	float InitialTremorIntensity = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Camera")
	float RuptureIntensity = 1.0f;

	// SetShakingParameters during the rupture (Frequency, Pitch, Roll, Vertical)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Camera")
	FVector4 RuptureShakeParameters = FVector4(8.0f, 2.5f, 3.0f, 6.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Camera")
	float SecondaryTremorDuration = 3.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Camera")
	float SecondaryTremorIntensity = 2.0f;

	// --- Props ---

	// Actors with this tag and simulated physics jitter during the quake
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Props")
	FName QuakePropTag = TEXT("QuakeProp");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Props")
	float PropImpulse = 60.0f;

	// --- Niagara ---

	// Plaster dust falling from the ceiling (NS_DustMote)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Effects")
	TObjectPtr<UNiagaraSystem> DustSystem;

	// Electrical sparks (used by the lights and the ceiling arcs)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Effects")
	TObjectPtr<UNiagaraSystem> SparkSystem;

	// Extra spots where dust falls. Actors with this tag are used in addition to the area above the player.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Effects")
	FName DustPointTag = TEXT("QuakeDust");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Effects")
	float DustScatterRadius = 250.0f;

	// --- Audio (all optional) ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio")
	TObjectPtr<USoundBase> RumbleSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio")
	TObjectPtr<USoundBase> StructuralCrackSound;

	// Played from each light as it sparks
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio")
	TObjectPtr<USoundBase> SparkSound;

	// Violent electrical pop at the blackout
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio")
	TObjectPtr<USoundBase> BlackoutPopSound;

	// "ETHAN?! THE LIGHTS WENT OUT!" through the intercom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio")
	TObjectPtr<USoundBase> MayaScreamSound;

	// --- Subtitles / monologue ---

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Text")
	FText QuakeStartLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Text")
	FText MayaScreamLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Text")
	FText EmergencyLine;

private:
	void EnterPhase(ECollapsePhase NewPhase);
	void EnterInitialTremor();
	void EnterRupture();
	void EnterBlackout();
	void EnterEmergency();

	void RuptureEffectsTick();
	void PropJitterTick();
	void StartStutterLamps();
	void RestoreShaking();

	void CollectLights();
	UApartmentLightController* EnsureController(AActor* LightActor);
	void SetAllLights(EApartmentLightMode Mode);
	void SetUnlockedState(bool bUnlocked);
	void SpawnDustBurst();

	ATimeLoopGameState* GetTimeLoopGameState() const;
	UFPSCameraEffects* GetPlayerCameraEffects() const;
	UFlashlightEffects* GetPlayerFlashlight() const;

	ECollapsePhase CurrentPhase = ECollapsePhase::Prologue;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> LightActors;

	UPROPERTY(Transient)
	TArray<TObjectPtr<AActor>> PropActors;

	FTimerHandle PrologueTimeoutHandle;
	FTimerHandle PhaseHandle;
	FTimerHandle EffectsHandle;
	FTimerHandle PropHandle;
	FTimerHandle StutterHandle;
	FTimerHandle AftershockHandle;

	bool bShakeSaved = false;
	float SavedShake[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool bFlowStarted = false;
};
