// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/WeaponItem.h"
#include "../Components/InventoryComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"

UWeaponItem::UWeaponItem()
{
	WeaponItemType = EWeaponItemType::EMS_MAX;
}


bool UWeaponItem::Equip(class APlayerCharacter* PlayerCharacter)
{
	bool bEquipSuccessful = Super::Equip(PlayerCharacter);

	if (bEquipSuccessful && PlayerCharacter)
	{ PlayerCharacter->EquipWeapon(this); }

	return bEquipSuccessful;
}


bool UWeaponItem::Unequip(class APlayerCharacter* PlayerCharacter)
{
	bool bUnequipSuccessful = Super::Unequip(PlayerCharacter);

	if (bUnequipSuccessful && PlayerCharacter)
	{ PlayerCharacter->UnequipWeapon(); }

	return bUnequipSuccessful;
}


void UWeaponItem::AddedToInventory(class UInventoryComponent* Inventory, int32 QuantityAdded)
{
	Super::AddedToInventory(Inventory, QuantityAdded);

	APlayerCharacter* PC = Cast<APlayerCharacter>(Inventory->GetOwner());
	PC->DestroyHolsterAndPouchBP();
}
