// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Components/InventoryComponent.h"
#include "../DebugMacros.h"

#define LOCTEXT_NAMESPACE "Inventory"

// sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	Capacity = 9;
}


UItem* UInventoryComponent::AddItem(UItem* Item)
{
	if (Item)
	{
		// creating a new instance of the object, now owned by this inventory, and set its properties
		UItem* NewItem = NewObject<UItem>(GetOwner(), Item->GetClass());
		NewItem->SetQuantity(Item->GetQuantity());
		NewItem->OwningInventory = this;
		NewItem->bDisableOnPickupSound = Item->bDisableOnPickupSound;
		NewItem->MaxStackSize = Item->MaxStackSize;
		NewItem->bStackable = Item->bStackable;

		// add it to inventory
		NewItem->AddedToInventory(this, Item->GetQuantity());
		Items.Add(NewItem);
		OnInventoryModified.Broadcast();

		return NewItem;
	}
	return nullptr;
}


void UInventoryComponent::SetCapacity(const int32 NewCapacity)
{
	Capacity = NewCapacity;
	OnInventoryModified.Broadcast();
}


// returns true if we have a given amount of an item
bool UInventoryComponent::HasItem(TSubclassOf<class UItem> ItemClass, const int32 Quantity /*= 1*/) const
{
	if (UItem* ItemToFind = FindItemByClass(ItemClass))
	{ return ItemToFind->GetQuantity() >= Quantity; }

	return false;
}


// returns the first item with the same class as the given item
UItem* UInventoryComponent::FindItem(class UItem* Item) const
{
	if (Item)
	{
		for (auto& InvItem : Items)
		{
			if (InvItem && InvItem->GetClass() == Item->GetClass())
			{ return InvItem; }
		}
	}

	return nullptr;
}


// returns the first item with the same class as ItemClass
UItem* UInventoryComponent::FindItemByClass(TSubclassOf<class UItem> ItemClass) const
{
	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass() == ItemClass)
		{ return InvItem; }
	}

	return nullptr;
}


// get all inventory items that are a child of ItemClass. useful for getting all Weapons, all Consumables, etc
TArray<UItem*> UInventoryComponent::FindItemsByClass(TSubclassOf<class UItem> ItemClass) const
{
	TArray<UItem*> ItemsOfClass;

	for (auto& InvItem : Items)
	{
		if (InvItem && InvItem->GetClass()->IsChildOf(ItemClass))
		{ ItemsOfClass.Add(InvItem); }
	}

	return ItemsOfClass;
}


int32 UInventoryComponent::ConsumeItem(class UItem* Item)
{
	if (Item)
	{ ConsumeItem(Item, Item->GetQuantity()); }
	return 0;
}



int32 UInventoryComponent::ConsumeItem(class UItem* Item, const int32 Quantity)
{
	if (Item)
	{
		// check that we are not consuming more than we have, then update quantity
		const int32 RemoveQuantity = FMath::Min(Quantity, Item->GetQuantity());
		ensure(!(Item->GetQuantity() - RemoveQuantity < 0));
		Item->SetQuantity(Item->GetQuantity() - RemoveQuantity);

		if (Item->GetQuantity() <= 0)
		{ RemoveItem(Item); }

		else
		{ OnInventoryModified.Broadcast(); }

		return RemoveQuantity;
	}
	return 0;
}


// removes a single item from the inventory
bool UInventoryComponent::RemoveItem(UItem* Item)
{
	if (Item)
	{
		Items.RemoveSingle(Item);
		OnInventoryModified.Broadcast();
		return true;
	}

	return false;
}

// wrapper function for AddItem - checks capacity/stacks prior to add, and adds partial if needed
FItemAddResult UInventoryComponent::TryAddItem(UItem* Item)
{
	// validates item's properties
	if (Item) 
	{
		const int32 AddAmount = Item->GetQuantity();

		if (!Item->bStackable && GetNumInventorySlotsInUse(false) + 1 > GetCapacity())
		{ return FItemAddResult::AddedNone(AddAmount, FText::Format(LOCTEXT("InventoryCapacityFullText", "Not enough room to take {ItemName}."), Item->ItemDisplayName)); }

		// if item is stackable, check if we already have any and if so add to corresponding stack
		if (Item->bStackable)
		{
			// should never go over max stack size
			ensure(Item->GetQuantity() <= Item->MaxStackSize);

			// if already have some of item and stackable, modify (increment) existing inventory quantity instead of adding entirely new
			if (UItem* ExistingItem = FindItem(Item))
			{
				// if room in stack
				if (ExistingItem->GetQuantity() < ExistingItem->MaxStackSize)
				{
					// determine how much of the item to add
					const int32 CapacityMaxAddAmount = ExistingItem->MaxStackSize - ExistingItem->GetQuantity();
					int32 ActualAddAmount = FMath::Min(AddAmount, CapacityMaxAddAmount);

					FText ErrorText = LOCTEXT("InventoryErrorText", "Couldn't add all of the {ItemName}s to your inventory.");

					if (ActualAddAmount < AddAmount)
					{
						// not enough capacity in inventory
						ErrorText = FText::Format(LOCTEXT("InventoryCapacityFullText", "Can't take all of the {ItemName}s - inventory is full."), Item->ItemDisplayName);
					}

					// we couldn't add *any* of the item to inventory
					if (ActualAddAmount <= 0)
					{ return FItemAddResult::AddedNone(AddAmount, LOCTEXT("InventoryErrorText", "Couldn't add item to inventory.")); }

					// success, checks passed: increment item quantity
					ExistingItem->SetQuantity(ExistingItem->GetQuantity() + ActualAddAmount);
					
					// call AddedToInventory for sound effect
					ExistingItem->AddedToInventory(this, ActualAddAmount);
					// if we somehow get more of the item than the max stack size, something is wrong with the math
					ensure(ExistingItem->GetQuantity() <= ExistingItem->MaxStackSize);

					if (ActualAddAmount < AddAmount)
					{ return FItemAddResult::AddedSome(AddAmount, ActualAddAmount, ErrorText); }

					else
					{ return FItemAddResult::AddedAll(AddAmount); }
				}

				else
				{ return FItemAddResult::AddedNone(AddAmount, FText::Format(LOCTEXT("InventoryFullStackText", "Can't carry any more {ItemName}s."), Item->ItemDisplayName)); }
			}

			else
			{
				// since we do not have any of this item, and there's no room to start a new stack, add none
				if (GetNumInventorySlotsInUse(false) + 1 > GetCapacity())
				{ return FItemAddResult::AddedNone(AddAmount, FText::Format(LOCTEXT("InventoryCapacityFullText", "Not enough room to take {ItemName}."), Item->ItemDisplayName)); }

				// since we do not have any of this item and there's room to start a stack, add the full stack
				AddItem(Item);
				return FItemAddResult::AddedAll(AddAmount);
			}
		}

		else // item isn't stackable
		{
			// non-stackable items should always have a quantity of 1
			ensure(Item->GetQuantity() == 1);

			AddItem(Item);
			return FItemAddResult::AddedAll(AddAmount);
		}
	}

	return FItemAddResult::AddedNone(-1, LOCTEXT("ErrorMessage", ""));
}


// construct item from class and try to add to inventory (i.e., not from a Pickup)
FItemAddResult UInventoryComponent::TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity /*= 1*/)
{
	UItem* Item = NewObject<UItem>(GetOwner(), ItemClass);
	Item->SetQuantity(Quantity);
	Item->bDisableOnPickupSound = true;
	return TryAddItem(Item);
}


int32 UInventoryComponent::GetNumInventorySlotsInUse(bool bDebug)
{
	int32 NumSlotsInUse = 0;
	TArray<UItem*> UniqueStackableItems;
	for (UItem* EachItem : Items)
	{
		if (EachItem->bStackable)
		{ UniqueStackableItems.AddUnique(EachItem); }

		else 
		{ NumSlotsInUse ++; }
	}
	
	NumSlotsInUse += UniqueStackableItems.Num();

	if (bDebug)
	{
		FString StackableCountStr = FString::FromInt(UniqueStackableItems.Num());
		printFString("stackable count: %s", *StackableCountStr);

		FString TotalCountStr = FString::FromInt(NumSlotsInUse);
		printFString("total slots in use: %s", *TotalCountStr);

		FString CapacityStr = FString::FromInt(GetCapacity());
		printFString("capacity: %s", *CapacityStr);
	}

	return NumSlotsInUse;
}


#undef LOCTEXT_NAMESPACE