// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once
#include "CoreMinimal.h"

#include "Components/ActorComponent.h"
#include "../Items/Item.h"
#include "InventoryComponent.generated.h"

// called when the inventory is changed and the UI needs to be updated accordingly
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryModified);


UENUM(BlueprintType)
enum class EItemAddResult : uint8
{
	IAR_NoItemsAdded UMETA(DisplayName = "No items added"),
	IAR_SomeItemsAdded UMETA(DisplayName = "Some items added"),
	IAR_AllItemsAdded UMETA(DisplayName = "All items added")
};


// represents the result of adding an item to the inventory 
USTRUCT(BlueprintType)
struct FItemAddResult
{
	GENERATED_BODY()


public:

	// empty constructor
	FItemAddResult() {};
	// constructor with item quantity param
	FItemAddResult(int32 InItemQuantity) : AmountToGive(InItemQuantity), ActualAmountGiven(0) {};
	// constructor with item quantity and item quantity added params
	FItemAddResult(int32 InItemQuantity, int32 InQuantityAdded) : AmountToGive(InItemQuantity), ActualAmountGiven(InQuantityAdded) {};

	// the amount of the item that we attempted to add to the inventory
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 AmountToGive;

	// the amount of  the item that was actually successfully added to the inventory
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	int32 ActualAmountGiven;

	// the result
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	EItemAddResult Result;

	// if something caused the add to fail (capacity, etc) this contains the reason why
	UPROPERTY(BlueprintReadOnly, Category = "Item Add Result")
	FText ErrorText;

	// helper functions
	static FItemAddResult AddedNone(const int32 InItemQuantity, const FText& ErrorText)
	{
		FItemAddResult AddedNoneResult(InItemQuantity);
		AddedNoneResult.Result = EItemAddResult::IAR_NoItemsAdded;
		AddedNoneResult.ErrorText = ErrorText;

		return AddedNoneResult;
	}

	static FItemAddResult AddedSome(const int32 InItemQuantity, const int32 ActualAmountGiven, const FText& ErrorText)
	{
		FItemAddResult AddedSomeResult(InItemQuantity, ActualAmountGiven);

		AddedSomeResult.Result = EItemAddResult::IAR_SomeItemsAdded;
		AddedSomeResult.ErrorText = ErrorText;

		return AddedSomeResult;
	}

	static FItemAddResult AddedAll(const int32 InItemQuantity)
	{
		FItemAddResult AddedAllResult(InItemQuantity, InItemQuantity);

		AddedAllResult.Result = EItemAddResult::IAR_AllItemsAdded;

		return AddedAllResult;
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ESCAPEROOMPROJECT_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class UItem;

public:	
	// sets default values for this component's properties
	UInventoryComponent();

	UPROPERTY(BlueprintAssignable)
	FOnInventoryModified OnInventoryModified;
	
	// for tracking an added item's pickup ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FName> NonStackablePickupsTaken;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<FVector> NonStackablePickupsTakenLocations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, int32> StackablePickupsTakenToQuantityMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FName, FVector> StackablePickupsTakenToLocationMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TMap<FVector, FName> LocationToStackablePickupsTakenMap;

protected:

	// items array
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TArray<class UItem*> Items;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ClampMin = 0, ClampMax = 100))
	int32 Capacity;

private:

	UItem* AddItem(class UItem* Item);

public:	

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE int32 GetCapacity() const { return Capacity; }

	void SetCapacity(const int32 NewCapacity);

	UFUNCTION(BlueprintPure, Category = "Inventory")
	FORCEINLINE TArray<class UItem*> GetItems() const { return Items; }

	// returns true if we have a given amount of an item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	bool HasItem(TSubclassOf<class UItem> ItemClass, const int32 Quantity = 1) const;

	// returns the first item with the same class as the given item
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItem(class UItem* Item) const;

	// returns the first item with the same class as ItemClass
	UFUNCTION(BlueprintPure, Category = "Inventory")
	UItem* FindItemByClass(TSubclassOf<class UItem> ItemClass) const;

	// get all inventory items that are a child of ItemClass. useful for getting all Weapons, all Consumables, etc
	UFUNCTION(BlueprintPure, Category = "Inventory")
	TArray<UItem*> FindItemsByClass(TSubclassOf<class UItem> ItemClass) const;

	// takes some quantity away from the item's current quantity; removes item from inventory when quantity reaches zero
	
	int32 ConsumeItem(class UItem* Item);
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 ConsumeItem(class UItem* Item, const int32 Quantity);

	// removes a single item from the inventory
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(class UItem* Item);

	// wrapper function for AddItem - checks capacity/stacks prior to add, and adds partial if needed
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItem(class UItem* Item);

	// add an item to the inventory using the item class instead of an item instance
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	FItemAddResult TryAddItemFromClass(TSubclassOf<class UItem> ItemClass, const int32 Quantity = 1);

	int32 GetNumInventorySlotsInUse(bool bDebug);
};
