// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/PlacableItem.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerCharacter/PlayerCharacterController.h"
#include "../Components/InventoryComponent.h"
#include "../Components/InteractionComponent.h"
#include "../World/PickupContainer.h"

UPlacableItem::UPlacableItem()
{
}


void UPlacableItem::Use(APlayerCharacter* PlayerCharacter)
{
	// checks if a Controller exists
	APlayerCharacterController* Controller = Cast<APlayerCharacterController>(PlayerCharacter->GetController());
	if (Controller)
	{
		// if PickupContainer found:
		if (FindPickupContainer(PlayerCharacter) != nullptr && FindPickupContainer(PlayerCharacter)->CheckIfContainsPickup() == false)
		{
			// pass reference of item to PickupContainer; clear current interaction with container
			FindPickupContainer(PlayerCharacter)->PlacePickup(this);
			PlayerCharacter->CouldntFindInteractable();
		}

		// otherwise, broadcast a message saying you can't place it here
		else 
		{ Controller->AddNewNarrativeText(FText::FromString("I can't use this here.")); }
	}
}


APickupContainer* UPlacableItem::FindPickupContainer(class APlayerCharacter* PlayerCharacter)
{
	// check if there is an interactable within range
	if (PlayerCharacter->bInteractableFoundOnLastCheck)
	{
		UInteractionComponent* CurrentlyFocusedInteractable = PlayerCharacter->InteractionData.ViewedInteractionComponent;
		
		// check if that interactable object is a PickupContainer
		if (APickupContainer* Container = Cast<APickupContainer>(CurrentlyFocusedInteractable->GetOwner()))
		{ return Container; }
	}

	return nullptr;
}