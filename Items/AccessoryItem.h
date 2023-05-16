// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Items/EquippableItem.h"
#include "AccessoryItem.generated.h"

UENUM(BlueprintType)
enum class EAccessoryItemType : uint8
{
	EMS_Flashlight UMETA(DisplayName = "Flashlight"),

	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS(Blueprintable)
class ESCAPEROOMPROJECT_API UAccessoryItem : public UEquippableItem
{
	GENERATED_BODY()

public:

	UAccessoryItem();

	// accessory data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessory")
	EAccessoryItemType AccessoryItemType;

	// the skeletal mesh corresponding to this accessory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Accessory")
	class USkeletalMesh* Mesh;

protected:

public:

	virtual bool Equip(class APlayerCharacter* PlayerCharacter) override;
	virtual bool Unequip(class APlayerCharacter* PlayerCharacter) override;


};
