// Copyright (c) 2026 Team Ficus

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InventoryComponent.generated.h"

UENUM(BlueprintType)
enum class EInventoryItemType : uint8
{
	Generic UMETA(DisplayName = "Generic"),
	Flashlight UMETA(DisplayName = "Flashlight"),
	Key UMETA(DisplayName = "Key"),
	Note UMETA(DisplayName = "Note"),
	Clue UMETA(DisplayName = "Clue")
};

USTRUCT(BlueprintType)
struct TIMELOOP_API FInventoryItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	EInventoryItemType ItemType = EInventoryItemType::Generic;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FText DisplayName;

	/** Optional key identity. Set this on a key to support doors that require the key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName PairKeyId = NAME_None;

	/** Optional key identity. Set this on a key to support doors that require the key. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bIsPersistent = false;
};

UCLASS(ClassGroup=(TimeLoop), meta=(BlueprintSpawnableComponent))
class TIMELOOP_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInventoryComponent();

	/** Adds an item if ItemId is valid and is not already present. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(FName ItemId, EInventoryItemType ItemType, FText DisplayName, FName PairKeyId, bool bIsPersistent);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(FName ItemId);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveNonPersistentItems();

	/** Returns true when the player owns the exact key identity required by a door. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasKeyForDoor(FName RequiredKeyId);

	/** Equips only equippable inventory types. This does not remove other items. */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool EquipItem(FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void UnequipItem();

	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool IsEquipped(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FName GetEquippedItemId() const;

	UFUNCTION(BlueprintPure, Category = "Inventory")
	const TArray<FInventoryItem>& GetItems() const;

	/** Uses an owned item without changing the equipped item. Notes and keys use this path. */
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool CanUseItem(FName ItemId) const;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventoryItem> Items;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	FName EquippedItemId = NAME_None;

private:
	const FInventoryItem* FindItem(FName ItemId) const;
};
