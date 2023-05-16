// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

UCLASS()
class ESCAPEROOMPROJECT_API APickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// sets default values for this actor's properties
	APickup();
	
	// name of pickup object
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	FName PickupID;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup")
	FVector PickupLocationID;

	// Reference to a Static Mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class UStaticMeshComponent* PickupMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class UInteractionComponent* InteractionComponent;

	// temp variable to set when/if spawned by a PickupContainer (i.e., when placing a PlacableItem)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
	class APickupContainer* CurrentPickupContainer;

	
protected:
	
	// called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pickup")
	class UItem* Item;

	UPROPERTY(EditAnywhere, Instanced, BlueprintReadWrite, Category = "Pickup")
	class UItem* ItemTemplate;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	FText OnTakePickup(class APlayerCharacter* Taker);

	UFUNCTION()
	void OnItemModified();

public:	

	UFUNCTION(BlueprintCallable)
	void InitializePickup(const TSubclassOf<class UItem> ItemClass, const int32 Quantity);
};
