// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/EquippableItem.h"
#include "../Components/InventoryComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerCharacter/PlayerCharacterController.h"

#define LOCTEXT_NAMESPACE "EquippableItem"

UEquippableItem::UEquippableItem()
{
	bStackable = false;
	bEquipped = false;
	UseActionText = LOCTEXT("ItemUseActionText", "Equip");
	EquipUnequipSoundVolumeMultiplier = 0.35f;
	bDisableOnEquipSound = false;
}


// instead of using, either Equip() or Unequip() depending on whether currently equipped
void UEquippableItem::Use(class APlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter)
	{
		// if item is already equipped, unequip. if not, equip
		if (PlayerCharacter->GetEquippedItems().Contains(Slot) && !bEquipped)
		{
			UEquippableItem* AlreadyEquippedItem = *PlayerCharacter->GetEquippedItems().Find(Slot);
			AlreadyEquippedItem->SetEquipped(false);
		}

		SetEquipped(!IsEquipped());
	}
}


void UEquippableItem::SetEquipped(bool bNewEquipped)
{
	bEquipped = bNewEquipped;
	EquipStatusChanged();
}


// adjust UseActionText 
void UEquippableItem::EquipStatusChanged()
{
	if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetOuter()))
	{	
		// "use" action text appropriately
		UseActionText = bEquipped ? LOCTEXT("UnequipText", "Unequip") : LOCTEXT("EquipText", "Equip");

		if (bEquipped) 
		{ Equip(PlayerCharacter); }

		else 
		{ Unequip(PlayerCharacter); }
	}

	// tell UI to update
	OnItemModified.Broadcast();
}


// call player's EquipItem() with this passed
bool UEquippableItem::Equip(class APlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter) 
	{ return PlayerCharacter->EquipItem(this); }

	return false;
}

// call player's UnequipItem() with this passed
bool UEquippableItem::Unequip(class APlayerCharacter* PlayerCharacter)
{
	if (PlayerCharacter) 
	{ return PlayerCharacter->UnequipItem(this); }

	return false;
}

#undef LOCTEXT_NAMESPACE