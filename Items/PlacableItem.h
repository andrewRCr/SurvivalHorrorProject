// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Items/Item.h"
#include "PlacableItem.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class ESCAPEROOMPROJECT_API UPlacableItem : public UItem
{
	GENERATED_BODY()

public:

	UPlacableItem();

protected:

private:


public:

	// override the base item use function
	virtual void Use(class APlayerCharacter* PlayerCharacter) override;

	class APickupContainer* FindPickupContainer(class APlayerCharacter* PlayerCharacter);
};
