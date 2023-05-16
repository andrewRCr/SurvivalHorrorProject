// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Items/EquippableItem.h"
#include "PlayerCharacter.generated.h"


// movement status enums
UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Dead		  UMETA(DisplayName = "Dead"),
	EMS_Idle_Standing UMETA(DisplayName = "Idle_Standing"),
	EMS_Idle_Crouched UMETA(DisplayName = "Idle_Crouched"),
	EMS_CrouchWalking UMETA(DisplayName = "CrouchWalking"),
	EMS_Walking		  UMETA(DisplayName = "Walking"),
	EMS_Sprinting	  UMETA(DisplayName = "Sprinting"),

	EMS_MAX		      UMETA(DisplayName = "DefaultMAX")
};


UENUM(BlueprintType)
enum EPlayerDeathPose
{
	EPD_BackwardPose1	UMETA(DisplayName = "BackwardPose1"),
	EPD_ForwardPose1	UMETA(DisplayName = "ForwardPose1"),

	EPD_MAX				UMETA(DisplayName = "DefaultMAX")
};


UENUM(BlueprintType)
enum class EHitDirection : uint8
{
	EHD_Front		UMETA(DisplayName = "Front"),
	EHD_Back		UMETA(DisplayName = "Back"),
	EHD_Left		UMETA(DisplayName = "Left"),
	EHD_Right		UMETA(DisplayName = "Right"),

	EHD_MAX		UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FPlayerHitReact
{
	GENERATED_BODY()

	UAnimMontage* MontageToPlay;
	FName SectionToPlay;
};

// interaction data struct
USTRUCT()
struct FInteractionData
{
	GENERATED_BODY()

	// set defaults
	FInteractionData()
	{
		ViewedInteractionComponent = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	// the current interactable component we're viewing, if there is one
	UPROPERTY()
	class UInteractionComponent* ViewedInteractionComponent;

	// the time when we last checked for an interactable
	UPROPERTY()
	float LastInteractionCheckTime;

	// whether the player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquippableSlot, Slot, const UEquippableItem*, Item);
DECLARE_DELEGATE_OneParam(FCustomInputDelegate, const bool);


UCLASS()
class ESCAPEROOMPROJECT_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// sets default values for this character's properties
	APlayerCharacter();

	/**
	*   camera & noisemaker
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArmComponent2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Noise", meta = (AllowPrivateAccess = "true"))
	class UPawnNoiseEmitterComponent* NoiseEmitter;

	/*
	*  modular Skeletal Mesh Components
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* HairMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* ChestMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* HandsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* LegsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* FeetMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* FlashlightMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* HolsterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* HolsteredPistolMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Components")
	class USkeletalMeshComponent* BeltPouchMesh;

	/**
	*   camera defaults
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float DefaultFOV;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	float AimingFOV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float AimingZoomSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float AimingSensitivityModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float XSensitivityValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float XInvertModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float YSensitivityValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float YInvertModifier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bInvertXInput;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	bool bInvertYInput;

	/**
	*   health stats + movement defaults
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus MovementStatus;

	FORCEINLINE EMovementStatus GetMovementStatus() { return MovementStatus; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums")
	EMovementStatus LastMovementStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SprintingSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchedSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bAlive;

	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EPlayerDeathPose> PlayerDeathPose;

	/**
	*  movement modifiers
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bIdle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bWantsToSprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bWantsToCrouch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bTurning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bWantsToAim;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bPushing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bInCinematic;

	/**
	*  interaction modifiers and data
	*/

	// tells interaction components in range to show their interactable icons (not input-specific)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	class USphereComponent* NearbyInteractionSphere;

	// did we find anything to interact with last check?
	UPROPERTY(BlueprintReadOnly)
	bool bInteractableFoundOnLastCheck;

	// how often in escond to check for an interactable object; set to zero for every tick
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckFrequency;

	// how far we'll trace when we check if the player is looking at an interactable object
	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	float InteractionCheckDistance;

	// information about the current state of the player's interaction
	UPROPERTY()
	FInteractionData InteractionData;

	FTimerHandle TimerHandle_Interact;

	/**
	*   items / inventory related
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UInventoryComponent* PlayerInventory;

	// called to update the inventory
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnEquippedItemsChanged OnEquippedItemsChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bHasAccessoryEquipped;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	bool bHasFlashlightEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bFlashlightOn;

	/**
	*   weapon related
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	class AWeapon* EquippedWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bHasWeaponEquipped;

	// amount of recoil to apply, smoothly over several frames
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilBumpAmount;

	// amount of recoil the gun has had, that we need to reset (after shooting, slowly return the recoil  to normal)
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	FVector2D RecoilResetAmount;

	// speed at which the recoil bumps up per second
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float CurrentRecoilSpeed;

	// speed at which the recoil resets per second
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float CurrentRecoilResetSpeed;

	// last time that we applied recoil
	UPROPERTY(VisibleAnywhere, Category = "Recoil")
	float LastRecoilTime;

	// determines spread of crosshairs
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairSpreadMultiplier;

	// velocity component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairVelocityFactor;

	// how much should the crosshair expand when affected by velocity?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	float VelocityCrosshairModifier;

	// aim component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairFocusedAimFactor;

	// shooting component for crosshair spread
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crosshairs")
	float CrosshairShootingFactor;

	// how much should the crosshair expand when shooting?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	float ShootingCrosshairModifier;

	// how long when aiming and idle before 'focusing' the crosshair
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	float FocusedAimDelay;

	// is crosshair focused?
	bool bFocusedAim;

	// how much should the crosshair shrink when focused?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshairs")
	float FocusedAimCrosshairModifier;

	// timer to determine if aimed long enough to be "focused"
	FTimerHandle TimerHandle_FocusedAim;

	float ShootTimeDuration;

	bool bFiringBullet;

	FTimerHandle TimerHandle_CrosshairShootTimer;

	FTimerHandle TimerHandle_HideAmmoCounterTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float HideAmmoCounterDelay;

	/**
	 *   combat related
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanTakeDamage;

	FTimerHandle TakeDamage_TimerHandle;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float TakeDamageDelay;

	FTimerHandle DeathEnd_TimerHandle;

	FTimerHandle Respawn_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	float RespawnDelay;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageAllSections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageBackwardOnlySections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageForwardOnlySections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FVector LastHitEnemyLocation;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* HitReactsMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	USoundBase* RandomHurtCue;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	USoundBase* RandomDeathCue;

protected:

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

	// for efficient access to currently equipped items
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TMap<EEquippableSlot, UEquippableItem*> EquippedItems;

public:	

	// called every frame
	virtual void Tick(float DeltaTime) override;

	/**
	*   input
	*/

	// called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// called for forwards/backwards input
	void MoveForward(float Value);

	// called for side to side (strafe) input
	void MoveRight(float Value);

	// called for x-axis mouse look / camera movement input
	void Turn(float Value);
	// called for y-axis mouse look / camera movement input
	void LookUp(float Value);

	// set movement status enum (and corresponding movement speed); called by Tick when idle and DetermineNewMovementStatus() when moving
	void SetMovementStatus(EMovementStatus NewStatus);

	// called by MoveForward and MoveRight to determine what to feed SetMovementStatus when transitioning out of idle
	void DetermineNewMovementStatus();

	// toggle between crouching / standing (sets crouch buffer flag)
	void CrouchToggle();
	void SetCrouching(const bool bNewCrouchBuffer);

	// start and stop sprinting functions (held / released, respectively)
	void StartSprinting();
	void StopSprinting();

	// set normal movement / sprinting state
	void SetSprinting(const bool bNewSprintBuffer);

	// start and stop aiming functions (held / released, respectively)
	void StartAiming();
	void StopAiming();

	// set aiming state
	void SetAiming(const bool bNewAimingBuffer);

	void StartFire();
	void StopFire();

	/**
	*   interaction
	*/

	void PerformInteractionCheck();

	void CouldntFindInteractable();

	void FoundNewInteractable(UInteractionComponent* Interactable);

	// returns true if we're interacting with an item that has an interaction time (e.g., a fireplace that takes 2 seconds to light)
	bool IsInteracting() const;

	UFUNCTION(BlueprintCallable)
	void BeginInteract();

	UFUNCTION(BlueprintCallable)
	void EndInteract();

	void Interact();

	// gets the time left until we complete/execute interacting with the current interactable
	float GetRemainingInteractTime();

	// helper function to make grabbing interactable faster
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class UInteractionComponent* GetInteractable() const { return InteractionData.ViewedInteractionComponent; }

	/**
	*   items / inventory related
	*/

	// use an item from our inventory
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UItem* Item);

	// handle equipping an equippable item
	bool EquipItem(class UEquippableItem* Item);
	bool UnequipItem(class UEquippableItem* Item);

	// should never be called directly; UAccessoryItem calls this on top of EquipItem
	void EquipAccessory(class UAccessoryItem* Accessory);
	void UnequipAccessory();

	// should never be called directly; UWeaponItem calls this on top of EquipItem
	void EquipWeapon(class UWeaponItem* WeaponItem);
	void UnequipWeapon();

	// helper function; expose otherwise protected EquippedItems
	UFUNCTION(BlueprintPure)
	FORCEINLINE TMap<EEquippableSlot, UEquippableItem*> GetEquippedItems() const { return EquippedItems; }

	UFUNCTION(BlueprintCallable, Category = "Weapons")
	FORCEINLINE class AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	/*
	*  weapon related
	*/

	UFUNCTION(BlueprintPure, Category = "Weapons")
	FORCEINLINE bool CanAim() const { return EquippedWeapon != nullptr && IsInteracting() == false && bPushing == false && bInCinematic == false && bAlive; }

	UFUNCTION(BlueprintPure, Category = "Weapons")
	FORCEINLINE bool IsAiming() const { return (CanAim() == true && bWantsToAim == true); }

	// manual reload
	void ReloadWeapon();

	/** applies recoil to the camera
	@param RecoilAmount the amount to recoil by. X = yaw, Y = pitch
	@param RecoilSpeed the speed to bump the camera up per second from the recoil
	@param RecoilResetSpeed the speed the camera will return to center at per second after the recoil is finished
	@param Shake an optional camera shake to play with the recoil*/
	void ApplyRecoil(const FVector2D& RecoilAmount, const float RecoilSpeed, const float RecoilResetSpeed);
	
	// update crosshair spread dependent on factors
	void CalculateCrosshairSpread(float DeltaTime);

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE void SetFocusedAim() { bFocusedAim = true; }

	UFUNCTION(BlueprintCallable)
	void ResetFocusedAim();

	void StartCrosshairBulletFire();

	FORCEINLINE void FinishCrosshairBulletFire() { bFiringBullet = false; }

	void ShowAmmoCounter();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowAmmoCounterBP();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowInvAmmoCounterImmediatelyBP();

	void HideAmmoCounterAfterDelay();

	UFUNCTION(BlueprintImplementableEvent)
	void HideAmmoCounterBP();

	UFUNCTION(BlueprintImplementableEvent)
	void HideAmmoCounterImmediatelyBP();

	UFUNCTION(BlueprintImplementableEvent)
	void HideInvAmmoCounterImmediatelyBP();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateAmmoCounterBP();

	UFUNCTION(BlueprintImplementableEvent)
	void DestroyHolsterAndPouchBP();

	/**
	*   combat related
	*/

	FORCEINLINE bool IsAlive() { return MovementStatus != EMovementStatus::EMS_Dead && !bAlive; }

	FORCEINLINE void ResetCanTakeDamage() { bCanTakeDamage = true; }

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AActor* Causer);

	UFUNCTION(BlueprintImplementableEvent)
	void OnDeathBP();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	UFUNCTION(BlueprintCallable)
	float ModifyHealth(const float Delta);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateBloodScreenBP();

	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);

	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	float PlayDeathMontage();

	bool IsClearFront();

	bool IsClearBehind();

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnPlayerBloodPoolBP();

	EHitDirection GetHitDirection();

	FPlayerHitReact GetPlayerHitReactToPlay();

	UFUNCTION(BlueprintImplementableEvent)
	void StopReloadAudioBP();

	UFUNCTION(BlueprintImplementableEvent)
	void DetachPistolFromHandBP();

	UFUNCTION(BlueprintImplementableEvent)
	void TurnOffFlashlightOnUnequipBP();

	UFUNCTION(BlueprintImplementableEvent)
	void ReportAINoiseEventBP(float Loudness, FVector Origin, FName Tag);

};