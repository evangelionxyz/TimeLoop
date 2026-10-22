// Copyright (c) 2026 Team Ficus

#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UInventoryComponent::AddItem(FName ItemId, EInventoryItemType ItemType, FText DisplayName, FName PairKeyId, bool bIsPersistent)
{
	if (ItemId.IsNone() || HasItem(ItemId))
	{
		return false;
	}

	FInventoryItem NewItem;
	NewItem.ItemId = ItemId;
	NewItem.ItemType = ItemType;
	NewItem.DisplayName = MoveTemp(DisplayName);
	NewItem.PairKeyId = PairKeyId;
	NewItem.bIsPersistent = bIsPersistent;
	Items.Add(MoveTemp(NewItem));
	return true;
}

bool UInventoryComponent::RemoveItem(FName ItemId)
{
	const int32 Removed = Items.RemoveAll([ItemId](const FInventoryItem& Item)
	{
		return Item.ItemId == ItemId;
	});

	if (Removed > 0)
	{
		if (EquippedItemId == ItemId)
		{
			UnequipItem();
		}
		return true;
	}

	return false;
}

bool UInventoryComponent::HasItem(FName ItemId) const
{
	return !ItemId.IsNone() && FindItem(ItemId) != nullptr;
}

void UInventoryComponent::RemoveNonPersistentItems()
{
	const int32 Removed = Items.RemoveAll([](const FInventoryItem& Item)
	{
		return !Item.bIsPersistent;
	});

	if (Removed > 0 && !EquippedItemId.IsNone() && !HasItem(EquippedItemId))
	{
		UnequipItem();
	}
}

bool UInventoryComponent::HasKeyForDoor(FName RequiredKeyId)
{
	if (RequiredKeyId.IsNone())
	{
		return true;
	}

	for (const FInventoryItem& Item : Items)
	{
		if (Item.ItemType == EInventoryItemType::Key &&
			(Item.ItemId == RequiredKeyId || Item.PairKeyId == RequiredKeyId))
		{
			return true;
		}
	}

	return false;
}

bool UInventoryComponent::EquipItem(FName ItemId)
{
	const FInventoryItem* Item = FindItem(ItemId);
	if (!Item || Item->ItemType != EInventoryItemType::Flashlight)
	{
		return false;
	}

	EquippedItemId = ItemId;
	return true;
}

void UInventoryComponent::UnequipItem()
{
	EquippedItemId = NAME_None;
}

bool UInventoryComponent::IsEquipped(FName ItemId) const
{
	return !ItemId.IsNone() && EquippedItemId == ItemId;
}

FName UInventoryComponent::GetEquippedItemId() const
{
	return EquippedItemId;
}

const TArray<FInventoryItem>& UInventoryComponent::GetItems() const
{
	return Items;
}

bool UInventoryComponent::CanUseItem(FName ItemId) const
{
	const FInventoryItem* Item = FindItem(ItemId);
	if (!Item)
	{
		return false;
	}

	// Keys, notes, clues, and generic items are usable directly from the inventory.
	// The flashlight is used through EquipItem and can remain equipped while these are used.
	return Item->ItemType != EInventoryItemType::Flashlight;
}

const FInventoryItem* UInventoryComponent::FindItem(FName ItemId) const
{
	return Items.FindByPredicate([ItemId](const FInventoryItem& Item)
	{
		return Item.ItemId == ItemId;
	});
}
