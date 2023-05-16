// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/AmmoItem.h"
#include "../Components/InventoryComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"

void UAmmoItem::AddedToInventory(class UInventoryComponent* Inventory, int32 QuantityAdded)
{
	Super::AddedToInventory(Inventory, QuantityAdded);

	APlayerCharacter* PC = Cast<APlayerCharacter>(Inventory->GetOwner());

	if (PC)
	{
		if (PC->bHasWeaponEquipped && PC->EquippedWeapon)
		{ PC->UpdateAmmoCounterBP(); }		 // update HUD
	}
}
