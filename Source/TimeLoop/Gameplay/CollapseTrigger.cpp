// Copyright (c) 2026 Team Ficus

#include "CollapseTrigger.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

ACollapseTrigger::ACollapseTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	RootComponent = TriggerBox;
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));
}

void ACollapseTrigger::BeginPlay()
{
	Super::BeginPlay();
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACollapseTrigger::OnOverlapBegin);
}

void ACollapseTrigger::OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromWeep, const FHitResult &SweepResult)
{
	if (bHasTriggered && bTriggerOnce)
	{
		return;
	}

	// Only trigger for the player character
	if (!OtherActor || OtherActor != UGameplayStatics::GetPlayerCharacter(this, 0))
	{
		return;
	}

	ACollapseDirector *Director = GetDirector();
	if (!Director)
	{
		return;
	}

	// Phase Check
	if (bCheckPhase && Director->GetPhase() != RequiredPhase)
	{
		return;
	}

	// Dialogue Finished Check (e.g Quake trigger only fires if gift dialogue is done)
	if (bRequiredDialogueFinished && !Director->HasDialogFinished())
	{
		return;
	}

	// Fire event to director
	bHasTriggered = true;
	Director->HandleTriggerEvent(EventId, OtherActor);

	if (bTriggerOnce)
	{
		TriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

ACollapseDirector *ACollapseTrigger::GetDirector()
{
	if (!CachedDirector)
	{
		CachedDirector = Cast<ACollapseDirector>(UGameplayStatics::GetActorOfClass(this, ACollapseDirector::StaticClass()));
	}
	return CachedDirector;
}

