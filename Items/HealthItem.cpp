// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/HealthItem.h"
#include "../DebugMacros.h"
#include "../Components/InventoryComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerCharacter/PlayerCharacterController.h"

UHealthItem::UHealthItem()
{
	RestoreAmount = 35.f;
}

void UHealthItem::Use(APlayerCharacter* PlayerCharacter)
{
	APlayerCharacterController* Controller = Cast<APlayerCharacterController>(PlayerCharacter->GetController());
	if (Controller) 
	{
		// validate before healing
		if (PlayerCharacter->Health < PlayerCharacter->MaxHealth)
		{ 
			// heal player + consume item
			PlayerCharacter->ModifyHealth(RestoreAmount); 
			if (UInventoryComponent* Inventory = PlayerCharacter->PlayerInventory)
			{ Inventory->ConsumeItem(this, 1); }
			BlueprintDefinedUse(PlayerCharacter);
		}

		else
		{ Controller->AddNewNarrativeText(FText::FromString("I don't need to use this now.")); }
	}
}
