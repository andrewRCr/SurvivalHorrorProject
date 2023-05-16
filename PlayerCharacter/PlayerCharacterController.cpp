// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../PlayerCharacter/PlayerCharacterController.h"
#include "Kismet/GameplayStatics.h"


APlayerCharacterController::APlayerCharacterController()
{
}

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacterController::Respawn()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}



