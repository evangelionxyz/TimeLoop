// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ApartmentLightController.h"
#include "InputCoreTypes.h"
#include "MayaState.h"
#include "CollapseDirector.generated.h"

class ATimeLoopGameState;
class UFPSCameraEffects;
class UFlashlightEffects;
class UNiagaraSystem;
class USoundBase;

UENUM(BlueprintType)
enum class ECollapsePhase : uint8
{
	Prologue          UMETA(DisplayName = "Prologue"),
	InitialTremor     UMETA(DisplayName = "Initial Tremor"),
	StructuralRupture UMETA(DisplayName = "Structural Rupture"),
	Emergency         UMETA(DisplayName = "Emergency")
};

UENUM(BlueprintType)
enum class ECollapseEndResult : uint8
{
	Success   UMETA(DisplayName = "Success"),
	Timeout   UMETA(DisplayName = "Timeout"),
	MayaDied  UMETA(DisplayName = "Maya Died"),
	EthanDied UMETA(DisplayName = "Ethan Died"),
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollapsePhaseChanged, ECollapsePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollapseCaption, FText, Message, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCollapseEvent, FName, EventId, AActor*, EventInstigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollapseCeilingTriggered);

// One line of a conversation. Speaker is shown as "Speaker: Text" in the caption.
USTRUCT(BlueprintType)
struct FDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (MultiLine = "true"))
	FText Text;

	// Optional: re-broadcast through OnCollapseEvent when this line appears (e.g. "GiveGift" for the gift animation)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FName EventId;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCollapseDialogueLine, FText, Speaker, FText, Text, int32, Index);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCollapseResetState, ECollapseEndResult, EndResult);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCollapseDialogueFinished);

UCLASS()
class TIMELOOP_API ACollapseDirector : public AActor
{
	GENERATED_BODY()

public:
	ACollapseDirector();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// --- Flow control ---

	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void StartFlow();

	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void ResetState(const ECollapseEndResult EndResult);

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

	// Triggers the ceiling collapse sequence (drops ceiling beams / debris in Maya's room)
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void TriggerCeilingCollapse();

	// Shows a line on the HUD: calls ShowMessage on HUD_TimeLoop / its HUDController if present, and broadcasts OnCollapseMessage.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Flow")
	void ShowCaption(const FText& Message, float Duration = 4.0f);

	// --- Events ---

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapsePhaseChanged OnCollapsePhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseCaption OnCollapseCaption;

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseEvent OnCollapseEvent;

	// Broadcast when the ceiling collapse is triggered (climax / countdown end)
	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseCeilingTriggered OnCeilingCollapseTriggered;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio|Maya")
	TObjectPtr<USoundBase> MayaScreamSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio|Maya")
	TObjectPtr<USoundBase> MayaCryingSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Audio|Maya")
	TObjectPtr<USoundBase> MayaSurprisedSound;

	// --- Prologue dialogue (Maya talks to Ethan, E advances) ---

	// Trigger event id (ACollapseTrigger / HandleTriggerEvent) that starts the prologue conversation
	// Conversation played when the prologue trigger fires. Edit per-instance in the level.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Dialogue")
	TArray<FDialogueLine> PrologueDialogue;

	// Key that advances the conversation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Dialogue")
	FKey DialogueAdvanceKey = EKeys::E;

	// Appended to each caption so the player knows how to continue. Empty = none.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Dialogue")
	FText ContinuePrompt;

	// Safety timeout passed to the HUD caption for each line (the line is replaced when the player presses the key).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Dialogue", meta = (ClampMin = "1.0"))
	float DialogueLineTimeout = 60.0f;

	// Freeze movement and look while talking
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collapse|Dialogue")
	bool bFreezePlayerDuringDialogue = true;

	// Starts the conversation (Maya -> TalkToEthan). Ignored if already talking or already played.
	UFUNCTION(BlueprintCallable, Category = "Collapse|Dialogue")
	void BeginPrologueDialogue();

	// Shows the next line, or ends the conversation after the last one. Bound to DialogueAdvanceKey automatically;
	// also callable from Blueprint (e.g. from your IA_Interact handler).
	UFUNCTION(BlueprintCallable, Category = "Collapse|Dialogue")
	void AdvanceDialogue();

	UFUNCTION(BlueprintPure, Category = "Collapse|Dialogue")
	bool HasDialogFinished() const { return bDialogueFinished; }

	UFUNCTION(BlueprintPure, Category = "Collapse|Dialogue")
	bool IsDialogueActive() const { return bDialogueActive; }

	// Broadcast for every line (Index is 0-based). Use it to drive speaker names, portraits, gift animations etc.
	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseDialogueLine OnDialogueLine;

	// Broadcast when the last line was dismissed. Maya is already set to MovingToDrawing at this point.
	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseDialogueFinished OnDialogueFinished;

	UPROPERTY(BlueprintAssignable, Category = "Collapse|Events")
	FOnCollapseResetState OnResetState;

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

	void ShowDialogueLine(int32 Index);
	void EndDialogue();
	void RestorePlayerControl();

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

	FTimerHandle MayaDrawingHandle;

	bool bDialogueFinished = false;
	bool bDialogueActive = false;
	bool bDialoguePlayed = false;
	int32 DialogueIndex = INDEX_NONE;
	bool bControlFrozen = false;

	bool bShakeSaved = false;
	float SavedShake[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool bFlowStarted = false;
};
