// Copyright (c) 2026 Team Ficus

#include "CollapseTrigger.h"
#include "CollapseDirector.h"
#include "InventoryComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ACollapseTrigger::ACollapseTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	Box->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));
	Box->SetGenerateOverlapEvents(true);
	Box->SetHiddenInGame(true);
}

void ACollapseTrigger::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &ACollapseTrigger::HandleBeginOverlap);
	Box->OnComponentEndOverlap.AddDynamic(this, &ACollapseTrigger::HandleEndOverlap);

	if (!Director)
	{
		Director = Cast<ACollapseDirector>(UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass()));
	}
}

void ACollapseTrigger::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RecheckHandle);
	}
	Super::EndPlay(EndPlayReason);
}

void ACollapseTrigger::SetTriggerEnabled(bool bEnabled)
{
	Box->SetCollisionEnabled(bEnabled ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	if (!bEnabled)
	{
		GetWorldTimerManager().ClearTimer(RecheckHandle);
		OverlappingPlayer.Reset();
	}
}

bool ACollapseTrigger::PlayerMeetsRequirements(const AActor* Player) const
{
	if (RequiredItemId.IsNone())
	{
		return true;
	}

	const UInventoryComponent* Inventory = Player ? Player->FindComponentByClass<UInventoryComponent>() : nullptr;
	return Inventory && Inventory->HasItem(RequiredItemId);
}

void ACollapseTrigger::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || !Pawn->IsPlayerControlled() || bFired)
	{
		return;
	}

	OverlappingPlayer = OtherActor;
	TryFire();

	// Missing the required item: keep checking while the player stays in the volume.
	if (!bFired)
	{
		GetWorldTimerManager().SetTimer(RecheckHandle, this, &ACollapseTrigger::TryFire, 0.25f, true);
	}
}

void ACollapseTrigger::HandleEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == OverlappingPlayer.Get())
	{
		OverlappingPlayer.Reset();
		GetWorldTimerManager().ClearTimer(RecheckHandle);
	}
}

void ACollapseTrigger::TryFire()
{
	AActor* Player = OverlappingPlayer.Get();
	if (!Player || bFired || !PlayerMeetsRequirements(Player))
	{
		return;
	}

	if (!Director)
	{
		Director = Cast<ACollapseDirector>(UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass()));
	}
	if (!Director)
	{
		UE_LOG(LogTemp, Warning, TEXT("CollapseTrigger '%s' has no ACollapseDirector to notify."), *GetName());
		return;
	}

	GetWorldTimerManager().ClearTimer(RecheckHandle);

	Director->HandleTriggerEvent(EventId, Player);

	if (bOneShot)
	{
		bFired = true;
		SetTriggerEnabled(false);
	}
}
