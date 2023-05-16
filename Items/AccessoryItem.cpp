// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/AccessoryItem.h"
#include "../PlayerCharacter/PlayerCharacter.h"

UAccessoryItem::UAccessoryItem()
{
	AccessoryItemType = EAccessoryItemType::EMS_MAX;
}


bool UAccessoryItem::Equip(class APlayerCharacter* PlayerCharacter)
{
	bool bEquipSuccessful = Super::Equip(PlayerCharacter);

	if (bEquipSuccessful && PlayerCharacter)
	{ PlayerCharacter->EquipAccessory(this); }

	return bEquipSuccessful;
}


bool UAccessoryItem::Unequip(class APlayerCharacter* PlayerCharacter)
{
	bool bUnequipSuccessful = Super::Unequip(PlayerCharacter);

	if (bUnequipSuccessful && PlayerCharacter)
	{ PlayerCharacter->UnequipAccessory(); }

	return bUnequipSuccessful;
}
