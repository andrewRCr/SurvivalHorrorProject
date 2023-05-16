// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../World/Pickup.h"
#include "../DebugMacros.h"
#include "../Items/Item.h"
#include "../Components/InventoryComponent.h"
#include "../Components/InteractionComponent.h"
#include "../World/PickupContainer.h"

#define LOCTEXT_NAMESPACE "Inventory"

// sets default values
APickup::APickup()
{
	// Creating a StaticMeshComponent and setting it's properties
	PickupMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("PickupMesh");
	PickupMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	SetRootComponent(PickupMeshComponent);

	InteractionComponent = CreateDefaultSubobject<UInteractionComponent>("PickupInteractionComponent");
	InteractionComponent->InteractionTime = 0.f;
	InteractionComponent->InteractionDistance = 200.0f;
	InteractionComponent->SetupAttachment(PickupMeshComponent);

	PickupID = MakeUniqueObjectName(GetOuter(), GetClass());
}

// called when the game starts or when spawned
void APickup::BeginPlay()
{
	Super::BeginPlay();

	if (ItemTemplate)
	{ InitializePickup(ItemTemplate->GetClass(), ItemTemplate->GetQuantity()); }
}


#if WITH_EDITOR
void APickup::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// if a new pickup is selected in the property editor, change the mesh to reflect the new item being selected
	if (PropertyName == GET_MEMBER_NAME_CHECKED(APickup, ItemTemplate))
	{
		if (ItemTemplate)
		{ PickupMeshComponent->SetStaticMesh(ItemTemplate->PickupMesh); }
	}
}
#endif


// called on interaction
FText APickup::OnTakePickup(APlayerCharacter* Taker)
{
	if (!Taker) { return FText::FromString("INVALID: no Taker");}

	if (Item)
	{
		if (UInventoryComponent* PlayerInventory = Taker->PlayerInventory)
		{
			const FItemAddResult AddResult = PlayerInventory->TryAddItem(Item);
			FText ResultText;

			// added all
			if (AddResult.ActualAmountGiven >= Item->GetQuantity())
			{
				// record pickup ID so we know whether to spawn pickup in world on save game load
				if (!Item->bStackable)
				{ 
					PlayerInventory->NonStackablePickupsTaken.Add(PickupID); 
					PlayerInventory->NonStackablePickupsTakenLocations.Add(PickupLocationID);
				}

				if (Item->bStackable)
				{	
					PlayerInventory->StackablePickupsTakenToLocationMap.Emplace(PickupID, GetActorLocation());
					PlayerInventory->LocationToStackablePickupsTakenMap.Emplace(GetActorLocation(), PickupID);
				}
	
				// if pickup is currently in a container, unflag ContainsPickup on that container
				if (CurrentPickupContainer != nullptr)
				{
					CurrentPickupContainer->PickupDestroyedNotification();
					CurrentPickupContainer->ContainsPickup = false;
					CurrentPickupContainer->InteractionComponent->InteractionDistance = 100.f;
					CurrentPickupContainer->InteractionComponent->bShouldShowInteractPrompt = true;
					CurrentPickupContainer->InteractionComponent->SetVisibility(true);
					CurrentPickupContainer->CurrentItem = nullptr;
					CurrentPickupContainer->CorrectItemPlaced = false;
					
				}

				// self-destruct
				Destroy();
				// notify player
				ResultText = FText::Format(LOCTEXT("SuccessText", "You got the {ItemName}."), Item->ItemDisplayName);
			}

			// added some
			else if (AddResult.ActualAmountGiven < Item->GetQuantity())
			{
				Item->SetQuantity(Item->GetQuantity() - AddResult.ActualAmountGiven);
				ResultText = AddResult.ErrorText;
			}

			return ResultText;
		}

		return FText::FromString("INVALID: Taker has no InventoryComponent");
	}

	return FText::FromString("INVALID: Pickup has no Item assigned");
}

void APickup::OnItemModified()
{
}

// initializes a new UItem object, sets quantity and Pickup's mesh
void APickup::InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity)
{
	if (ItemClass && Quantity > 0) 
	{
		Item = NewObject<UItem>(this, ItemClass);
		Item->SetQuantity(Quantity);
		PickupMeshComponent->SetStaticMesh(Item->PickupMesh);
		Item->OnItemModified.AddDynamic(this, &APickup::OnItemModified);
	}
}

#undef LOCTEXT_NAMESPACE