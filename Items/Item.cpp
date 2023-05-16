// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Items/Item.h"
#include "../Components/InventoryComponent.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"


#if WITH_EDITOR
void UItem::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// UPROPERTY clamping doesn't support using a variable to clamp, so doing it here instead (if Quantity is what changed)
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UItem, Quantity))
	{ Quantity = FMath::Clamp(Quantity, 1, bStackable ? MaxStackSize : 1); }
}
#endif

// sets default values
UItem::UItem()
{
	ItemDisplayName = FText::FromString(TEXT("Item Name"));
	ItemDisplayDescription = FText::FromString(TEXT("Item Description"));
	bStackable = false;
	Quantity = 1;
	MaxStackSize = 2;
	PickupSoundVolumeMultiplier = 0.35f;
	bDisableOnPickupSound = false;
}

// function to be overwritten by classes inheriting from this class
void UItem::Use(APlayerCharacter* PlayerCharacter)
{
	BlueprintDefinedUse(PlayerCharacter);
}

// checks if NewQuantity is not the same as Quantity, if true, Quantity is set to NewQuantity, clamping it between
// 1 and either MaxStackSize or 1, depending on whether or not the object is stackable.
void UItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity && bStackable) 
	{
		Quantity = FMath::Clamp(NewQuantity, 0, MaxStackSize);
		OnItemModified.Broadcast();
	}
}

// function to be called by Inventory class when item is added to inventory
void UItem::AddedToInventory(class UInventoryComponent* Inventory, int32 QuantityAdded)
{
	// play pickup sound
	if (PickupSound && !bDisableOnPickupSound)
	{ UGameplayStatics::PlaySound2D(GetWorld(), PickupSound, PickupSoundVolumeMultiplier); }
}
