// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Enemies/Zombie.h"
#include "../DebugMacros.h"
#include "Components/CapsuleComponent.h"

AZombie::AZombie()
{
	ZombieType = EZombieType::EMS_MAX;

	TopClothingMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("TopClothingMesh"));
	TopClothingMesh->SetupAttachment(GetMesh());

	BottomClothingMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BottomClothingMesh"));
	BottomClothingMesh->SetupAttachment(GetMesh());

	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMesh->SetupAttachment(GetMesh());
}


void AZombie::BeginPlay()
{
	Super::BeginPlay();
}



void AZombie::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
