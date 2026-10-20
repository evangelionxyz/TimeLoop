// Copyright (c) 2026 Team Ficus

#include "CollapseDirector.h"
#include "ApartmentLightController.h"
#include "FPSCameraEffects.h"
#include "FlashlightEffects.h"
#include "TimeLoopGameState.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "Components/InputComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogCollapse, Log, All);

namespace
{
	bool CallBlueprintShowCaption(UObject* Target, const FText& Message, float Duration)
	{
		UFunction* Func = Target ? Target->FindFunction(TEXT("ShowCaption")) : nullptr;
		if (!Func || Func->ParmsSize <= 0)
		{
			return false;
		}

		uint8* Params = static_cast<uint8*>(FMemory_Alloca(Func->ParmsSize));
		FMemory::Memzero(Params, Func->ParmsSize);

		for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->InitializeValue_InContainer(Params);
		}

		bool bTextSet = false;
		for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			FProperty* Prop = *It;
			if (Prop->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				continue;
			}

			if (const FTextProperty* TextProp = CastField<FTextProperty>(Prop))
			{
				TextProp->SetPropertyValue_InContainer(Params, Message);
				bTextSet = true;
			}
			else if (const FStrProperty* StrProp = CastField<FStrProperty>(Prop))
			{
				StrProp->SetPropertyValue_InContainer(Params, Message.ToString());
				bTextSet = true;
			}
			else if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
			{
				FloatProp->SetPropertyValue_InContainer(Params, Duration);
			}
			else if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
			{
				DoubleProp->SetPropertyValue_InContainer(Params, Duration);
			}
		}

		if (bTextSet)
		{
			Target->ProcessEvent(Func, Params);
		}

		for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
		{
			It->DestroyValue_InContainer(Params);
		}

		return bTextSet;
	}
}

ACollapseDirector::ACollapseDirector()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ApartmentLightClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/Core/Lighting/BP_ArpartmentLight.BP_ArpartmentLight_C")));

	QuakeStartLine = NSLOCTEXT("Collapse", "QuakeStart", "What the hell... an earthquake?");
	MayaScreamLine = NSLOCTEXT("Collapse", "MayaScream", "ETHAN?! THE LIGHTS WENT OUT! SOMETHING CRASHED OUTSIDE! ETHAN, HELP ME!");
	EmergencyLine = NSLOCTEXT("Collapse", "Emergency", "The power's out! I need to grab the flashlight and get down to Maya!");

	MayaClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/Core/Characters/BP_Maya.BP_Maya_C")));

	ContinuePrompt = NSLOCTEXT("Collapse", "ContinuePrompt", "[E] Continue");

	auto MakeLine = [](const FText& Speaker, const FText& Text, FName EventId = NAME_None)
	{
		FCollapseDialogueLine Line;
		Line.Speaker = Speaker;
		Line.Text = Text;
		Line.EventId = EventId;
		return Line;
	};
	const FText Maya = NSLOCTEXT("Collapse", "SpeakerMaya", "Maya");
	const FText Ethan = NSLOCTEXT("Collapse", "SpeakerEthan", "Ethan");
	PrologueDialogue = {
		MakeLine(Maya, NSLOCTEXT("Collapse", "P1", "Ethan! You came!")),
		MakeLine(Ethan, NSLOCTEXT("Collapse", "P2", "Happy birthday, Maya. I got you something."), TEXT("GiveGift")),
		MakeLine(Maya, NSLOCTEXT("Collapse", "P3", "You didn't have to... Thank you, I love it!")),
		MakeLine(Ethan, NSLOCTEXT("Collapse", "P4", "Anything for my little sister. Go on, don't let me keep you from your drawing.")),
		MakeLine(Maya, NSLOCTEXT("Collapse", "P5", "Okay! I'll finish it before dinner. Bye, Ethan!")),
	};
}

void ACollapseDirector::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoStartPrologue)
	{
		// Next tick so the GameState has finished syncing the loop count from the GameInstance.
		GetWorldTimerManager().SetTimerForNextTick(this, &ACollapseDirector::StartFlow);
	}
}

void ACollapseDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		FTimerManager& TimerManager = World->GetTimerManager();
		TimerManager.ClearTimer(PrologueTimeoutHandle);
		TimerManager.ClearTimer(PhaseHandle);
		TimerManager.ClearTimer(EffectsHandle);
		TimerManager.ClearTimer(PropHandle);
		TimerManager.ClearTimer(StutterHandle);
		TimerManager.ClearTimer(AftershockHandle);
		TimerManager.ClearTimer(MayaDrawingHandle);

		RestorePlayerControl();

		// Don't leave the quake's shake parameters on the player if the director goes away mid-sequence.
		RestoreShaking();
	}

	Super::EndPlay(EndPlayReason);
}

// ---------------------------------------------------------------------------------------------
// Flow
// ---------------------------------------------------------------------------------------------

void ACollapseDirector::StartFlow()
{
	if (bFlowStarted)
		return;

	bFlowStarted = true;

	CollectLights();
	ATimeLoopGameState* GameState = GetTimeLoopGameState();
	if (GameState)
	{
		GameState->StartPrologue();
		GameState->OnMayaStateChanged.AddUniqueDynamic(this, &ACollapseDirector::HandleMayaStateChanged);
	}

	// No GameState means we can't know the loop count: treat it as a fresh run.
	const bool bPrologue = !GameState || GameState->IsInPrologue();
	if (!bPrologue)
	{
		ApplyPostBlackoutState();
		return;
	}

	CurrentPhase = ECollapsePhase::Prologue;
	SetUnlockedState(false);

	for (AActor* LightActor : LightActors)
	{
		if (LightActor && !BuzzLampTag.IsNone() && LightActor->ActorHasTag(BuzzLampTag))
		{
			EnsureController(LightActor)->SetMode(EApartmentLightMode::Buzz);
		}
	}

	if (PrologueTimeout > 0.0f)
	{
		GetWorldTimerManager().SetTimer(PrologueTimeoutHandle, this, &ACollapseDirector::TriggerEarthquakeSequence, PrologueTimeout, false);
	}

	OnCollapsePhaseChanged.Broadcast(ECollapsePhase::Prologue);
}

void ACollapseDirector::TriggerEarthquakeSequence()
{
	if (CurrentPhase != ECollapsePhase::Prologue)
	{
		return;
	}

	if (!bFlowStarted)
	{
		StartFlow();
	}

	GetWorldTimerManager().ClearTimer(PrologueTimeoutHandle);
	EndDialogue();
	EnterInitialTremor();
}

void ACollapseDirector::HandleTriggerEvent(FName EventId, AActor* EventInstigator)
{
	if (EventId == QuakeEventId)
	{
		TriggerEarthquakeSequence();
	}
	else if (EventId == SecondaryTremorEventId)
	{
		PlaySecondaryTremor();
	}
	else if (EventId == PrologueDialogueEventId)
	{
		BeginPrologueDialogue();
	}

	OnCollapseEvent.Broadcast(EventId, EventInstigator);
}

void ACollapseDirector::EnterPhase(ECollapsePhase NewPhase)
{
	CurrentPhase = NewPhase;
	OnCollapsePhaseChanged.Broadcast(NewPhase);
}

void ACollapseDirector::EnterInitialTremor()
{
	EnterPhase(ECollapsePhase::InitialTremor);

	PropActors.Reset();
	if (!QuakePropTag.IsNone())
	{
		TArray<AActor*> Found;
		UGameplayStatics::GetAllActorsWithTag(this, QuakePropTag, Found);
		PropActors.Append(Found);
	}

	if (UFPSCameraEffects* Camera = GetPlayerCameraEffects())
	{
		Camera->PlayEarthQuakeShake(InitialTremorDuration, InitialTremorIntensity);
	}

	if (RumbleSound)
	{
		UGameplayStatics::PlaySound2D(this, RumbleSound);
	}

	ShowCaption(QuakeStartLine, InitialTremorDuration);

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(PropHandle, this, &ACollapseDirector::PropJitterTick, 0.2f, true);
	TimerManager.SetTimer(PhaseHandle, this, &ACollapseDirector::EnterRupture, InitialTremorDuration, false);
}

void ACollapseDirector::EnterRupture()
{
	EnterPhase(ECollapsePhase::StructuralRupture);

	if (UFPSCameraEffects* Camera = GetPlayerCameraEffects())
	{
		if (!bShakeSaved)
		{
			Camera->GetShakingParameters(SavedShake[0], SavedShake[1], SavedShake[2], SavedShake[3]);
			bShakeSaved = true;
		}
		Camera->SetShakingParameters(RuptureShakeParameters.X, RuptureShakeParameters.Y, RuptureShakeParameters.Z, RuptureShakeParameters.W);
		Camera->PlayEarthQuakeShake(RuptureDuration + AftershockDuration, RuptureIntensity);
	}

	if (UFlashlightEffects* Flashlight = GetPlayerFlashlight())
	{
		Flashlight->FlickerFlashlight(RuptureDuration);
	}

	if (StructuralCrackSound)
	{
		UGameplayStatics::PlaySound2D(this, StructuralCrackSound);
	}

	SetAllLights(EApartmentLightMode::Flicker);
	SpawnDustBurst();

	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.SetTimer(EffectsHandle, this, &ACollapseDirector::RuptureEffectsTick, 0.3f, true);
	TimerManager.SetTimer(PhaseHandle, this, &ACollapseDirector::EnterBlackout, RuptureDuration, false);
}

void ACollapseDirector::EnterBlackout()
{
	FTimerManager& TimerManager = GetWorldTimerManager();
	TimerManager.ClearTimer(EffectsHandle);
	TimerManager.ClearTimer(PropHandle);

	// Violent pop: every light dies at the same instant, each with a spark burst.
	for (AActor* LightActor : LightActors)
	{
		if (LightActor)
		{
			EnsureController(LightActor)->PopOff();
		}
	}

	if (BlackoutPopSound)
	{
		UGameplayStatics::PlaySound2D(this, BlackoutPopSound);
	}

	if (MayaScreamSound)
	{
		UGameplayStatics::PlaySound2D(this, MayaScreamSound);
	}
	ShowCaption(MayaScreamLine, 4.0f);

	SetUnlockedState(true);

	TimerManager.SetTimer(AftershockHandle, this, &ACollapseDirector::RestoreShaking, FMath::Max(AftershockDuration, 0.01f), false);
	TimerManager.SetTimer(StutterHandle, this, &ACollapseDirector::StartStutterLamps, FMath::Max(StutterStartDelay, 0.01f), false);
	TimerManager.SetTimer(PhaseHandle, this, &ACollapseDirector::EnterEmergency, FMath::Max(EmergencyDelay, 0.01f), false);
}

void ACollapseDirector::EnterEmergency()
{
	EnterPhase(ECollapsePhase::Emergency);
	ShowCaption(EmergencyLine, 5.0f);
}

void ACollapseDirector::ApplyPostBlackoutState()
{
	if (!bFlowStarted)
	{
		bFlowStarted = true;
		CollectLights();
	}

	GetWorldTimerManager().ClearTimer(PrologueTimeoutHandle);

	SetAllLights(EApartmentLightMode::Off);
	SetUnlockedState(true);
	StartStutterLamps();

	EnterPhase(ECollapsePhase::Emergency);
}

void ACollapseDirector::PlaySecondaryTremor()
{
	if (UFPSCameraEffects* Camera = GetPlayerCameraEffects())
	{
		Camera->PlayEarthQuakeShake(SecondaryTremorDuration, SecondaryTremorIntensity);
	}

	if (RumbleSound)
	{
		UGameplayStatics::PlaySound2D(this, RumbleSound);
	}

	if (StructuralCrackSound)
	{
		UGameplayStatics::PlaySound2D(this, StructuralCrackSound);
	}

	SpawnDustBurst();
}

// ---------------------------------------------------------------------------------------------
// Effects
// ---------------------------------------------------------------------------------------------

void ACollapseDirector::RuptureEffectsTick()
{
	if (LightActors.Num() > 0 && FMath::FRand() < 0.6f)
	{
		if (AActor* LightActor = LightActors[FMath::RandRange(0, LightActors.Num() - 1)])
		{
			EnsureController(LightActor)->EmitSparks();
		}
	}

	if (FMath::FRand() < 0.3f)
	{
		SpawnDustBurst();
	}

	PropJitterTick();
}

void ACollapseDirector::PropJitterTick()
{
	const float Strength = CurrentPhase == ECollapsePhase::StructuralRupture ? PropImpulse * 2.0f : PropImpulse;

	for (AActor* Prop : PropActors)
	{
		UPrimitiveComponent* Primitive = Prop ? Cast<UPrimitiveComponent>(Prop->GetRootComponent()) : nullptr;
		if (Primitive && Primitive->IsSimulatingPhysics())
		{
			const FVector Direction(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(0.0f, 0.4f));
			Primitive->AddImpulse(Direction * Strength, NAME_None, true);
		}
	}
}

void ACollapseDirector::SpawnDustBurst()
{
	if (!DustSystem)
	{
		return;
	}

	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const FVector Start = Player->GetActorLocation();
		FVector CeilingPoint = Start + FVector(0.0f, 0.0f, 250.0f);

		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(CollapseDust), false, Player);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Start + FVector(0.0f, 0.0f, 60.0f), Start + FVector(0.0f, 0.0f, 600.0f), ECC_Visibility, Params))
		{
			CeilingPoint = Hit.ImpactPoint - FVector(0.0f, 0.0f, 5.0f);
		}

		const FVector2D Offset = FMath::RandPointInCircle(DustScatterRadius);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DustSystem, CeilingPoint + FVector(Offset.X, Offset.Y, 0.0f));
	}

	if (!DustPointTag.IsNone())
	{
		TArray<AActor*> Points;
		UGameplayStatics::GetAllActorsWithTag(this, DustPointTag, Points);
		for (const AActor* Point : Points)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DustSystem, Point->GetActorLocation());
		}
	}
}

void ACollapseDirector::RestoreShaking()
{
	if (!bShakeSaved)
	{
		return;
	}
	bShakeSaved = false;

	if (UFPSCameraEffects* Camera = GetPlayerCameraEffects())
	{
		Camera->SetShakingParameters(SavedShake[0], SavedShake[1], SavedShake[2], SavedShake[3]);
	}
}

// ---------------------------------------------------------------------------------------------
// Lights
// ---------------------------------------------------------------------------------------------

void ACollapseDirector::CollectLights()
{
	LightActors.Reset();

	if (!ApartmentLightClass.IsNull())
	{
		if (UClass* LightClass = ApartmentLightClass.LoadSynchronous())
		{
			TArray<AActor*> Found;
			UGameplayStatics::GetAllActorsOfClass(this, LightClass, Found);
			for (AActor* Actor : Found)
			{
				LightActors.AddUnique(Actor);
			}
		}
		else
		{
			UE_LOG(LogCollapse, Warning, TEXT("ApartmentLightClass '%s' could not be loaded."), *ApartmentLightClass.ToString());
		}
	}

	for (const FName& Tag : { ApartmentLightTag, BuzzLampTag, StutterLampTag })
	{
		if (Tag.IsNone())
		{
			continue;
		}

		TArray<AActor*> Tagged;
		UGameplayStatics::GetAllActorsWithTag(this, Tag, Tagged);
		for (AActor* Actor : Tagged)
		{
			LightActors.AddUnique(Actor);
		}
	}

	for (AActor* Actor : LightActors)
	{
		EnsureController(Actor);
	}

	UE_LOG(LogCollapse, Log, TEXT("Collapse director controls %d apartment light actors."), LightActors.Num());
}

UApartmentLightController* ACollapseDirector::EnsureController(AActor* LightActor)
{
	check(LightActor);

	UApartmentLightController* Controller = LightActor->FindComponentByClass<UApartmentLightController>();
	if (!Controller)
	{
		Controller = NewObject<UApartmentLightController>(LightActor, TEXT("ApartmentLightController"));
		LightActor->AddInstanceComponent(Controller);
		Controller->RegisterComponent();
	}

	if (!Controller->SparkSystem)
	{
		Controller->SparkSystem = SparkSystem;
	}
	if (!Controller->SparkSound)
	{
		Controller->SparkSound = SparkSound;
	}

	return Controller;
}

void ACollapseDirector::SetAllLights(EApartmentLightMode Mode)
{
	for (AActor* LightActor : LightActors)
	{
		if (LightActor)
		{
			EnsureController(LightActor)->SetMode(Mode);
		}
	}
}

void ACollapseDirector::StartStutterLamps()
{
	TArray<AActor*> Chosen;

	if (!StutterLampTag.IsNone())
	{
		for (AActor* LightActor : LightActors)
		{
			if (LightActor && LightActor->ActorHasTag(StutterLampTag))
			{
				Chosen.Add(LightActor);
			}
		}
	}

	if (Chosen.IsEmpty() && FallbackStutterLampCount > 0)
	{
		TArray<AActor*> Sorted;
		for (AActor* LightActor : LightActors)
		{
			if (LightActor)
			{
				Sorted.Add(LightActor);
			}
		}

		const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
		const FVector Origin = Player ? Player->GetActorLocation() : GetActorLocation();
		Sorted.Sort([&Origin](const AActor& A, const AActor& B)
		{
			return FVector::DistSquared(A.GetActorLocation(), Origin) < FVector::DistSquared(B.GetActorLocation(), Origin);
		});

		for (int32 Index = 0; Index < FMath::Min(FallbackStutterLampCount, Sorted.Num()); ++Index)
		{
			Chosen.Add(Sorted[Index]);
		}
	}

	for (AActor* LightActor : Chosen)
	{
		EnsureController(LightActor)->SetMode(EApartmentLightMode::Stutter);
	}
}

// ---------------------------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------------------------

void ACollapseDirector::SetUnlockedState(bool bUnlocked)
{
	for (AActor* Actor : LockedUntilBlackoutActors)
	{
		if (Actor)
		{
			Actor->SetActorEnableCollision(bUnlocked);
		}
	}
}

void ACollapseDirector::ShowCaption(const FText& Message, float Duration)
{
	if (Message.IsEmpty())
	{
		return;
	}

	if (const APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (AHUD* HUD = PC->GetHUD())
		{
			bool bShown = CallBlueprintShowCaption(HUD, Message, Duration);

			if (!bShown)
			{
				if (const FObjectProperty* ControllerProp = FindFProperty<FObjectProperty>(HUD->GetClass(), TEXT("HUDController")))
				{
					bShown = CallBlueprintShowCaption(ControllerProp->GetObjectPropertyValue_InContainer(HUD), Message, Duration);
				}
			}

			if (!bShown)
			{
				UE_LOG(LogCollapse, Verbose, TEXT("No ShowCaption function found on the HUD; relying on OnCollapseCaption."));
			}
		}
	}

	OnCollapseCaption.Broadcast(Message, Duration);
}

// ---------------------------------------------------------------------------------------------
// Prologue dialogue
// ---------------------------------------------------------------------------------------------

void ACollapseDirector::BeginPrologueDialogue()
{
	if (bDialogueActive || bDialoguePlayed || CurrentPhase != ECollapsePhase::Prologue || PrologueDialogue.IsEmpty())
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC)
	{
		return;
	}

	bDialogueActive = true;
	bDialoguePlayed = true;

	const bool bHadInput = InputComponent != nullptr;
	EnableInput(PC);
	if (InputComponent && !bHadInput)
	{
		InputComponent->BindKey(DialogueAdvanceKey, IE_Pressed, this, &ACollapseDirector::AdvanceDialogue);
	}

	if (bFreezePlayerDuringDialogue)
	{
		PC->SetIgnoreMoveInput(true);
		PC->SetIgnoreLookInput(true);
		bControlFrozen = true;
	}

	if (ATimeLoopGameState* GameState = GetTimeLoopGameState())
	{
		GameState->SetInDialogue(true);
		GameState->SetMayaState(EMayaState::TalkToEthan);
	}

	ShowDialogueLine(0);
}

void ACollapseDirector::AdvanceDialogue()
{
	if (!bDialogueActive)
	{
		return;
	}

	if (DialogueIndex + 1 < PrologueDialogue.Num())
	{
		ShowDialogueLine(DialogueIndex + 1);
	}
	else
	{
		EndDialogue();
	}
}

void ACollapseDirector::ShowDialogueLine(int32 Index)
{
	if (!PrologueDialogue.IsValidIndex(Index))
	{
		return;
	}

	DialogueIndex = Index;
	const FCollapseDialogueLine& Line = PrologueDialogue[Index];

	FText Caption = Line.Text;
	if (!Line.Speaker.IsEmpty())
	{
		Caption = FText::Format(NSLOCTEXT("Collapse", "DialogueFormat", "{0}: {1}"), Line.Speaker, Line.Text);
	}
	if (!ContinuePrompt.IsEmpty())
	{
		Caption = FText::Format(NSLOCTEXT("Collapse", "DialoguePromptFormat", "{0}\n{1}"), Caption, ContinuePrompt);
	}

	ShowCaption(Caption, DialogueLineTimeout);
	OnDialogueLine.Broadcast(Line.Speaker, Line.Text, Index);

	if (!Line.EventId.IsNone())
	{
		OnCollapseEvent.Broadcast(Line.EventId, UGameplayStatics::GetPlayerPawn(this, 0));
	}
}

void ACollapseDirector::EndDialogue()
{
	if (!bDialogueActive)
	{
		return;
	}

	bDialogueActive = false;
	RestorePlayerControl();

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		DisableInput(PC);
	}

	// Replaces the open caption with a copy that expires almost immediately (ShowCaption has no "hide").
	if (PrologueDialogue.IsValidIndex(DialogueIndex))
	{
		ShowCaption(PrologueDialogue[DialogueIndex].Text, 0.05f);
	}
	DialogueIndex = INDEX_NONE;

	if (ATimeLoopGameState* GameState = GetTimeLoopGameState())
	{
		GameState->SetInDialogue(false);
	}

	OnDialogueFinished.Broadcast();

	// Preferred: let BP_Maya run her own state machine (walk to desk, then Drawing). Its SetState mirrors into the GameState.
	if (SetMayaBlueprintState(EMayaState::MovingToDrawing))
	{
		return;
	}

	// Fallback when no Maya Blueprint is in the level: just drive the GameState state on a timer.
	if (ATimeLoopGameState* GameState = GetTimeLoopGameState())
	{
		GameState->SetMayaState(EMayaState::MovingToDrawing);
	}
	GetWorldTimerManager().SetTimer(MayaDrawingHandle, this, &ACollapseDirector::FinishMayaMoveToDrawing, FMath::Max(MayaMoveToDrawingDuration, 0.01f), false);
}

void ACollapseDirector::HandleMayaStateChanged(EMayaState NewState, EMayaState OldState)
{
	// BP_Maya reaches TalkToEthan once she has walked up to him: that is when the conversation starts.
	if (NewState == EMayaState::TalkToEthan)
	{
		BeginPrologueDialogue();
	}
}

bool ACollapseDirector::SetMayaBlueprintState(EMayaState NewState)
{
	UClass* Class = MayaClass.LoadSynchronous();
	if (!Class || !GetWorld())
	{
		return false;
	}

	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(this, Class, Found);
	UFunction* Func = Class->FindFunctionByName(TEXT("SetState"));
	if (Found.IsEmpty() || !Func || Func->ParmsSize <= 0)
	{
		return false;
	}

	uint8* Params = static_cast<uint8*>(FMemory_Alloca(Func->ParmsSize));
	FMemory::Memzero(Params, Func->ParmsSize);
	bool bSet = false;
	for (TFieldIterator<FProperty> It(Func); It && It->HasAnyPropertyFlags(CPF_Parm); ++It)
	{
		if (const FByteProperty* ByteProp = CastField<FByteProperty>(*It))
		{
			ByteProp->SetPropertyValue_InContainer(Params, static_cast<uint8>(NewState));
			bSet = true;
			break;
		}
		if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(*It))
		{
			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(EnumProp->ContainerPtrToValuePtr<void>(Params), static_cast<int64>(NewState));
			bSet = true;
			break;
		}
	}

	if (bSet)
	{
		Found[0]->ProcessEvent(Func, Params);
	}
	return bSet;
}

void ACollapseDirector::FinishMayaMoveToDrawing()
{
	if (ATimeLoopGameState* GameState = GetTimeLoopGameState())
	{
		// Don't overwrite the quake reaction if the earthquake started while she was walking.
		if (GameState->GetMayaState() == EMayaState::MovingToDrawing)
		{
			GameState->SetMayaState(EMayaState::Drawing);
		}
	}
}

void ACollapseDirector::RestorePlayerControl()
{
	if (!bControlFrozen)
	{
		return;
	}

	bControlFrozen = false;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		PC->SetIgnoreMoveInput(false);
		PC->SetIgnoreLookInput(false);
	}
}

ATimeLoopGameState* ACollapseDirector::GetTimeLoopGameState() const{
	return GetWorld() ? GetWorld()->GetGameState<ATimeLoopGameState>() : nullptr;
}

UFPSCameraEffects* ACollapseDirector::GetPlayerCameraEffects() const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	return Player ? Player->FindComponentByClass<UFPSCameraEffects>() : nullptr;
}

UFlashlightEffects* ACollapseDirector::GetPlayerFlashlight() const
{
	const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	return Player ? Player->FindComponentByClass<UFlashlightEffects>() : nullptr;
}
