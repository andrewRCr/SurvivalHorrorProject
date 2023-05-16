// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerCharacter/PlayerCharacterController.h"
#include "../Components/InteractionComponent.h"
#include "../Components/InventoryComponent.h"
#include "../DebugMacros.h"
#include "../Enemies/Enemy.h"
#include "../Items/AccessoryItem.h"
#include "../Items/WeaponItem.h"
#include "../Weapons/Weapon.h"
#include "../World/PickupContainer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"


// sets default values
APlayerCharacter::APlayerCharacter()
{
 	// set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	/**
	*	create modular Skeletal Mesh Components
	*/

	BeltPouchMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BeltPouchMesh"));
	BeltPouchMesh->SetupAttachment(GetMesh());

	HolsteredPistolMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HolsterPistolMesh"));
	HolsteredPistolMesh->SetupAttachment(GetMesh(), FName("Holster_Socket"));

	HolsterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HolsterMesh"));
	HolsterMesh->SetupAttachment(GetMesh());

	FlashlightMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FlashlightMesh"));
	FlashlightMesh->SetupAttachment(GetMesh(), FName("FlashlightSocket"));

	FeetMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FeetMesh"));
	FeetMesh->SetupAttachment(GetMesh());

	LegsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("LegsMesh"));
	LegsMesh->SetupAttachment(GetMesh());

	HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh"));
	HandsMesh->SetupAttachment(GetMesh());

	ChestMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ChestMesh"));
	ChestMesh->SetupAttachment(GetMesh());

	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairMesh->SetupAttachment(GetMesh(), FName("Hair_Socket"));

	/**
	*   create camera and noise components
	*/

	// create + attach third-person spring arm to mesh (pulls towards the player if there's a collision)
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetMesh(), FName("CameraSocket"));
	SpringArmComponent->bUsePawnControlRotation = true; // rotate arm based on controller

	// follows at this target distance + offset from right shoulder
	SpringArmComponent->TargetArmLength = 185.f;
	SpringArmComponent->SocketOffset = FVector(0.f, 65.f, -25.f);

	// second spring-arm (for smoother adjustment when collision detected)
	SpringArmComponent2 = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent2"));
	SpringArmComponent2->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	SpringArmComponent2->bUsePawnControlRotation = true; // rotate arm based on controller
	SpringArmComponent2->TargetArmLength = 0.f;

	// create + attach third-person camera to spring arm
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent2);
	CameraComponent->bUsePawnControlRotation = true; // rotate camera based on controller

	NoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("NoiseEmitter"));


	/**
	*   camera defaults
	*/

	DefaultFOV = 80.f;
	AimingFOV = DefaultFOV - 25.f;
	AimingZoomSpeed = 5.f;

	AimingSensitivityModifier = .75f;
	XSensitivityValue = 1.25f;
	XInvertModifier = 1.f;
	YSensitivityValue = 1.25f;
	YInvertModifier = 1.f;
	bInvertXInput = false;
	bInvertYInput = false;

	/**
	*   base stats + movement defaults
	*/

	MaxHealth = 100.f;
	Health = MaxHealth;

	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
	WalkingSpeed = 200.f;
	SprintingSpeed = 350.f;
	CrouchedSpeed = 165.f;

	// default movement status = Idle_Standing; default speed = WalkingSpeed
	MovementStatus = EMovementStatus::EMS_Idle_Standing;
	LastMovementStatus = MovementStatus;
	GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
	
	// set CharacterMovement crouched defaults; using built-in crouch function purely to set adjusted capsule size
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;
	GetCharacterMovement()->SetCrouchedHalfHeight(57.f);
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchedSpeed;

	/**
	*   movement modifiers
	*/

	bIdle = true;
	bWantsToCrouch = false;
	bWantsToSprint = false;
	bTurning = false;
	bPushing = false;
	bInCinematic = false;
	bAlive = true;

	/**
	*   interaction defaults
	*/

	NearbyInteractionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("NearbyInteractionSphere"));
	NearbyInteractionSphere->SetupAttachment(GetRootComponent());
	NearbyInteractionSphere->InitSphereRadius(400.f);

	InteractionCheckFrequency = 0.25f;
	InteractionCheckDistance = 1000.0f;
	bInteractableFoundOnLastCheck = false;

	/**
	*   inventory/equipment defaults
	*/

	PlayerInventory = CreateDefaultSubobject<UInventoryComponent>("PlayerInventory");
	PlayerInventory->SetCapacity(20);
	bHasAccessoryEquipped = false;
	bHasFlashlightEquipped = false;

	/**
	*   combat related defaults
	*/

	bCanTakeDamage = true;
	TakeDamageDelay = 1.f;
	bWantsToAim = false;
	FocusedAimDelay = 1.5f;
	HideAmmoCounterDelay = 2.f;
	RespawnDelay = 5.0f;

	// crosshair modifiers
	CrosshairSpreadMultiplier = 0.f;
	CrosshairVelocityFactor = 0.f;
	CrosshairFocusedAimFactor = 0.f;
	CrosshairShootingFactor = 0.f;
	VelocityCrosshairModifier = 1.5f;
	ShootingCrosshairModifier = 0.4f;
	FocusedAimCrosshairModifier = 0.7f;

	// shooting fire timer variables
	ShootTimeDuration = 0.05f;
	bFiringBullet = false;
}


// called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();	

	Tags.Add(FName("Player"));
}


// called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// flag whether currently idle
	float Speed = GetVelocity().Length();
	bIdle = Speed > 0 ? false : true;

	// set appropriate idle movement status
	if (bIdle)
	{
		EMovementStatus IdleStatus = bWantsToCrouch ? EMovementStatus::EMS_Idle_Crouched : EMovementStatus::EMS_Idle_Standing;

		if (MovementStatus != IdleStatus)
		{ SetMovementStatus(IdleStatus); }
	}

	// check for player interactions; optimization (so not checking every single frame) + make noise if sprinting
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();

		if (!bIdle && MovementStatus == EMovementStatus::EMS_Walking || MovementStatus == EMovementStatus::EMS_Sprinting)
		{
			float Loudness = MovementStatus == EMovementStatus::EMS_Sprinting ? 1.0f : .5f;
			NoiseEmitter->MakeNoise(this, Loudness, GetActorLocation());
		}
	}

	// handle aiming/zoom
	AimingFOV = FMath::Clamp(DefaultFOV - 30.f, 55.f, 70.f);
	float DesiredFOV = bWantsToAim && bAlive ? AimingFOV : DefaultFOV;
	CameraComponent->SetFieldOfView(FMath::FInterpTo(CameraComponent->FieldOfView, DesiredFOV, DeltaTime, AimingZoomSpeed));

	// update crosshair spread multiplier value
	CalculateCrosshairSpread(DeltaTime);

	// set bFocusedAim depending on how long idle and aiming
	if (IsAiming() && bIdle && EquippedWeapon->CurrentState == EWeaponState::Idle)
	{
		if (GetWorldTimerManager().IsTimerActive(TimerHandle_FocusedAim) == false)
		{ GetWorldTimerManager().SetTimer(TimerHandle_FocusedAim, this, &APlayerCharacter::SetFocusedAim, FocusedAimDelay, false); }
	}

	else
	{
		if (bFocusedAim) { bFocusedAim = false; }
		if (GetWorldTimerManager().IsTimerActive(TimerHandle_FocusedAim) == true)
		{ ResetFocusedAim(); }
	}
	

	if (!bAlive)
	{
		bIdle = true;

		// offset if utilizing certain poses to look aligned with ground correctly
		FVector CorpseVerticalOffset;
		if (PlayerDeathPose == EPlayerDeathPose::EPD_ForwardPose1)
		{ CorpseVerticalOffset = FVector(0.f, 0.f, 5.f); }

		else
		{ CorpseVerticalOffset = FVector(0.f, 0.f, 3.f); }

		GetCapsuleComponent()->AddLocalOffset(CorpseVerticalOffset);
	}
}


// called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	// directional movement
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);

	// mouse look/camera movement
	PlayerInputComponent->BindAxis("Turn", this, &APlayerCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerCharacter::LookUp);

	// crouching; deprecated
	//PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &APlayerCharacter::CrouchToggle);

	// sprinting
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &APlayerCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &APlayerCharacter::StopSprinting);

	// interacting
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerCharacter::BeginInteract);
	PlayerInputComponent->BindAction("Interact", IE_Released, this, &APlayerCharacter::EndInteract);

	// aiming / zoomed view when no weapon equipped
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &APlayerCharacter::StartAiming);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &APlayerCharacter::StopAiming);

	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &APlayerCharacter::StartFire);
	PlayerInputComponent->BindAction("Shoot", IE_Released, this, &APlayerCharacter::StopFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &APlayerCharacter::ReloadWeapon);
}


void APlayerCharacter::MoveForward(float Value)
{
	// if axis input exists
	if (Value != 0.f)
	{
		// apply movement (of magnitude of the passed in Value) in the PlayerCharacter's forward direction
		AddMovementInput(GetActorForwardVector(), Value);

		DetermineNewMovementStatus();
	}
}


void APlayerCharacter::MoveRight(float Value)
{
	// if axis input exists
	if (Value != 0.f)
	{
		// apply movement (of magnitude of the passed in Value) in the PlayerCharacter's right direction
		AddMovementInput(GetActorRightVector(), Value);

		DetermineNewMovementStatus();
	}
}


void APlayerCharacter::Turn(float Value)
{
	if (IsAiming())
	{ Value = Value * AimingSensitivityModifier; }

	/* RECOIL APPLICATION*/
	// if the player has moved their camera to compensate for recoil we need this to cancel out the recoil reset effect
	if (!FMath::IsNearlyZero(RecoilResetAmount.X, 0.01f))
	{
		if (RecoilResetAmount.X > 0.f && Value > 0.f)
		{ RecoilResetAmount.X = FMath::Max(0.f, RecoilResetAmount.X - Value); }

		else if (RecoilResetAmount.X < 0.f && Value < 0.f)
		{ RecoilResetAmount.X = FMath::Min(0.f, RecoilResetAmount.X - Value); }
	}

	// apply the recoil over several frames
	if (!FMath::IsNearlyZero(RecoilBumpAmount.X, 0.1f))
	{
		FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.X = FMath::FInterpTo(RecoilBumpAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

		AddControllerYawInput(LastCurrentRecoil.X - RecoilBumpAmount.X);
	}

	// slowly reset back to center after recoil is processed
	FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.X = FMath::FInterpTo(RecoilResetAmount.X, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddControllerYawInput(LastRecoilResetAmount.X - RecoilResetAmount.X);

	// if axis input exists
	if (Value != 0.f && !bInCinematic)
	{ AddControllerYawInput(Value * XSensitivityValue * XInvertModifier); }
}


void APlayerCharacter::LookUp(float Value)
{
	if (IsAiming())
	{
		Value = Value * AimingSensitivityModifier;
	}

	/* RECOIL APPLICATION*/
	// if the player has moved their camera to compensate for recoil we need this to cancel out the recoil reset effect
	if (!FMath::IsNearlyZero(RecoilResetAmount.Y, 0.01f))
	{
		if (RecoilResetAmount.Y > 0.f && Value > 0.f)
		{ RecoilResetAmount.Y = FMath::Max(0.f, RecoilResetAmount.Y - Value); }
		else if (RecoilResetAmount.Y < 0.f && Value < 0.f)
		{ RecoilResetAmount.Y = FMath::Min(0.f, RecoilResetAmount.Y - Value); }
	}

	// apply the recoil over several frames
	if (!FMath::IsNearlyZero(RecoilBumpAmount.Y, 0.01f))
	{
		FVector2D LastCurrentRecoil = RecoilBumpAmount;
		RecoilBumpAmount.Y = FMath::FInterpTo(RecoilBumpAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilSpeed);

		AddControllerPitchInput(LastCurrentRecoil.Y - RecoilBumpAmount.Y);
	}

	// slowly reset back to center after recoil is processed
	FVector2D LastRecoilResetAmount = RecoilResetAmount;
	RecoilResetAmount.Y = FMath::FInterpTo(RecoilResetAmount.Y, 0.f, GetWorld()->DeltaTimeSeconds, CurrentRecoilResetSpeed);
	AddControllerPitchInput(LastRecoilResetAmount.Y - RecoilResetAmount.Y);

	// if axis input exists
	if (Value != 0.f && !bInCinematic)
	{ AddControllerPitchInput(Value * YSensitivityValue * YInvertModifier); }
}


void APlayerCharacter::SetMovementStatus(EMovementStatus NewStatus)
{
	// validate
	if (NewStatus == MovementStatus) { return; }

	// update character's status
	LastMovementStatus = MovementStatus;
	MovementStatus = NewStatus;

	// using character movement crouch function to automatically set adjusted capsule height
	if (NewStatus == EMovementStatus::EMS_CrouchWalking || NewStatus == EMovementStatus::EMS_Idle_Crouched) { Crouch(); }
	else { UnCrouch(); }

	// set appropriate speed on character movement component
	switch (MovementStatus)
	{
		case EMovementStatus::EMS_Dead:
			break;

		case EMovementStatus::EMS_Idle_Standing:
			break;

		case EMovementStatus::EMS_Idle_Crouched:
			break;

		case EMovementStatus::EMS_CrouchWalking:
			GetCharacterMovement()->MaxWalkSpeed = CrouchedSpeed;
			break;

		case EMovementStatus::EMS_Walking:
			GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
			break;

		case EMovementStatus::EMS_Sprinting:
			GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;

			// sprinting clears crouching/aiming buffer flags; ending sprint always resets to standing/walking
			SetCrouching(false);
			SetAiming(false);

			break;

		default:
			GetCharacterMovement()->MaxWalkSpeed = WalkingSpeed;
			break;
	}
}


// called by tick function when transitioning from idle to moving; checks buffer flags + sets status accordingly
void APlayerCharacter::DetermineNewMovementStatus()
{
	if (bAlive)
	{
		// from Idle_Crouched: sprinting breaks out of crouching; if no sprint buffered, crouch walk
		if (bWantsToCrouch)
		{
			if (!bWantsToAim)
			{
				EMovementStatus NewStatus = bWantsToSprint ? EMovementStatus::EMS_Sprinting : EMovementStatus::EMS_CrouchWalking;
				SetMovementStatus(NewStatus);
			}

			else
			{ SetMovementStatus(EMovementStatus::EMS_CrouchWalking); }

		}

		// from Idle_Standing
		else
		{
			if (!bWantsToAim)
			{
				EMovementStatus NewStatus = bWantsToSprint ? EMovementStatus::EMS_Sprinting : EMovementStatus::EMS_Walking;
				SetMovementStatus(NewStatus);
			}

			else
			{ SetMovementStatus(EMovementStatus::EMS_Walking); }
		}
	}

	else
	{ SetMovementStatus(EMovementStatus::EMS_Dead); }

}


void APlayerCharacter::CrouchToggle()
{
	bool bNewCrouchBuffer = bWantsToCrouch ? false : true;
	SetCrouching(bNewCrouchBuffer);
}


void APlayerCharacter::SetCrouching(const bool bNewCrouchBuffer)
{
	if (bNewCrouchBuffer == bWantsToCrouch) { return; }

	// update flag
	bWantsToCrouch = bNewCrouchBuffer;
}

void APlayerCharacter::StartSprinting()
{
	SetSprinting(true);
}


void APlayerCharacter::StopSprinting()
{
	SetSprinting(false);
}


void APlayerCharacter::SetSprinting(const bool bNewSprintBuffer)
{
	if (bNewSprintBuffer == bWantsToSprint) { return; }

	// update flag
	bWantsToSprint = bNewSprintBuffer;
}


void APlayerCharacter::StartAiming()
{
	SetAiming(true);

	if (EquippedWeapon && bAlive && !bInCinematic)
	{
		ShowAmmoCounter();
		GetWorldTimerManager().ClearTimer(TimerHandle_HideAmmoCounterTimer);
	}
}


void APlayerCharacter::StopAiming()
{
	SetAiming(false);
	
	if (EquippedWeapon)
	{
		EquippedWeapon->StopFire();
		HideAmmoCounterAfterDelay();
	}
}


void APlayerCharacter::SetAiming(const bool bNewAimingBuffer)
{
	if (bNewAimingBuffer == bWantsToAim) { return; }

	// update flag
	bWantsToAim = bNewAimingBuffer;
}


void APlayerCharacter::StartFire()
{
	if (IsAiming())
	{ EquippedWeapon->StartFire(); }
}


void APlayerCharacter::StopFire()
{
	if (IsAiming())
	{ EquippedWeapon->StopFire(); }
}


void APlayerCharacter::PerformInteractionCheck()
{
	// validate
	if (GetController() == nullptr) { return; }

	// store time of interaction check in struct for reference in tick function
	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	// perform capsule sweep in front of player
	FVector PlayerLoc;
	FRotator PlayerRot;

	PlayerLoc = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	PlayerRot = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorRotation();

	FVector SweepStart = PlayerLoc;
	FVector SweepEnd = (PlayerRot.Vector() * InteractionCheckDistance) + SweepStart;

	TArray<FHitResult> OutResults;
	FCollisionShape PlayerCloseCapsule = FCollisionShape::MakeCapsule(50.f, 100.f);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	bool isHit = (GetWorld()->SweepMultiByChannel(OutResults, SweepStart, SweepEnd, FQuat::Identity, ECC_Visibility, PlayerCloseCapsule, QueryParams));

	// sweep hit something
	if (isHit)
	{
		for (auto& Hit : OutResults)
		{
			// verify hit object is an interactable
			if (Hit.GetActor() != nullptr)
			{
				if (UInteractionComponent* InteractionComponent = Cast<UInteractionComponent>(Hit.GetActor()->FindComponentByClass(UInteractionComponent::StaticClass())))
				{
					float Distance = (SweepStart - InteractionComponent->GetComponentLocation()).Size();

					if (InteractionComponent != GetInteractable() && Distance <= InteractionComponent->InteractionDistance)
					{
						// check if interactable detected is a PickupContainer; if so, if has pickup placed, ignore it
						if (APickupContainer* Container = Cast<APickupContainer>(InteractionComponent->GetOwner()))
						{
							if (Container->ContainsPickup) { continue; }
						}

						// return success
						FoundNewInteractable(InteractionComponent);
					}

					// interactable object is too far away
					else if (Distance > InteractionComponent->InteractionDistance && GetInteractable())
					{ CouldntFindInteractable(); }	// return failure

					return;
				}
			}
		}
	}

	// return failure
	CouldntFindInteractable();
}


// called when PerformInteractionCheck() returns failure
void APlayerCharacter::CouldntFindInteractable()
{
	// we've lost focus on an interactable; clear the timer
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_Interact))
	{ GetWorldTimerManager().ClearTimer(TimerHandle_Interact); }

	// tell the interactable we've stopped focusing on it, and clear the current interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		Interactable->EndFocus(this);

		if (InteractionData.bInteractHeld)
		{ EndInteract(); }
	}

	InteractionData.ViewedInteractionComponent = nullptr;
	bInteractableFoundOnLastCheck = false;
}


// called when PerformInteractionCheck() returns success
void APlayerCharacter::FoundNewInteractable(UInteractionComponent* Interactable)
{
	// cancel any current interaction
	EndInteract();

	if (UInteractionComponent* OldInteractable = GetInteractable())
	{ OldInteractable->EndFocus(this); }

	InteractionData.ViewedInteractionComponent = Interactable;
	Interactable->BeginFocus(this);

	bInteractableFoundOnLastCheck = true;
}

// helper function - is the player currently interacting?
bool APlayerCharacter::IsInteracting() const
{
	return GetWorldTimerManager().IsTimerActive(TimerHandle_Interact);
}


void APlayerCharacter::BeginInteract()
{
	// manual interaction check call
	PerformInteractionCheck();

	InteractionData.bInteractHeld = true;

	// if have found interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{
		// if it has an interaction component
		Interactable->BeginInteract(this);

		// instant interact (on press)
		if (FMath::IsNearlyZero(Interactable->InteractionTime))
		{ Interact(); }

		// hold to interact (time required)
		else
		{ GetWorldTimerManager().SetTimer(TimerHandle_Interact, this, &APlayerCharacter::Interact, Interactable->InteractionTime, false); }
	}
}


void APlayerCharacter::EndInteract()
{
	InteractionData.bInteractHeld = false;

	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	// if have interactable
	if (UInteractionComponent* Interactable = GetInteractable())
	{ Interactable->EndInteract(this); }
}


void APlayerCharacter::Interact()
{
	GetWorldTimerManager().ClearTimer(TimerHandle_Interact);

	if (UInteractionComponent* Interactable = GetInteractable())
	{
		if (IsAiming() == false)
		{ Interactable->Interact(this); }
	}
}


// helper function - how much time remaining in current interaction?
float APlayerCharacter::GetRemainingInteractTime()
{
	return GetWorldTimerManager().GetTimerRemaining(TimerHandle_Interact);
}


void APlayerCharacter::UseItem(class UItem* Item)
{
	if (Item)
	{
		// can't use an item you don't have
		if (PlayerInventory && !PlayerInventory->FindItem(Item)) { return; }

		Item->Use(this);
		PlayerInventory->OnInventoryModified.Broadcast();
	}
}


bool APlayerCharacter::EquipItem(class UEquippableItem* Item)
{
	if (Item)
	{
		// equip the item in the appropriate slot
		EquippedItems.Add(Item->Slot, Item);
		OnEquippedItemsChanged.Broadcast(Item->Slot, Item);

		// play OnEquip sound
		if (Item->OnEquipSound && !Item->bDisableOnEquipSound)
		{ UGameplayStatics::PlaySound2D(GetWorld(), Item->OnEquipSound, Item->EquipUnequipSoundVolumeMultiplier); }

		return true;
	}

	return false;
}


bool APlayerCharacter::UnequipItem(class UEquippableItem* Item)
{
	if (Item)
	{
		if (EquippedItems.Contains(Item->Slot))
		{
			if (Item == *EquippedItems.Find(Item->Slot))
			{
				// remove item from appropriate slot
				EquippedItems.Remove(Item->Slot);
				OnEquippedItemsChanged.Broadcast(Item->Slot, nullptr);

				// play OnUnequip sound
				if (Item->OnUnequipSound)
				{ UGameplayStatics::PlaySound2D(GetWorld(), Item->OnUnequipSound, Item->EquipUnequipSoundVolumeMultiplier); }
			}
		}

		return true;
	}

	return false;
}


void APlayerCharacter::EquipAccessory(class UAccessoryItem* Accessory)
{
	if (Accessory && Accessory->AccessoryItemType != EAccessoryItemType::EMS_MAX)
	{
		// if already equipped, unequip
		if (bHasAccessoryEquipped)
		{ UnequipAccessory(); }

		else
		{
			if (Accessory->AccessoryItemType == EAccessoryItemType::EMS_Flashlight)
			{
				FlashlightMesh->SetSkeletalMesh(Accessory->Mesh);
				bHasFlashlightEquipped = true;
			}

			bHasAccessoryEquipped = true;
		}
	}
}


void APlayerCharacter::UnequipAccessory()
{
	if (bHasAccessoryEquipped)
	{
		// flashlight equipped?
		if (FlashlightMesh != nullptr)
		{
			FlashlightMesh->SetSkeletalMesh(nullptr);
			bHasFlashlightEquipped = false;

			if (bFlashlightOn)
			{ TurnOffFlashlightOnUnequipBP(); }
		}

		bHasAccessoryEquipped = false;
	}
}


void APlayerCharacter::EquipWeapon(class UWeaponItem* WeaponItem)
{
	if (WeaponItem && WeaponItem->WeaponClass)
	{
		if (EquippedWeapon)
		{ UnequipWeapon(); }

		// spawn weapon
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		SpawnParams.Owner = SpawnParams.Instigator = this;

		if (AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponItem->WeaponClass, SpawnParams))
		{
			Weapon->WeaponItem = WeaponItem;
			EquippedWeapon = Weapon;
			Weapon->OnEquip();
			ShowAmmoCounterBP();
			ShowInvAmmoCounterImmediatelyBP();
		}
	}
}


void APlayerCharacter::UnequipWeapon()
{
	if (EquippedWeapon)
	{ EquippedWeapon->OnUnequip(); }

	HideAmmoCounterImmediatelyBP();
	HideInvAmmoCounterImmediatelyBP();
}


// manual reload via player input
void APlayerCharacter::ReloadWeapon()
{
	if (EquippedWeapon)
	{
		if (EquippedWeapon->CanReload())
		{ EquippedWeapon->StartReload(); }
	}
}


void APlayerCharacter::ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed)
{
	RecoilBumpAmount += RecoilAmount;
	RecoilResetAmount += -RecoilAmount;

	CurrentRecoilSpeed = RecoilSpeed;
	CurrentRecoilResetSpeed = RecoilResetSpeed;

	LastRecoilTime = GetWorld()->GetTimeSeconds();
}


// update crosshair spread dependent on factors
void APlayerCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	// calculate crosshair velocity factor
	FVector2D WalkSpeedRange {0.f, WalkingSpeed};
	FVector2D VelocityMultiplierRange {0.f, 1.f};
	FVector Velocity = GetVelocity();
	// only want lateral velocity, so zero out Z
	Velocity.Z = 0.f;

	// normalize velocity factor
	CrosshairVelocityFactor = VelocityCrosshairModifier * FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	// calculate crosshair focused aim factor
	float FocusedAimFactorTarget = bFocusedAim ? FocusedAimCrosshairModifier : 0.0f;
	CrosshairFocusedAimFactor = FMath::FInterpTo(CrosshairFocusedAimFactor, FocusedAimFactorTarget, DeltaTime, 30.f);

	// calculate crosshair shooting factor
	float ShootingFactorTarget = bFiringBullet ? ShootingCrosshairModifier : 0.0f;
	CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, ShootingFactorTarget, DeltaTime, 60.f);

	if (EquippedWeapon)
	{
		if (EquippedWeapon->CurrentAmmoInClip > 0)
		{
			// multiplier will be between 0.5 and 1.5
			CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor - CrosshairFocusedAimFactor + CrosshairShootingFactor;
		}

		else
		{	
			// when no ammo in magazine, only display crosshair at max spread (i.e., as if moving and no focused aim reduction)
			CrosshairSpreadMultiplier = 0.5f + 1.f;
		}
	}
}


// getter, blueprint callable
float APlayerCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}


// clear flag, and if timer active, clear timer
void APlayerCharacter::ResetFocusedAim()
{
	bFocusedAim = false;
	if (GetWorldTimerManager().IsTimerActive(TimerHandle_FocusedAim) == true)
	{ GetWorldTimerManager().ClearTimer(TimerHandle_FocusedAim); }
}


// set flag + set timer to unflag
void APlayerCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(TimerHandle_CrosshairShootTimer, this, &APlayerCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void APlayerCharacter::ShowAmmoCounter()
{
	UpdateAmmoCounterBP();
	ShowAmmoCounterBP();
	GetWorldTimerManager().ClearTimer(TimerHandle_HideAmmoCounterTimer);
}

void APlayerCharacter::HideAmmoCounterAfterDelay()
{
	GetWorldTimerManager().SetTimer(TimerHandle_HideAmmoCounterTimer, this, &APlayerCharacter::HideAmmoCounterBP, HideAmmoCounterDelay, false);
}


float APlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	if (bCanTakeDamage)
	{
		if (EquippedWeapon)
		{
			StopFire();
			EquippedWeapon->StopReload();
		}

		// call parent function
		Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

		// take health damage;
		bCanTakeDamage = false;
		ModifyHealth(-DamageAmount);

		// deathblow?
		if (Health <= 0.f)
		{
			Die(DamageCauser);
			return 0.0f;
		}

		else // still alive after damage proc
		{
			// reset can take damage flag
			GetWorldTimerManager().SetTimer(TakeDamage_TimerHandle, this, &APlayerCharacter::ResetCanTakeDamage, TakeDamageDelay);

			// play random hurt cue
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), RandomHurtCue, GetActorLocation(), 1.f);
			
			// play the appropriate hit reaction anim
			FPlayerHitReact HitReactData = GetPlayerHitReactToPlay();
			PlayAnimMontage(HitReactData.MontageToPlay, 1.f, HitReactData.SectionToPlay);
		
			return DamageAmount;
		}
	}

	return 0.f;
}


void APlayerCharacter::Die(AActor* Causer)
{
	bAlive = false;
	bWantsToAim = false;
	SetMovementStatus(EMovementStatus::EMS_Dead);
	if (EquippedWeapon) { EquippedWeapon->StopReload(); }
	Tags.Add(FName("Dead"));

	// play random death vocal cue
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), RandomDeathCue, GetActorLocation(), 1.f);

	// play death anim -> death pose + death speech cue + set DeathEnd timer + call BP-side function
	float AnimDuration = PlayDeathMontage();
	if (AnimDuration <= 0.f) { AnimDuration = 1.f; }
	OnDeathBP();
	GetWorldTimerManager().SetTimer(DeathEnd_TimerHandle, this, &APlayerCharacter::DeathEnd, AnimDuration);
}


void APlayerCharacter::DeathEnd()
{
	// spawn blood pool + initiate respawn timer
	SpawnPlayerBloodPoolBP();

	if (APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController()))
	{
		PC->ShowDeathScreenBP();
		GetWorldTimerManager().SetTimer(Respawn_TimerHandle, PC, &APlayerCharacterController::Respawn, RespawnDelay);
	}
}

float APlayerCharacter::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;
	Health = FMath::Clamp<float>(Health + Delta, 0.0f, MaxHealth);
	APlayerCharacterController* PC = Cast<APlayerCharacterController>(GetController());

	if (PC) 
	{ 
		PC->ShowHealthBarBP();
		PC->UpdateHealthBarBP();
	}

	UpdateBloodScreenBP();
	return Health - OldHealth;
}


void APlayerCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 APlayerCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) { return -1; }

	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}


float APlayerCharacter::PlayDeathMontage()
{
	// play appropriate death anim + update death pose to match
	int32 Selection;

	// validate death anim space
	bool ClearFront = IsClearFront();
	bool ClearBehind = IsClearBehind();

	if (!ClearFront && ClearBehind)
	{
		Selection = PlayRandomMontageSection(DeathMontage, DeathMontageBackwardOnlySections);
		PlayerDeathPose = EPlayerDeathPose::EPD_BackwardPose1;
	}

	else if (!ClearBehind && ClearFront)
	{
		Selection = PlayRandomMontageSection(DeathMontage, DeathMontageForwardOnlySections);
		PlayerDeathPose = EPlayerDeathPose::EPD_ForwardPose1;
	}

	else // either both blocked or both clear
	{
		Selection = PlayRandomMontageSection(DeathMontage, DeathMontageBackwardOnlySections);
		PlayerDeathPose = EPlayerDeathPose::EPD_BackwardPose1; // favor backwards pose
	}

	FString SelectStr = FString::FromInt(Selection);

	float AnimDuration = DeathMontage->GetSectionLength(Selection);
	return AnimDuration;
}


bool APlayerCharacter::IsClearFront()
{
	FHitResult FrontTraceHit;
	FVector PlayerLoc = GetActorLocation();
	FVector LoweredPlayerLoc = FVector(PlayerLoc.X, PlayerLoc.Y, (PlayerLoc.Z - 50.f));
	const FVector TraceStart = LoweredPlayerLoc + (GetActorForwardVector() * 10.f);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * 50.f);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 40.f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, FrontTraceHit, true, FLinearColor::Gray, FLinearColor::Blue, 0.f);

	return !Hit;
}


bool APlayerCharacter::IsClearBehind()
{
	FHitResult BehindTraceHit;
	FVector PlayerLoc = GetActorLocation();
	FVector LoweredPlayerLoc = FVector(PlayerLoc.X, PlayerLoc.Y, (PlayerLoc.Z - 50.f));
	const FVector TraceStart = LoweredPlayerLoc + (GetActorForwardVector() * -10.f);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * -50.f);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 40.f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, BehindTraceHit, true, FLinearColor::Gray, FLinearColor::Blue, 0.f);

	return !Hit;
}


EHitDirection APlayerCharacter::GetHitDirection()
{
	// determine direction of last hit
	const FVector Forward = GetActorForwardVector();  // returns a normalized vector
	const FVector ToEnemy = (LastHitEnemyLocation - GetActorLocation()).GetSafeNormal();

	// Forward * ToHit = |Forward||ToHit| * cos(theta)
	// |Forward| = 1, |ToHit| = 1, so Forward * ToHit = cos(theta)
	const double CosTheta = FVector::DotProduct(Forward, ToEnemy);
	// take inverse cosine (arc-cosine) of cos(theta) to get just theta
	double Theta = FMath::Acos(CosTheta);
	// convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);


	// if CrossProduct points down, Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToEnemy);
	if (CrossProduct.Z < 0) { Theta *= -1.f; }

	EHitDirection HitDirection;

	// strictly: -45.f && 45.f
	if (Theta >= -45.f && Theta < 45.f)
	{ HitDirection = EHitDirection::EHD_Front; }

	// strictly: -135.f && -45.f
	else if (Theta >= -135.f && Theta < -45.f)
	{ HitDirection = EHitDirection::EHD_Left; }

	// strictly: 45.f && 135.f
	else if (Theta >= 45.f && Theta < 135.f)
	{ HitDirection = EHitDirection::EHD_Right; }

	else
	{ HitDirection = EHitDirection::EHD_Back; }

	return HitDirection;
}


FPlayerHitReact APlayerCharacter::GetPlayerHitReactToPlay()
{
	EHitDirection HitDirection = GetHitDirection();

	// determine which montage and section to play
	UAnimMontage* MontageToPlay = HitReactsMontage;
	FName SectionToPlay;

	switch (HitDirection)
	{
	case EHitDirection::EHD_Front:
		SectionToPlay = FName("FromFront");
		break;
	case EHitDirection::EHD_Back:
		SectionToPlay = FName("FromBack");
		break;
	case EHitDirection::EHD_Left:
		SectionToPlay = FName("FromLeft");
		break;
	case EHitDirection::EHD_Right:
		SectionToPlay = FName("FromRight");
		break;

	default:
		SectionToPlay = FName("FromFront");
		break;
	}

	FPlayerHitReact HitReactData;
	HitReactData.MontageToPlay = MontageToPlay;
	HitReactData.SectionToPlay = SectionToPlay;

	return HitReactData;
}