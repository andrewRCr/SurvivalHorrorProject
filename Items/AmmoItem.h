// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Items/Item.h"
#include "AmmoItem.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEROOMPROJECT_API UAmmoItem : public UItem
{
	GENERATED_BODY()
	
public:

	virtual void AddedToInventory(class UInventoryComponent* Inventory, int32 QuantityAdded) override;
};
