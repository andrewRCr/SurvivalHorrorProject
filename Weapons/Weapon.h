// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UAnimMontage;
class APlayerCharacter;
class UAudioComponent;
class UParticleSystemComponent;
class UCameraShake;
class UForceFeedbackEffect;
class USoundCue;
class UNiagaraSystem;


UENUM(BlueprintType)
enum class EWeaponState : uint8 
{
	Idle,
	Empty,
	Firing,
	Reloading,
	Equipping
};


USTRUCT(BlueprintType)
struct FWeaponData
{
	GENERATED_USTRUCT_BODY()

	// magazine size
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	int32 AmmoPerClip;

	// the item that this weapon uses for ammo
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ammo")
	TSubclassOf<class UAmmoItem> AmmoClass;

	// time between two consecutive shots
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WeaponStats")
	float TimeBetweenShots;

	// defaults
	FWeaponData()
	{
		AmmoPerClip = 10;
		TimeBetweenShots = 0.5f;
	}

};


USTRUCT(BlueprintType)
struct FHitScanConfiguration
{
	GENERATED_BODY()

	// defaults
	FHitScanConfiguration()
	{
		Distance = 10000.f;
		Damage = 25.f;
		Radius = 0.f;
		DamageType = UDamageType::StaticClass();
	}

	// a map of bone -> damage amount
	UPROPERTY(EditDefaultsOnly, Category = "TraceInfo")
	TMap<FName, float> BoneDamageModifiers;

	// how far the hitscan traces for a hit
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TraceInfo")
	float Distance;

	// the amount of damage to deal when hit by the hitscan
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TraceInfo")
	float Damage;

	// optional trace radius. a value of zero = line trace; anything higher = sphere trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "TraceInfo")
	float Radius;

	// type of damage dealt
	UPROPERTY(EditDefaultsOnly, Category = "WeaponStats")
	TSubclassOf<UDamageType> DamageType;

};


UCLASS()
class ESCAPEROOMPROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()

	friend class APlayerCharacter;
	
public:	
	// sets default values for this actor's properties
	AWeapon();

	// weapon mesh
	UPROPERTY(EditAnywhere, Category = "Components")
	USkeletalMeshComponent* WeaponMesh;

protected:

	// the weapon item in the player's inventory
	UPROPERTY(BlueprintReadOnly)
	class UWeaponItem* WeaponItem;

	// the player pawn owner
	UPROPERTY()
	class APlayerCharacter* PawnOwner;

	// weapon data
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FWeaponData WeaponConfig;

	// line trace data
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	FHitScanConfiguration HitScanConfig;

	// firing audio
	UPROPERTY(Transient)
	UAudioComponent* FireAC;

	// name of bone/socket for muzzle in weapon mesh
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	FName MuzzleAttachPoint;

	// name of socket to attach to the character on, hand
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	FName AttachSocket;

	// name of socket to attach to the character on, holster
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	FName HolsterSocket;

	// muzzle flash FX
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* MuzzleFX;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UNiagaraSystem* BulletEjectionFX;

	// FX for impact particles (spawned on bullet impact)
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UNiagaraSystem* ImpactParticles;

	// FX for beam particles (smoke trail for bullets)
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* BeamParticles;

	// spawned component for muzzle FX
	UPROPERTY(Transient)
	UParticleSystemComponent* MuzzlePSC;

	// time required to aim down sights, in seconds
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	float ADSTime;

	// amount of recoil to apply
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	class UCurveVector* RecoilCurve;

	// speed at which the recoil bumps up, per second
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoilSpeed;

	// speed at which the recoil resets, per second
	UPROPERTY(EditDefaultsOnly, Category = "Recoil")
	float RecoilResetSpeed;

	// force feedback effect (controller vibration) to play when the weapon is fired
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UForceFeedbackEffect* FireForceFeedback;

	// how long after firing to wait until playing the sound of the casing hitting the ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	float BulletCasingSoundDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	float BulletCasingVolumeMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UMaterialInstance* BulletHoleDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UMaterialInstance* BloodBulletHoleDecal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	FVector BulletHoleSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	FVector BloodBulletHoleSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	float BulletHoleLifespan;
	
	/**
	*  sound and animation
	*/

	// single shot fired sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* FireSound;

	// looped fire sound (bLoopedFireSound set)
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* FireLoopSound;

	// finished firing sound (bLoopedFireSound set)
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* FireFinishSound;

	// out of ammo sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* OutOfAmmoSound;

	// reload sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* ReloadSound;

	// sound of spent bullet casing hitting the ground
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* BulletCasingLandingSound;

	// reload animation: player making reload motions with gun lowered
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* LoweredReloadAnim;

	// reload animation: player making reload motions with gun raised
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AimingReloadAnim;

	// reload animation: weapon itself being reloaded
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* WeaponReloadAnim;

	// equip sound
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	USoundCue* EquipSound;

	// equip anim (player drawing from holster)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* EquipAnim;

	// unequip anim (player holstering weapon)
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* UnequipAnim;

	// firing anim: player character pulling trigger
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* FireAnim;

	// firing anim: weapon itself firing
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimationAsset* WeaponFireAnim;

	// empty anim: weapon itself out of ammo
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimationAsset* WeaponEmptyAnim;

	// idle anim: weapon itself, neither firing, empty, nor reloading
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimationAsset* WeaponIdleAnim;

	/**
	*  flags
	*/

	// has the weapon been fired since equipping?
	bool bHasBeenFired;

	// has this sound been played once?
	bool bHavePlayedOutOfAmmoSound;

	// is fire anim playing?
	uint32 bPlayingFireAnim : 1;

	// is fire sound looped?
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	uint32 bLoopedFireSound : 1;

	// is fire animation looped?
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	uint32 bLoopedFireAnim : 1;

	// are muzzle FX looped?
	UPROPERTY(EditDefaultsOnly, Category = "FX")
	uint32 bLoopedMuzzleFX : 1;

	// is weapon currently equipped?
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	uint32 bIsEquipped : 1;

	// is weapon fire active?
	uint32 bWantsToFire : 1;

	// is reload anim playing?
	UPROPERTY(BlueprintReadOnly, Transient)
	uint32 bPendingReload : 1;

	// is equip anim playing?
	UPROPERTY(BlueprintReadOnly, Transient)
	uint32 bPendingEquip : 1;

	// weapon is re-firing
	uint32 bRefiring;

	// weapon is being discarded
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDiscarding;

	// adjustment to handle frame rate affecting actual timer interval
	UPROPERTY(Transient)
	float TimerIntervalAdjustment;

	// whether to allow automatic weapons to catch up with shorter re-fire cycles
	UPROPERTY(Config)
	bool bAllowAutomaticWeaponCatchup = true;

	// current weapon state
	EWeaponState CurrentState;

	// time of last successful weapon firing
	float LastFireTime;

	// last time this weapon was switched to / equipped
	float EquipStartedTime;

	// how much time weapon needs to be equipped
	float EquipDuration;

	// current ammo inside of magazine
	UPROPERTY(Transient, EditAnywhere, BlueprintReadWrite)
	int32 CurrentAmmoInClip;

	// burst counter for tracking fire events
	UPROPERTY(Transient)
	int32 BurstCounter;

	// handle for efficient management of OnEquipFinished timer
	FTimerHandle TimerHandle_OnEquipFinished;

	// handle for efficient management of OnEquipFinished timer
	FTimerHandle TimerHandle_OnUnequipFinished;

	// handle for efficient management of StopReload timer
	FTimerHandle TimerHandle_StopReload;

	// handle for efficient management of ReloadWeapon timer
	FTimerHandle TimerHandle_ReloadWeapon;

	// handle for efficient management of HandleFiring timer
	FTimerHandle TimerHandle_HandleFiring;

	// handle for efficient management of BulletCasingLandingSound timer
	FTimerHandle TimerHandle_BulletCasingLandingSound;

	FTimerHandle TimerHandle_BulletImpactSoundForAIHearing;

	FVector LastBulletImpactLocation;

public:

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;
	virtual void Destroyed() override;

protected:
	
	// consume a bullet from the magazine
	void UseClipAmmo();

	// consume ammo from the inventory
	void ConsumeAmmo(const int32 Amount);

	// return ammo to the inventory when weapon is unequipped
	UFUNCTION(BlueprintCallable)
	void ReturnAmmoToInventory();

	// weapon is being equipped by player
	virtual void OnEquip();

	// weapon is now equipped by player
	virtual void OnEquipFinished();

	// weapon is being unequipped by player
	virtual void OnUnequip();

	// weapon is no longer equipped by player
	virtual void OnUnequipFinished();

	// is weapon currently equipped?
	bool IsEquipped() const;

	// is weapon mesh already attached?
	bool IsAttachedToPlayer() const;

	/**
	*  weapon input
	*/

	// begin weapon firing
	virtual void StartFire();

	// stop weapon firing
	virtual void StopFire();

	// begins weapon reload
	virtual void StartReload();

	// interrupts weapon reload
	virtual void StopReload();

	// performs actual reload
	virtual void ReloadWeapon();

	// validates - have ammo, not currently reloading, etc
	bool CanFire() const;
	bool CanReload() const;

	// get idle, firing, reloading, etc
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE EWeaponState GetCurrentState() const { return CurrentState; }

	// get current ammo amount (total)
	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentAmmoInInventory() const;

	// get current ammo remaining in magazine
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE int32 GetCurrentAmmoInClip() const { return CurrentAmmoInClip; }

	// get magazine size
	FORCEINLINE int32 GetAmmoPerClip() const { return WeaponConfig.AmmoPerClip; }

	// get weapon mesh
	UFUNCTION(BlueprintPure, Category = "Weapon")
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }

	// get pawn owner
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	FORCEINLINE class APlayerCharacter* GetPawnOwner() const { return PawnOwner; }

	// gets last time this weapon was switched to
	FORCEINLINE float GetEquipStartedTime() const { return EquipStartedTime; }

	// gets the duration of weapon being equipped
	FORCEINLINE float GetEquipDuration() const { return EquipDuration; }

	/**
	*  weapon usage
	*/

	// start / stop cosmetic FX for firing
	virtual void SimulateWeaponFire();
	virtual void StopSimulatingWeaponFire();

	// process the hit
	void HandleHit(const FHitResult& Hit, class AEnemy* HitEnemy = nullptr);

	// weapon-specific fire implementation
	virtual void FireShot();

	// handle weapon re-fire
	void HandleRefiring();

	// handle weapon firing
	void HandleFiring();

	// firing started / finished
	virtual void OnBurstStarted();
	virtual void OnBurstFinished();

	// update weapon state
	void SetWeaponState(EWeaponState NewState);

	// determine current weapon state
	void DetermineWeaponState();

	// attaches weapon mesh to player pawn's mesh
	void AttachMeshToPawn();

	// detaches weapon mesh from player pawn's mesh
	void DetachMeshFromPawn();

	/**
	*  weapon usage helpers
	*/

	// play weapon sounds
	UAudioComponent* PlayWeaponSound(USoundCue* Sound);

	// play weapon animations on the player character
	float PlayPlayerWeaponAnimation(UAnimMontage* Animation, float PlayRate);

	// stop playing weapon animations on the player character
	void StopPlayerWeaponAnimation(UAnimMontage* Animation);

	// get the aim of the camera
	FVector GetCameraAim() const;

	void PlayBulletCasingLandingSound();

	void MakeNoiseAtLastBulletImpactLocation();
};
