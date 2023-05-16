// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Items/Item.h"
#include "HealthItem.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEROOMPROJECT_API UHealthItem : public UItem
{
	GENERATED_BODY()
	
public:
	UHealthItem();

	// how much health this item restores when used
	float RestoreAmount;

protected:

private:
	// override the base item use function
	virtual void Use(class APlayerCharacter* PlayerCharacter) override;

};
