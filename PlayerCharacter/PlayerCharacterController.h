// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.generated.h"

//

/**
 * 
 */
UCLASS()
class ESCAPEROOMPROJECT_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()

public:

	APlayerCharacterController();

protected:
	
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintImplementableEvent)
	void AddNewNotification(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void AddNewNarrativeText(const FText& Message);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealthBarBP();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHealthBarBP();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowDeathScreenBP();

	UFUNCTION(BlueprintCallable)
	void Respawn();
	
};

