// Copyright (c) 2026 Team Ficus

#include "MayaCharacter.h"
#include "AIController.h"
#include "TimeLoopGameState.h"
#include "CollapseDirector.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/OutputDeviceNull.h"

AMayaCharacter::AMayaCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AMayaCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 1. Listen to the Collapse Director
    if (AActor *DirectorActor = UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass()))
    {
        if (ACollapseDirector *Director = Cast<ACollapseDirector>(DirectorActor))
        {
            Director->OnCollapsePhaseChanged.AddDynamic(this, &AMayaCharacter::HandleCollapsePhaseChanged);
        }
    }

    // 2. Check Loop Count / Prologue via GameState
    ATimeLoopGameState *GameState = GetWorld()->GetGameState<ATimeLoopGameState>();
    const bool bInPrologue = GameState && GameState->IsInPrologue() && GameState->GetLoopCount() == 0;

    if (bInPrologue)
    {
        // Calm run: begin walking to the door
        SetMayaState(EMayaState::MovingToDoor);
    }
    else
    {
        // Emergency run (Loop >= 1): skip prologue, spawn directly huddled on the floor
        if (PlayMatActor)
        {
            SetActorLocation(PlayMatActor->GetActorLocation());
        }
        SetMayaState(EMayaState::Panicked);
    }
}

bool AMayaCharacter::CanTransitionTo(EMayaState NewState) const
{
    if (CurrentState == NewState) return false;
    if (CurrentState == EMayaState::Saved || CurrentState == EMayaState::Crushed) return false;

    // Disasters (Tremor / Blackout) can interrupt any prologue state immediately
    if (NewState == EMayaState::Startled || NewState == EMayaState::Panicked) return true;

    switch (NewState)
    {
    case EMayaState::MovingToDoor:
        return CurrentState == EMayaState::Inactive;
    case EMayaState::InteractingWithDoor:
        return CurrentState == EMayaState::MovingToDoor;
    case EMayaState::MovingToDrawing:
        return CurrentState == EMayaState::InteractingWithDoor;
    case EMayaState::Drawing:
        return CurrentState == EMayaState::MovingToDrawing;
    case EMayaState::SeekingShelter:
        return CurrentState == EMayaState::Panicked;
    case EMayaState::Sheltered:
        return CurrentState == EMayaState::SeekingShelter;
    default:
        return true;
    }
}

void AMayaCharacter::SetMayaState(EMayaState NewState)
{
    if (!CanTransitionTo(NewState)) return;

    EMayaState OldState = CurrentState;
    OnStateExited(OldState, NewState);
    CurrentState = NewState;
    OnStateEntered(NewState, OldState);

    OnMayaStateChanged.Broadcast(NewState, OldState);
}

void AMayaCharacter::OnStateEntered(EMayaState NewState, EMayaState OldState)
{
    switch (NewState)
    {
    case EMayaState::MovingToDoor:
        MoveToTargetActor(TargetPoint_A);
        break;

    case EMayaState::InteractingWithDoor:
        if (DoorActor)
        {
            InteractWithDoor(DoorActor);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("AMayaCharacter: DoorActor is NULL! Please assign DoorActor in the Details panel."));
        }

        // Wait 1.0s then proceed to drawing area (matches BP_Maya Delay 1.0)
        GetWorldTimerManager().SetTimer(StateTimerHandle, [this]()
            {
                if (DoorActor)
                {
                    if (UFunction* EnableColFunc = DoorActor->FindFunction(TEXT("EnableCollision")))
                    {
                        DoorActor->ProcessEvent(EnableColFunc, nullptr);
                    }
                    else
                    {
                        FOutputDeviceNull Ar;
                        DoorActor->CallFunctionByNameWithArguments(TEXT("EnableCollision"), Ar, nullptr, true);
                    }
                }
                SetMayaState(EMayaState::MovingToDrawing);
            }, 1.0f, false);
        break;

    case EMayaState::MovingToDrawing:
        MoveToTargetActor(TargetPoint_B);
        break;

    case EMayaState::Drawing:
        if (CrayonActor && GetMesh())
        {
            CrayonActor->SetActorEnableCollision(true);
            CrayonActor->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, HandSocketName);
        }
        break;

    case EMayaState::Startled:
        // Stop movement, drop crayon, freeze
        if (AAIController *AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
        if (CrayonActor)
        {
            CrayonActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        }
        break;

    case EMayaState::Panicked:
        if (AAIController *AI = Cast<AAIController>(GetController()))
        {
            AI->StopMovement();
        }
        // In ABP_Maya, this triggers ANIM_MayaSitHide (curled on play mat crying)
        break;

    case EMayaState::SeekingShelter:
        MoveToTargetActor(CurrentMoveGoal, 30.0f);
        break;

    case EMayaState::Sheltered:
        if (ATimeLoopGameState *GameState = GetWorld()->GetGameState<ATimeLoopGameState>())
        {
            GameState->SetObjectiveSolved(true);
        }
        OnMayaSheltered.Broadcast();
        break;

    default:
        break;
    }
}

void AMayaCharacter::OnStateExited(EMayaState OldState, EMayaState NewState)
{
    GetWorldTimerManager().ClearTimer(StateTimerHandle);
}

void AMayaCharacter::HandleCollapsePhaseChanged(ECollapsePhase NewPhase)
{
    if (NewPhase == ECollapsePhase::InitialTremor)
    {
        SetMayaState(EMayaState::Startled);
    }
    else if (NewPhase == ECollapsePhase::StructuralRupture || NewPhase == ECollapsePhase::Emergency)
    {
        SetMayaState(EMayaState::Panicked);
    }
}

void AMayaCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AAIController* AI = Cast<AAIController>(GetController());
    const bool bAIStopped = AI && !AI->IsFollowingAPath();

    if (CurrentState == EMayaState::MovingToDoor && TargetPoint_A)
    {
        const float Dist = FVector::Dist2D(GetActorLocation(), TargetPoint_A->GetActorLocation());
        if (Dist <= ArrivalDistance || (bAIStopped && Dist <= ArrivalDistance * 1.25f))
        {
            SetMayaState(EMayaState::InteractingWithDoor);
        }
    }
    else if (CurrentState == EMayaState::MovingToDrawing && TargetPoint_B)
    {
        const float Dist = FVector::Dist2D(GetActorLocation(), TargetPoint_B->GetActorLocation());
        if (Dist <= ArrivalDistance || (bAIStopped && Dist <= ArrivalDistance * 1.25f))
        {
            SetMayaState(EMayaState::Drawing);
        }
    }
    else if (CurrentState == EMayaState::SeekingShelter && CurrentMoveGoal)
    {
        const float Dist = FVector::Dist2D(GetActorLocation(), CurrentMoveGoal->GetActorLocation());
        if (Dist <= ArrivalDistance || (bAIStopped && Dist <= ArrivalDistance * 1.25f))
        {
            SetMayaState(EMayaState::Sheltered);
        }
    }
}

void AMayaCharacter::MoveToTargetActor(AActor *Target, float AcceptanceRadius)
{
    if (!Target)
    {
        return;
    }

    if (AAIController *AI = Cast<AAIController>(GetController()))
    {
        AI->MoveToActor(Target, AcceptanceRadius);
    }
}

void AMayaCharacter::HandleMoveCompleted()
{
}

void AMayaCharacter::OrderGoToShelter(AActor *ShelterDestination)
{
    CurrentMoveGoal = ShelterDestination;
    SetMayaState(EMayaState::SeekingShelter);
}

void AMayaCharacter::InteractWithDoor_Implementation(AActor *Door)
{
    if (!Door)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("AMayaCharacter::InteractWithDoor executing on %s"), *Door->GetName());

    // 1. Disable collision on door (matching BP_Maya: Class|BPInteractDoor|DisableCollision)
    if (UFunction *DisableColFunc = Door->FindFunction(TEXT("DisableCollision")))
    {
        Door->ProcessEvent(DisableColFunc, nullptr);
    }
    else
    {
        FOutputDeviceNull Ar;
        Door->CallFunctionByNameWithArguments(TEXT("DisableCollision"), Ar, nullptr, true);
    }

    // 2. Call BPI_Interactable::Interact(Door, Self)
    // Matches BP_Maya: (Class|BPIInteractable|Interact _door _self)
    bool bSuccess = false;
    static const FName CandidateNames[] = {
        TEXT("Interact"),
        TEXT("Execute_Interact"),
        TEXT("BPIInteractable_Interact"),
        TEXT("Open")
    };

    for (const FName &FuncName : CandidateNames)
    {
        if (UFunction *Func = Door->FindFunction(FuncName))
        {
            if (Func->ParmsSize > 0)
            {
                uint8 *Buffer = (uint8 *)FMemory_Alloca(Func->ParmsSize);
                FMemory::Memzero(Buffer, Func->ParmsSize);

                for (TFieldIterator<FProperty> It(Func); It && (It->PropertyFlags & CPF_Parm); ++It)
                {
                    if (FObjectProperty *ObjProp = CastField<FObjectProperty>(*It))
                    {
                        ObjProp->SetObjectPropertyValue_InContainer(Buffer, this);
                    }
                }

                Door->ProcessEvent(Func, Buffer);
            }
            else
            {
                Door->ProcessEvent(Func, nullptr);
            }

            UE_LOG(LogTemp, Log, TEXT("AMayaCharacter: Called %s on door %s"), *FuncName.ToString(), *Door->GetName());
            bSuccess = true;
            break;
        }
    }

    if (!bSuccess)
    {
        FOutputDeviceNull Ar;
        Door->CallFunctionByNameWithArguments(TEXT("Interact"), Ar, nullptr, true);
        Door->CallFunctionByNameWithArguments(TEXT("Open"), Ar, nullptr, true);
    }
}