// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../World/PickupContainer.h"
#include "../Components/InteractionComponent.h"
#include "../Items/Item.h"
#include "../World/Pickup.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../Components/InventoryComponent.h"


// sets default values
APickupContainer::APickupContainer()
{	
	// setup Components
	PickupContainerMeshComp = CreateDefaultSubobject<UStaticMeshComponent>("PickupContainerMesh");
	this->SetRootComponent(PickupContainerMeshComp);
	
	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("InteractionComponent");
	InteractionComponent->InteractionTime = 0.f;
	InteractionComponent->InteractionDistance = 200.f;
	InteractionComponent->SetupAttachment(PickupContainerMeshComp);
	
	// initialize variables
	ContainsPickup = false;
	CorrectItemPlaced = false;
	CurrentItem = nullptr;
	CorrectItem = nullptr;
}


// called when the game starts or when spawned
void APickupContainer::BeginPlay()
{
	Super::BeginPlay();
}


// called every frame
void APickupContainer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


// when a PlacableItem is placed, stores reference to that item and spawns corresponding pickup
void APickupContainer::PlacePickup(class UItem* IncomingItem)
{
	// Set Current Item to the Incoming Item; call BP-implemented BuildPickup() to spawn pickup
	if (IncomingItem) 
	{
		CurrentItem = IncomingItem;
		BuildPickup();
	}
}