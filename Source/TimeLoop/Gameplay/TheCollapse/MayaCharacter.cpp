// Copyright (c) 2026 Team Ficus

#include "MayaCharacter.h"
#include "AIController.h"
#include "CollapseDirector.h"
#include "TimeLoopGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AMayaCharacter::AMayaCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->bUseRVOAvoidance = true;
		MoveComp->AvoidanceWeight = 0.5f;
		MoveComp->MaxWalkSpeed = 250.0f;
	}
}

void AMayaCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCollapseDirector();
	ATimeLoopGameState* GS = GetTimeLoopGameState();

	// Bind on dialog finished
	if (ACollapseDirector *Director = GetCollapseDirector())
	{
		Director->OnDialogueFinished.AddDynamic(this, &AMayaCharacter::HandleDialogueFinished);
		Director->OnCollapseEvent.AddDynamic(this, &AMayaCharacter::HandleCollapseEvent);
		Director->OnResetState.AddDynamic(this, &AMayaCharacter::HandleCollpaseResetState);
	}

	// Maya first state must be drawing
	if (GS && GS->IsInPrologue())
	{
		SetState(EMayaState::MovingToDrawing);
	}
	else
	{
		SetState(EMayaState::Panicked);
	}
}

void AMayaCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (CachedDirector)
	{
		CachedDirector->OnDialogueFinished.RemoveDynamic(this, &AMayaCharacter::HandleDialogueFinished);
		CachedDirector->OnCollapseEvent.RemoveDynamic(this, &AMayaCharacter::HandleCollapseEvent);
		CachedDirector->OnResetState.RemoveDynamic(this, &AMayaCharacter::HandleCollpaseResetState);
	}

	Super::EndPlay(EndPlayReason);
}

void AMayaCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AAIController* AICon = Cast<AAIController>(NewController))
	{
		if (UPathFollowingComponent* PathComp = AICon->GetPathFollowingComponent())
		{
			if (MoveCompleteHandle.IsValid())
			{
				PathComp->OnRequestFinished.Remove(MoveCompleteHandle);
			}
			MoveCompleteHandle = PathComp->OnRequestFinished.AddUObject(this, &AMayaCharacter::HandleMoveCompleted);
		}
	}
}

void AMayaCharacter::SetState(EMayaState NewState)
{
	if (CurrentState == NewState)
	{
		return;
	}

	const EMayaState OldState = CurrentState;
	CurrentState = NewState;

	RunStateLogic();

	OnMayaStateChanged.Broadcast(CurrentState, OldState);
}

void AMayaCharacter::RunStateLogic()
{
	switch (CurrentState)
	{
	case EMayaState::MovingToEthan:
	{
		DetachDrawingActors();
		MoveToTarget(Ethan_TargetPoint);
		break;
	}
	case EMayaState::TalkToEthan:
	{
		StopMovement();
		break;
	}
	case EMayaState::MovingToDrawing:
	{
		MoveToTarget(Drawing_TargetPoint);
		break;
	}
	case EMayaState::Drawing:
	{
		bIsDrawing = true;
		AttachDrawingActors();
		break;
	}
	case EMayaState::Startled:
	case EMayaState::Panicked:
	{
		StopMovement();
		bIsDrawing = false;
		DetachDrawingActors();
		break;
	}
	case EMayaState::SeekingShelter:
	{
		StopMovement();
		MoveToTarget(Shelter_TargetPoint);
		break;
	}
	case EMayaState::Sheltered:
	{
		StopMovement();
		if (ATimeLoopGameState* GS = GetTimeLoopGameState())
		{
			GS->SetObjectiveSolved(true);
		}
		break;
	}
	case EMayaState::Crushed:
	{
		StopMovement();
		break;
	}
	case EMayaState::Saved:
	{
		break;
	}

	default:
		break;
	}
}

void AMayaCharacter::HandleInteraction()
{
	switch (CurrentState)
	{
	case EMayaState::TalkToEthan:
	{
		// We begin the prologue dialogue
		// And advance the dialogue

		// NOTE:
		// BeginPrologueDialog will Freeze Player Input
		// Advance Dialog will Restore Player Input
		if (ACollapseDirector *director = GetCollapseDirector())
		{
			director->BeginPrologueDialogue();
			director->AdvanceDialogue();
		}
		break;
	}
	}
}

void AMayaCharacter::MoveToTarget(AActor* TargetActor)
{
	if (!TargetActor)
	{
		return;
	}

	if (AAIController* AICon = Cast<AAIController>(GetController()))
	{
		AICon->MoveToActor(TargetActor, MoveAcceptanceRadius);
	}
}

void AMayaCharacter::StopMovement()
{
	if (AController* Con = GetController())
	{
		Con->StopMovement();
	}
	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}
}

void AMayaCharacter::HandleMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	if (!Result.IsSuccess())
	{
		return;
	}

	switch (CurrentState)
	{
	case EMayaState::MovingToEthan:
	{
		SetState(EMayaState::TalkToEthan);
		break;
	}
	case EMayaState::MovingToDrawing:
	{
		SetState(EMayaState::Drawing);
		break;
	}

	case EMayaState::SeekingShelter:
	{
		SetState(EMayaState::Sheltered);
		break;
	}
	default:
		break;
	}
}

void AMayaCharacter::HandleCollapseEvent(FName EventId, AActor *EventInstigator)
{
	if (EventId == ApproachEthanId)
	{
		SetState(EMayaState::MovingToEthan);
	}
}

void AMayaCharacter::HandleDialogueFinished()
{
	// When finished talked to Ethan
	switch (CurrentState)
	{
	case EMayaState::TalkToEthan:
	{
		if (CachedDirector)
		{
			// When in prologue Maya going to finished her drawing
			if (CachedDirector->IsPrologue())
			{
				SetState(EMayaState::MovingToDrawing);
			}
		}
		break;
	}

	default:
		break;
	}
}


void AMayaCharacter::HandleCollpaseResetState(const ECollapseEndResult EndResult)
{
	SetState(EMayaState::MovingToDrawing);
}

void AMayaCharacter::AttachDrawingActors()
{
	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp)
	{
		return;
	}

	const FAttachmentTransformRules SnapRules(EAttachmentRule::SnapToTarget, false);

	if (Crayon)
	{
		Crayon->SetActorEnableCollision(true);
		Crayon->AttachToComponent(MeshComp, SnapRules, CrayonSocketName);
	}

	if (DrawingPaper)
	{
		DrawingPaper->SetActorEnableCollision(true);
		DrawingPaper->AttachToComponent(MeshComp, SnapRules, PaperSocketName);
	}
}

void AMayaCharacter::DetachDrawingActors()
{
	const FDetachmentTransformRules KeepWorldRules(EDetachmentRule::KeepWorld, false);

	if (Crayon)
	{
		Crayon->DetachFromActor(KeepWorldRules);
	}

	if (DrawingPaper)
	{
		DrawingPaper->DetachFromActor(KeepWorldRules);
		DrawingPaper->SetActorEnableCollision(true);
	}
}

ACollapseDirector* AMayaCharacter::GetCollapseDirector()
{
	if (!CachedDirector)
	{
		CachedDirector = Cast<ACollapseDirector>(UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass()));
	}
	return CachedDirector;
}

ATimeLoopGameState* AMayaCharacter::GetTimeLoopGameState()
{
	if (!CachedGameState)
	{
		CachedGameState = Cast<ATimeLoopGameState>(UGameplayStatics::GetGameState(this));
	}
	return CachedGameState;
}
