// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "../Enemies/Enemy.h"
#include "Zombie.generated.h"

UENUM(BlueprintType)
enum class EZombieType : uint8
{
	EMS_MaleSkinny		UMETA(DisplayName = "MaleSkinny"),
	EMS_MaleBloated		UMETA(DisplayName = "MaleBloated"),
	EMS_FemaleSkinny	UMETA(DisplayName = "FemaleSkinny"),
	EMS_FemaleBloated	UMETA(DisplayName = "FemaleBloated"),

	EMS_MAX			UMETA(DisplayName = "DefaultMAX")
};

/**
 * 
 */
UCLASS()
class ESCAPEROOMPROJECT_API AZombie : public AEnemy
{
	GENERATED_BODY()

public:

	AZombie();

	// zombie subclass type (male, female, body type)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	EZombieType ZombieType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	class USkeletalMeshComponent* TopClothingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	class USkeletalMeshComponent* BottomClothingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zombie")
	class USkeletalMeshComponent* HairMesh;

protected:

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// called every frame
	virtual void Tick(float DeltaTime) override;
	
};
