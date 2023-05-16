// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Weapons/Weapon.h"
#include "../DebugMacros.h"
#include "../Components/InventoryComponent.h"
#include "../Enemies/Enemy.h"
#include "../Items/EquippableItem.h"
#include "../Items/AmmoItem.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../PlayerCharacter/PlayerCharacterController.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h "
#include "Components/PawnNoiseEmitterComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveVector.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

// sets default values
AWeapon::AWeapon()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->bReceivesDecals = false;
	WeaponMesh->SetCollisionObjectType(ECC_WorldDynamic);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;

	bLoopedFireAnim = false;
	bLoopedFireSound = false;
	bLoopedMuzzleFX = false;
	bPlayingFireAnim = false;
	bIsEquipped = false;
	bWantsToFire = false;
	bPendingReload = false;
	bPendingEquip = false;
	bHasBeenFired = false;
	bHavePlayedOutOfAmmoSound = false;
	CurrentState = EWeaponState::Idle;
	AttachSocket = FName("Pistol_Socket");

	CurrentAmmoInClip = 0;
	BurstCounter = 0;
	LastFireTime = 0.0f;

	ADSTime = 0.5f;
	RecoilResetSpeed = 5.f;
	RecoilSpeed = 10.f;

	BulletCasingSoundDelay = 1.f;
	BulletCasingVolumeMultiplier = .75f;
	BulletHoleSize = FVector(3.f, 3.f, 3.f);
	BloodBulletHoleSize = FVector(1.5f, 1.5f, 1.5f);
	BulletHoleLifespan = 300.f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PrePhysics;
}


// called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	PawnOwner = Cast<APlayerCharacter>(GetOwner());
}


void AWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// ensure nothing attached
	DetachMeshFromPawn();
}


// stop particle FX / etc on destruction
void AWeapon::Destroyed()
{
	Super::Destroyed();

	StopSimulatingWeaponFire();
}


void AWeapon::UseClipAmmo()
{
	--CurrentAmmoInClip;
}


// consume ammo from player's inventory (used by reload function)
void AWeapon::ConsumeAmmo(const int32 Amount)
{
	if (PawnOwner)
	{
		if (UInventoryComponent* Inventory = PawnOwner->PlayerInventory)
		{
			if (UItem* AmmoItem = Inventory->FindItemByClass(WeaponConfig.AmmoClass))
			{ Inventory->ConsumeItem(AmmoItem, Amount); }
		}
	}
}

// when weapon in unequipped, try return ammo in mag to their inventory
void AWeapon::ReturnAmmoToInventory()
{
	if (PawnOwner && CurrentAmmoInClip > 0)
	{
		if (UInventoryComponent* Inventory = PawnOwner->PlayerInventory)
		{
			Inventory->TryAddItemFromClass(WeaponConfig.AmmoClass, CurrentAmmoInClip);
			CurrentAmmoInClip = 0;
		}
	}
}


// attach mesh, set flags, update state + play anim and call OnEquipFinished() when done
void AWeapon::OnEquip()
{
	// attach weapon to Holster_Socket and ensure holster/etc visible on player
	AttachMeshToPawn();
	PawnOwner->HolsterMesh->SetHiddenInGame(false);
	PawnOwner->BeltPouchMesh->SetHiddenInGame(false);

	bPendingEquip = true;
	DetermineWeaponState();

	if (PawnOwner)
	{	
		if (EquipAnim)
		{
			float AnimDuration = PlayPlayerWeaponAnimation(EquipAnim, 1.f);
			if (AnimDuration <= 0.0f) { AnimDuration = .5f; }

			GetWorldTimerManager().SetTimer(TimerHandle_OnEquipFinished, this, &AWeapon::OnEquipFinished, AnimDuration, false);
		}

		else { OnEquipFinished(); }


		WeaponMesh->AttachToComponent(PawnOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachSocket);
		PawnOwner->HolsteredPistolMesh->SetHiddenInGame(true);
		OnEquipFinished();

		// show ammo counter briefly
		PawnOwner->ShowAmmoCounter();
		PawnOwner->HideAmmoCounterAfterDelay();
	}
}


// set flags, update state + reload if needed
void AWeapon::OnEquipFinished()
{
	bIsEquipped = true;
	bPendingEquip = false;

	PawnOwner->bHasWeaponEquipped = true;

	// determine state so CanReload() checks work
	DetermineWeaponState();

	if (PawnOwner)
	{
		// try to reload an empty clip
		if (CurrentAmmoInClip <= 0 && CanReload())
		{ StartReload(); }
	}
}

// detach mesh, stop firing, cleanup
void AWeapon::OnUnequip()
{
	DetachMeshFromPawn();
	bIsEquipped = false;
	bHasBeenFired = false;
	StopFire();

	if (bPendingReload)
	{
		StopPlayerWeaponAnimation(LoweredReloadAnim);
		StopPlayerWeaponAnimation(AimingReloadAnim);
		bPendingReload = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_StopReload);
		GetWorldTimerManager().ClearTimer(TimerHandle_ReloadWeapon);
	}

	if (bPendingEquip)
	{
		StopPlayerWeaponAnimation(EquipAnim);
		bPendingEquip = false;

		GetWorldTimerManager().ClearTimer(TimerHandle_OnEquipFinished);
	}

	if (UnequipAnim)
	{
		float AnimDuration = .5f;

		if (!bDiscarding)
		{
			AnimDuration = PlayPlayerWeaponAnimation(UnequipAnim, 1.f);
			if (AnimDuration <= 0.0f) { AnimDuration = .5f; }
		}

		GetWorldTimerManager().SetTimer(TimerHandle_OnUnequipFinished, this, &AWeapon::OnUnequipFinished, AnimDuration, false);
	}

	PawnOwner->DetachPistolFromHandBP();
	OnUnequipFinished();

	ReturnAmmoToInventory();
	DetermineWeaponState();
}


void AWeapon::OnUnequipFinished()
{
	PawnOwner->bHasWeaponEquipped = false;
	PawnOwner->EquippedWeapon = nullptr;
	DetermineWeaponState();
	PawnOwner->BeltPouchMesh->SetHiddenInGame(true);
	PawnOwner->HolsteredPistolMesh->SetHiddenInGame(true);
}


bool AWeapon::IsEquipped() const
{
	return bIsEquipped;
}


bool AWeapon::IsAttachedToPlayer() const
{
	return bIsEquipped || bPendingEquip;
}


/**
*  weapon input
*/


// entry point for firing. set flags, update state
void AWeapon::StartFire()
{
	if (!bWantsToFire)
	{
		bWantsToFire = true;
		DetermineWeaponState();
	}
}

// exit point for firing. set flags, update state
void AWeapon::StopFire()
{
	if (bWantsToFire)
	{
		bWantsToFire = false;
		DetermineWeaponState();
	}
}

// validate, set flags, update state, play anim, set timers + play sound
void AWeapon::StartReload()
{
	if (CanReload())
	{
		bPendingReload = true;
		DetermineWeaponState();

		// update HUD widget
		PawnOwner->ShowAmmoCounter();

		UAnimMontage* AnimToPlay = PawnOwner->IsAiming() ? AimingReloadAnim : LoweredReloadAnim;
		float AnimDuration = PlayPlayerWeaponAnimation(AnimToPlay, 0.85f);
		if (AnimDuration <= 0.0f) { AnimDuration = .5f; }

		GetWorldTimerManager().SetTimer(TimerHandle_StopReload, this, &AWeapon::StopReload, AnimDuration, false);
		GetWorldTimerManager().SetTimer(TimerHandle_ReloadWeapon, this, &AWeapon::ReloadWeapon, FMath::Max(0.1f, AnimDuration - 0.1f), false);
	}
}


// interrupts reload
void AWeapon::StopReload()
{
	if (CurrentState == EWeaponState::Reloading)
	{
		bPendingReload = false;
		DetermineWeaponState();
		StopPlayerWeaponAnimation(LoweredReloadAnim);
		StopPlayerWeaponAnimation(AimingReloadAnim);
		PawnOwner->StopReloadAudioBP();
	}
}


void AWeapon::ReloadWeapon()
{
	// how much space is remaining in magazine
	int32 ClipDelta = FMath::Min(WeaponConfig.AmmoPerClip - CurrentAmmoInClip, GetCurrentAmmoInInventory());

	// fill remainder from inventory ammo supply
	if (ClipDelta > 0)
	{
		CurrentAmmoInClip += ClipDelta;
		ConsumeAmmo(ClipDelta);

		// update ammo counter HUD + hide after delay if not aiming
		PawnOwner->UpdateAmmoCounterBP();
		PawnOwner->HideAmmoCounterAfterDelay();

		// reset flag
		bHavePlayedOutOfAmmoSound = false;
	}

	else
	{ UE_LOG(LogTemp, Warning, TEXT("Not enough ammo for a reload.")); }
}


// validate firing conditions
bool AWeapon::CanFire() const
{
	bool bCanFire = PawnOwner != nullptr && PawnOwner->bAlive;
	bool bStateOKToFire = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Empty) || (CurrentState == EWeaponState::Firing));
	
	return ((bCanFire == true) && (bStateOKToFire == true) && (bPendingReload == false));
}


// validate reload conditions
bool AWeapon::CanReload() const
{
	bool bCanReload = PawnOwner != nullptr && IsEquipped() && PawnOwner->bAlive;
	bool bHaveAmmo = (CurrentAmmoInClip < WeaponConfig.AmmoPerClip) && (GetCurrentAmmoInInventory() > 0);
	bool bStateOKToReload = ((CurrentState == EWeaponState::Idle) || (CurrentState == EWeaponState::Empty) || (CurrentState == EWeaponState::Firing));

	return ((bCanReload == true) && (bHaveAmmo == true) && (bStateOKToReload == true));
}


// how much ammo for this weapon remains in player's inventory?
int32 AWeapon::GetCurrentAmmoInInventory() const
{
	if (PawnOwner)
	{
		if (UInventoryComponent* Inventory = PawnOwner->PlayerInventory)
		{
			if (UItem* Ammo = Inventory->FindItemByClass(WeaponConfig.AmmoClass))
			{ return Ammo->GetQuantity(); }
		}
	}
	
	return 0;
}


/**
*  weapon usage
*/


// turn on various firing FX
void AWeapon::SimulateWeaponFire()
{
	if (CurrentState != EWeaponState::Firing) { return; }

	// spawn particle FX
	if (MuzzleFX)
	{
		if (!bLoopedMuzzleFX || MuzzlePSC == NULL)
		{
			if (APlayerCharacterController* PC = Cast<APlayerCharacterController>(PawnOwner->GetController()))
			{ MuzzlePSC = UGameplayStatics::SpawnEmitterAttached(MuzzleFX, WeaponMesh, MuzzleAttachPoint); }
		}
	}

	if (BulletEjectionFX)
	{
		//UNiagaraComponent* BulletEjectComp = UNiagaraFunctionLibrary::SpawnSystemAttached(BulletEjectionFX, WeaponMesh, MuzzleAttachPoint, FVector(0.f), FRotator(0.f), EAttachLocation::KeepRelativeOffset, true);
		FVector EjectLoc = WeaponMesh->GetSocketLocation(MuzzleAttachPoint);
		UNiagaraComponent* BulletEjectComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BulletEjectionFX, EjectLoc);

		if (BulletCasingLandingSound)
		{ GetWorldTimerManager().SetTimer(TimerHandle_BulletCasingLandingSound, this, &AWeapon::PlayBulletCasingLandingSound, BulletCasingSoundDelay); }
	}

	// play firing anim
	if (!bLoopedFireAnim || !bPlayingFireAnim)
	{
		PlayPlayerWeaponAnimation(FireAnim, 1.f);
		WeaponMesh->PlayAnimation(WeaponFireAnim, false);
		bPlayingFireAnim = true;
	}

	// play firing sound
	if (bLoopedFireSound)
	{
		if (FireAC == NULL)
		{ FireAC = PlayWeaponSound(FireLoopSound); }
	}

	else
	{ PlayWeaponSound(FireSound); }

	// apply recoil / play controller vibration
	if (APlayerCharacterController* PC = Cast<APlayerCharacterController>(PawnOwner->GetController()))
	{
		if (RecoilCurve)
		{
			const FVector2D RecoilAmount(RecoilCurve->GetVectorValue(FMath::RandRange(0.f, 1.f)).X, RecoilCurve->GetVectorValue(FMath::RandRange(0.f, 1.f)).Y);
			PawnOwner->ApplyRecoil(RecoilAmount, RecoilSpeed, RecoilResetSpeed);
		}

		if (FireForceFeedback)
		{
			FForceFeedbackParameters FFParams;
			FFParams.Tag = "Weapon";
			PC->ClientPlayForceFeedback(FireForceFeedback, FFParams);
		}
	}
}


// turn off all firing FX
void AWeapon::StopSimulatingWeaponFire()
{
	// stop muzzle particle FX
	if (bLoopedMuzzleFX)
	{
		if (MuzzlePSC)
		{
			MuzzlePSC->DeactivateSystem();
			MuzzlePSC = NULL;
		}
	}

	// stop firing anim
	if (bLoopedFireAnim && bPlayingFireAnim)
	{
		StopPlayerWeaponAnimation(FireAnim);
		bPlayingFireAnim = false;
	}

	// stop sound FX
	if (FireAC)
	{
		FireAC->FadeOut(0.1f, 0.0f);
		FireAC = NULL;

		PlayWeaponSound(FireFinishSound);
	}
}


// process the hit
void AWeapon::HandleHit(const FHitResult& Hit, class AEnemy* HitEnemy /*= nullptr*/)
{
	if (PawnOwner)
	{
		float DamageMultiplier = 1.f;

		// apply any bone-specific damage modifiers
		for (auto& BoneDamageModifier : HitScanConfig.BoneDamageModifiers)
		{
			if (Hit.BoneName == BoneDamageModifier.Key)
			{
				DamageMultiplier = BoneDamageModifier.Value;
				break;
			}
		}

		// apply damage to enemy
		if (HitEnemy)
		{
			// store which bone was hit and the direction hit came from
			HitEnemy->LastHitResult = Hit;
			HitEnemy->LastBoneHit = Hit.BoneName;
			HitEnemy->LastHitImpactPoint = Hit.ImpactPoint;
			HitEnemy->LastHitPlayerLocation = PawnOwner->GetActorLocation();
	
			UGameplayStatics::ApplyPointDamage(HitEnemy, HitScanConfig.Damage * DamageMultiplier, (Hit.TraceStart - Hit.TraceEnd).GetSafeNormal(), Hit, PawnOwner->GetController(), PawnOwner, HitScanConfig.DamageType);
		
			// spawn blood splash impact particle FX
			HitEnemy->PlayBloodHitFX(Hit);

			// spawn bullet hole decal
			FRotator RandomDecalRotation = Hit.ImpactNormal.Rotation();
			RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);
			UDecalComponent* BloodBulletHoleDecalComp = UGameplayStatics::SpawnDecalAttached(BloodBulletHoleDecal, BloodBulletHoleSize, Hit.GetComponent(), Hit.BoneName, Hit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition);
			BloodBulletHoleDecalComp->SetFadeScreenSize(0.f);
		}
	}
}


// weapon-specific fire implementation
void AWeapon::FireShot()
{
	if (PawnOwner)
	{
		//PawnOwner->MakeNoise(1.f, PawnOwner, GetActorLocation());
		PawnOwner->ReportAINoiseEventBP(1.f, GetActorLocation(), FName("WeaponFiring"));
		
		const FTransform MuzzleTransform = WeaponMesh->GetSocketTransform(MuzzleAttachPoint);

		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			// get current viewport size, store in ViewportSize
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		// get screen space location of crosshairs
		FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		CrosshairLocation.Y -= 150.f; // adjust up by 150 units (also done in BP_HUD)

		// get world position + direction of crosshairs
		FVector CrosshairWorldPosition;
		FVector CrosshairWorldDirection;

		bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), 
		CrosshairLocation,CrosshairWorldPosition, CrosshairWorldDirection);

		// if deprojection was successful, perform line trace
		if (bScreenToWorld)
		{
			// setup line trace
			FHitResult ScreenTraceHit;
			const FVector TraceStart = CrosshairWorldPosition;
			const FVector TraceEnd = CrosshairWorldPosition + (CrosshairWorldDirection * HitScanConfig.Distance);
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(this);
			QueryParams.AddIgnoredActor(PawnOwner);
			QueryParams.bReturnPhysicalMaterial = true;
			QueryParams.bTraceComplex = true;

			// for smoke trail - set beam end point to line trace end point
			FVector BeamEndPoint = TraceEnd;
			FHitResult FinalBlockingHit;

			// line trace outwards from crosshairs world location
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
			//DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 2.f);

			// check hit result of trace for blocking hit
			if (ScreenTraceHit.bBlockingHit)
			{
				// smoke trail end point is now updated to trace hit location
				BeamEndPoint = ScreenTraceHit.Location;
				// the hit result that will ultimately be passed
				FinalBlockingHit = ScreenTraceHit;
				
				PawnOwner->ReportAINoiseEventBP(1.f, FinalBlockingHit.Location, FName("BulletImpact"));
		
				if (AEnemy* HitEnemy = Cast<AEnemy>(FinalBlockingHit.GetActor()))
				{ HandleHit(FinalBlockingHit, HitEnemy); }

				// don't call impact particle FX if hit enemy (different FX, handled on enemy's damage application)
				else if (ImpactParticles && BulletHoleDecal)
				{
					// spawn impact particle FX
					UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactParticles, FinalBlockingHit.Location);
					
					// spawn bullet hole decal
					FRotator RandomDecalRotation = FinalBlockingHit.ImpactNormal.Rotation();
					RandomDecalRotation.Roll = FMath::FRandRange(-180.0f, 180.0f);
					UDecalComponent* BulletHoleDecalComp = UGameplayStatics::SpawnDecalAttached(BulletHoleDecal, BulletHoleSize, FinalBlockingHit.GetComponent(), FinalBlockingHit.BoneName, FinalBlockingHit.ImpactPoint, RandomDecalRotation, EAttachLocation::KeepWorldPosition);
					BulletHoleDecalComp->SetFadeScreenSize(0.f);
				}
			}
		}

		// start bullet fire timer for crosshair adjustment
		PawnOwner->StartCrosshairBulletFire();
	}
}


// handle weapon re-fire
void AWeapon::HandleRefiring()
{
	UWorld* World = GetWorld();
	float SlackTimeThisFrame = FMath::Max(0.0f, (World->TimeSeconds - LastFireTime) - WeaponConfig.TimeBetweenShots);

	// compensate timer to maintain consistent rate of fire
	if (bAllowAutomaticWeaponCatchup)
	{ TimerIntervalAdjustment -= SlackTimeThisFrame; }

	HandleFiring();
}


// handle weapon firing
void AWeapon::HandleFiring()
{
	// does player have enough ammo to fire?
	if ((CurrentAmmoInClip > 0) && CanFire())
	{
		SimulateWeaponFire();

		// fire
		if (PawnOwner)
		{
			FireShot();
			BurstCounter++;
		}
	}

	// if not, reload instead if able
	else if (CanReload())
	{ StartReload(); }

	// not enough ammo in inventory to reload weapon
	else
	{
		//if (GetCurrentAmmoInInventory() == 0 && !bRefiring && !bHavePlayedOutOfAmmoSound && !bHasBeenFired)
		if (GetCurrentAmmoInInventory() == 0 && !bRefiring)
		{
			PlayWeaponSound(OutOfAmmoSound);
			bHavePlayedOutOfAmmoSound = true;
		}

		// stop weapon fire FX, but stay in Firing state
		if (BurstCounter > 0)
		{ OnBurstFinished(); }
	}

	if (PawnOwner)
	{
		const bool bShouldUpdateAmmo = (CurrentAmmoInClip > 0 && CanFire());

		// consume ammo from magazine
		if (bShouldUpdateAmmo)
		{
			UseClipAmmo();
			BurstCounter++;

			PawnOwner->UpdateAmmoCounterBP();
		}

		// reload after firing last round in magazine
		if (CurrentAmmoInClip <= 0)
		{
			WeaponMesh->PlayAnimation(WeaponEmptyAnim, true);
			SetWeaponState(EWeaponState::Empty);

			if (CanReload())
			{ StartReload(); }
		}

		// setup re-firing timer
		bRefiring = (CurrentState == EWeaponState::Firing && WeaponConfig.TimeBetweenShots > 0.0f);
		if (bRefiring)
		{
			GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleRefiring, FMath::Max<float>(WeaponConfig.TimeBetweenShots + TimerIntervalAdjustment, SMALL_NUMBER), false);
			TimerIntervalAdjustment = 0.f;
		}
	}

	// record time of last firing
	LastFireTime = GetWorld()->GetTimeSeconds();
	bHasBeenFired = true;
}


// firing started
void AWeapon::OnBurstStarted()
{
	// start firing; can be delayed to satisfy TimeBetweenShots
	const float GameTime = GetWorld()->GetTimeSeconds();
	if (LastFireTime > 0 && WeaponConfig.TimeBetweenShots > 0.0f && LastFireTime + WeaponConfig.TimeBetweenShots > GameTime)
	{ GetWorldTimerManager().SetTimer(TimerHandle_HandleFiring, this, &AWeapon::HandleFiring, LastFireTime + WeaponConfig.TimeBetweenShots - GameTime, false); }

	else
	{ HandleFiring(); }
}


// firing finished
void AWeapon::OnBurstFinished()
{
	// stop firing FX
	BurstCounter = 0;
	StopSimulatingWeaponFire();

	GetWorldTimerManager().ClearTimer(TimerHandle_HandleFiring);
	bRefiring = false;

	// reset firing interval adjustment
	TimerIntervalAdjustment = 0.0f;
}


// update weapon state
void AWeapon::SetWeaponState(EWeaponState NewState)
{
	const EWeaponState PrevState = CurrentState;

	if (PrevState == EWeaponState::Firing && NewState != EWeaponState::Firing)
	{ OnBurstFinished(); }

	CurrentState = NewState;

	if (PrevState != EWeaponState::Firing && NewState == EWeaponState::Firing)
	{ OnBurstStarted(); }
}


// determine current weapon state
void AWeapon::DetermineWeaponState()
{
	// set default state before performing checks
	EWeaponState NewState = EWeaponState::Idle;

	// determine if reloading, firing, or equipping
	if (bIsEquipped)
	{
		if (bPendingReload)
		{
			if (CanReload() == false) { NewState = CurrentState; }
			else { NewState = EWeaponState::Reloading; }
		}

		else if ((!bPendingReload) && (bWantsToFire) && (CanFire() == true))
		{ NewState = EWeaponState::Firing; }
	}

	else if (bPendingEquip) { NewState = EWeaponState::Equipping; }

	// set state + play any relevant looped idle anims
	SetWeaponState(NewState);

	if (NewState == EWeaponState::Idle && CurrentAmmoInClip == 0 && bHasBeenFired && WeaponEmptyAnim)
	{ WeaponMesh->PlayAnimation(WeaponEmptyAnim, true); }

	else if (NewState == EWeaponState::Idle)
	{ WeaponMesh->PlayAnimation(WeaponIdleAnim, true); }
}


// attaches weapon mesh to player pawn's mesh
void AWeapon::AttachMeshToPawn()
{
	if (PawnOwner)
	{
		// remove mesh if present
		DetachMeshFromPawn();

		USkeletalMeshComponent* PawnMesh = PawnOwner->GetMesh();
		AttachToComponent(PawnOwner->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, HolsterSocket);
	}
}


// detaches weapon mesh from player pawn's mesh
void AWeapon::DetachMeshFromPawn()
{
	// holster? maybe, TBD
}


/**
*  weapon usage helpers
*/


// play weapon sounds
UAudioComponent* AWeapon::PlayWeaponSound(USoundCue* Sound)
{
	UAudioComponent* AC = NULL;
	if (Sound && PawnOwner)
	{ AC = UGameplayStatics::SpawnSoundAttached(Sound, PawnOwner->GetRootComponent()); }

	return AC;
}


// play weapon anims
float AWeapon::PlayPlayerWeaponAnimation(UAnimMontage* Animation, float PlayRate)
{
	float Duration = 0.0f;

	if (PawnOwner && Animation)
	{ Duration = PawnOwner->PlayAnimMontage(Animation, PlayRate); }

	return Duration;
}


// stop playing weapon anims
void AWeapon::StopPlayerWeaponAnimation(UAnimMontage* Animation)
{
	if (PawnOwner && Animation)
	{ PawnOwner->StopAnimMontage(Animation); }
}


void AWeapon::PlayBulletCasingLandingSound()
{
	if (BulletCasingLandingSound)
	{ UGameplayStatics::PlaySoundAtLocation(GetWorld(), BulletCasingLandingSound, PawnOwner->GetActorLocation(), BulletCasingVolumeMultiplier); }
}


void AWeapon::MakeNoiseAtLastBulletImpactLocation()
{
	PawnOwner->MakeNoise(1.f, PawnOwner, LastBulletImpactLocation);
}