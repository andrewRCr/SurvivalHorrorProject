// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Items/EquippableItem.h"
#include "WeaponItem.generated.h"

UENUM(BlueprintType)
enum class EWeaponItemType : uint8
{
	EMS_Pistol UMETA(DisplayName = "Pistol"),
	EMS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS(Blueprintable)
class ESCAPEROOMPROJECT_API UWeaponItem : public UEquippableItem
{
	GENERATED_BODY()
	
public: 

	UWeaponItem();

	virtual bool Equip(class APlayerCharacter* PlayerCharacter) override;
	virtual bool Unequip(class APlayerCharacter* PlayerCharacter) override;

	// the weapon actor class to give to the player upon equipping this weapon item
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<class AWeapon> WeaponClass;

	// weapon data
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponItemType WeaponItemType;

	virtual void AddedToInventory(class UInventoryComponent* Inventory, int32 QuantityAdded) override;


};
