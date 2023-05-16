// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupContainer.generated.h"

UCLASS()
class ESCAPEROOMPROJECT_API APickupContainer : public AActor
{
	GENERATED_BODY()
	
public:	

	// sets default values for this actor's properties
	APickupContainer();

	/*
	*  components
	*/
	
	// mesh representing the container's in-world appearance/object-type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PickupContainer")
	UStaticMeshComponent* PickupContainerMeshComp;

	// to prompt the player to place items / detect if in range to place
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PickupContainer")
	class UInteractionComponent* InteractionComponent;

	/*
	*  variables
	*/

	// the item currently "in" the container (represented in-world by a corresponding Pickup)
	UPROPERTY(EditInstanceOnly, Instanced, BlueprintReadWrite, Category = "PickupContainer")
	class UItem* CurrentItem;

	// flag whether the container contains a (ANY, not just correct) pickup
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PickupContainer")
	bool ContainsPickup;

	// the "correct" item needed to progress a puzzle
	UPROPERTY(EditInstanceOnly, Instanced, BlueprintReadWrite, Category = "PickupContainer")
	class UItem* CorrectItem;

	// flag whether CorrectItem is currently placed in the container
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PickupContainer")
	bool CorrectItemPlaced;

	// when spawning pickup, do so at this loc/rot/scale
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "PickupContainer", meta = (MakeEditWidget = true))
	FTransform PickupSpawnTransform;

protected:

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// called every frame
	virtual void Tick(float DeltaTime) override;

	void PlacePickup(class UItem* IncomingItem);
	
	// getter for bool ContainsPickup
	FORCEINLINE bool CheckIfContainsPickup() { return ContainsPickup; }

	UFUNCTION(BlueprintImplementableEvent)
	void BuildPickup();

	UFUNCTION(BlueprintImplementableEvent)
	void PickupDestroyedNotification();
};
