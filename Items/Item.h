// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Item.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnItemModified);


/**
 * 
 */
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class ESCAPEROOMPROJECT_API UItem : public UObject
{
	GENERATED_BODY()

public:

	UItem();

	// inventory this item currently belongs to
	UPROPERTY(VisibleAnywhere, Category = "Item")
	class UInventoryComponent* OwningInventory;

	// inventory display name for this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FText ItemDisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, noclear, Category = "Item")
	TSubclassOf<class UItem> LookupClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (EditCondition = bStackable))
	int32 Quantity;

	// whether or not this item can be stacked in the inventory
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	bool bStackable;

	// maximum size a stack of these items can be
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 2, EditCondition = bStackable))
	int32 MaxStackSize;

	// description of the item to display in the inventory when item is selected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
	FText ItemDisplayDescription;

	// mesh to display for this item's in-world pickup
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* PickupMesh;

	// thumbnail inventory picture for this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UTexture2D* ItemThumbnail;

	// mesh to display when examining this item in inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* ExaminationMesh;

	// for adjusting examination mesh location relative to scene capture camera
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FVector ExaminationMeshOffset;

	// for adjusting examination mesh rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FRotator ExaminationMeshRotation;

	// the verb text for using the item (e.g., equip, eat, etc)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	FText UseActionText;

	// on-pickup sound
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class USoundBase* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	bool bDisableOnPickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float PickupSoundVolumeMultiplier;

	UPROPERTY(BlueprintAssignable)
	FOnItemModified OnItemModified;

	UFUNCTION(Category = "Item")
	FORCEINLINE bool ShouldNotifyOnAdd() const { return bDisableOnPickupSound; }


protected:
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	
	virtual void Use(class APlayerCharacter* PlayerCharacter);

	// any BP-only defined Use behavior to be called
	UFUNCTION(BlueprintImplementableEvent, Category = "Item")
	void BlueprintDefinedUse(class APlayerCharacter* PlayerCharacter);

	UFUNCTION(BlueprintCallable, Category = "Item")
	FORCEINLINE int32 GetQuantity() const { return Quantity; }

	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetQuantity(const int32 NewQuantity);

	UFUNCTION(BlueprintCallable)
	virtual void AddedToInventory(class UInventoryComponent* Inventory, int32 QuantityAdded);

};
