// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Items/Item.h"
#include "EquippableItem.generated.h"


UENUM(BlueprintType)
enum class EEquippableSlot : uint8
{
	EIS_Accessory UMETA(DisplayName = "Accessory"),
	EIS_Weapon UMETA(DisplayName = "Weapon")
};


UCLASS(Abstract, NotBlueprintable)
class ESCAPEROOMPROJECT_API UEquippableItem : public UItem
{
	GENERATED_BODY()

public:

	UEquippableItem();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equippables")
	EEquippableSlot Slot;

	// plays when equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equippables")
	class USoundBase* OnEquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equippables")
	bool bDisableOnEquipSound;

	// plays when unequipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equippables")
	class USoundBase* OnUnequipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equippables")
	float EquipUnequipSoundVolumeMultiplier;

protected:

	UPROPERTY()
	bool bEquipped;

	UFUNCTION()
	void EquipStatusChanged();
	
public:
	
	// instead of using/consuming, either Equip() or Unequip() depending on whether currently equipped
	virtual void Use(class APlayerCharacter* PlayerCharacter) override;

	// set appropriately + call protected EquipStatusChanged()
	void SetEquipped(bool bNewEquipped);

	// call player's EquipItem() with this passed
	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool Equip(class APlayerCharacter* PlayerCharacter);

	// call player's UnequipItem() with this passed
	UFUNCTION(BlueprintCallable, Category = "Equippables")
	virtual bool Unequip(class APlayerCharacter* PlayerCharacter);

	// helper function
	UFUNCTION(BlueprintPure, Category = "Equippables")
	bool IsEquipped() { return bEquipped; };
};
