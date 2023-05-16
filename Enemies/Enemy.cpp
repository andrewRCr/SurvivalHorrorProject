// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Enemies/Enemy.h"
#include "../Enemies/EnemyController.h"
#include "../PlayerCharacter/PlayerCharacter.h"
#include "../DebugMacros.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNotifies/AnimNotifyState_DisableRootMotion.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "NavigationSystem.h"
#include "Perception/PawnSensingComponent.h"
#include "Sound/SoundCue.h"


// sets default values
AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));

	// create and attach combat range sphere to root component
	CombatRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Combat Range Sphere"));
	CombatRangeSphere->SetupAttachment(GetRootComponent());
	CombatRangeSphere->InitSphereRadius(100.0);
	
	// set default enemy base stats
	MaxHealth = 150.f;
	Health = MaxHealth;
	Damage = 35.f;

	// set movement defaults
	PassiveWalkSpeed = 20.f;
	HostileWalkSpeed = 40.f;
	GetCharacterMovement()->MaxWalkSpeed = PassiveWalkSpeed;
	PassiveRotationRate = FRotator(0.f, 30.f, 0.f);
	HostileRotationRate = FRotator(0.f, 90.f, 0.f);
	IncapacitatedRotationRate = FRotator(0.f, 60.f, 0.f);
	GetCharacterMovement()->RotationRate = PassiveRotationRate;

	// set default awareness level and combat state
	AwarenessLevel = EEnemyAwarenessLevel::EAL_Passive;
	CombatState = EEnemyCombatState::ECS_Idle;

	// set default combat modifiers
	CombatRadius = 1000.f;
	AttackRange = 100.f;
	LungeAttackRange = 150.f;
	CrawlingAttackRange = 125.f;
	bAlive = true;
	bIsRagdoll = false;
	bShouldPlayPhysicalHitReact = false;
	bStaggered = false;
	bCanTakeDamage = true;
	bInAttackRange = false;
	bCanLookAtPlayer = true;
	TakeDamageDelay = 0.25f;
	AttackDelayMin = 1.f;
	AttackDelayMax = 2.f;

	// set default perception values
	PerceptionRange = 2000.f;
	VisionRange = 1500.f;
	VisionAggroRange = VisionRange;
	HearingAggroRange = PerceptionRange;
	PeripheralVisionAngle = 75.f;
	bCanSeePlayer = false;
	PlayerLOSCheckFrequency = 1.f;
	TargetLossDelay = 3.f;

	PawnSensingComp->SetPeripheralVisionAngle(PeripheralVisionAngle);
	PawnSensingComp->SightRadius = 2000.f;

	// navigation defaults
	DistanceToPlayerCharacter = 0.f;
	PatrolAcceptanceRadius = 150.f;
	PatrolIdleTimeMin = 10.f;
	PatrolIdleTimeMax = 15.f;
	PassiveRotateTowardsDelay = 2.f;
	HostileRotateTowardsDelay = 1.f;
	bIncapacitated = false;
	MoveToAcceptanceRadius = 15.f;
	CrawlingMoveToAcceptanceRadius = 45.f;

	// hit react defaults
	LeftLegHitCounter = 0;
	RightLegHitCounter = 0;
	LeftArmHitCounter = 0;
	RightArmHitCounter = 0;
	HeadHitCounter = 0;
	bEnableLeftArmProfile = false;
	bEnableRightArmProfile = false;
	bEnableLeftLegProfile = false;
	bEnableRightLegProfile = false;

	// audio defaults
	IdleCueIntervalMin = 5.f;
	IdleCueIntervalMax = 12.f;
	ChasingCueIntervalMin = 4.f;
	ChasingCueIntervalMax = 6.f;
	bCanPlayIdleSpeech = false;

}

// blueprint-callable setter for enemy awareness
// validates, updates, updates corresponding Blackboard key, and updates movement speed
void AEnemy::SetEnemyAwarenessLevel(EEnemyAwarenessLevel NewAwarenessLevel)
{
	// validate
	if (NewAwarenessLevel == AwarenessLevel) { return; }

	// update enemy's awareness level
	AwarenessLevel = NewAwarenessLevel;

	// set appropriate movement speed on character movement component
	switch (AwarenessLevel)
	{
		case EEnemyAwarenessLevel::EAL_Passive:
			// clear any pending chasing speech cues + set speed/rotation rate
			GetWorldTimerManager().ClearTimer(ChasingCue_TimerHandle);
			GetCharacterMovement()->MaxWalkSpeed = PassiveWalkSpeed;
			GetCharacterMovement()->RotationRate = PassiveRotationRate;
			break;

		case EEnemyAwarenessLevel::EAL_Hostile:
			// clear any pending idle speech cues + set speed/rotation rate
			GetWorldTimerManager().ClearTimer(IdleCue_TimerHandle);
			GetCharacterMovement()->MaxWalkSpeed = bIncapacitated ? PassiveWalkSpeed : HostileWalkSpeed;
			GetCharacterMovement()->RotationRate = bIncapacitated ? IncapacitatedRotationRate : HostileRotationRate;
			break;

		default:
			GetCharacterMovement()->MaxWalkSpeed = PassiveWalkSpeed;
			GetCharacterMovement()->RotationRate = PassiveRotationRate;
			break;
	}
}


// blueprint-callable setter for combat state; validates, updates, and updates corresponding Blackboard key
void AEnemy::SetEnemyCombatState(EEnemyCombatState NewCombatState)
{
	// validate
	if (NewCombatState == CombatState) { return; }
	
	// update combat state
	PreviousCombatState = CombatState;
	CombatState = NewCombatState;
}


// called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	CombatRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereOverlap);
	CombatRangeSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatRangeSphereOverlapEnd);

	// keep enemy meshes/capsules from colliding with camera
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECR_Ignore);

	SetEnemyCombatState(EEnemyCombatState::ECS_Idle);
	SpawnLocation = GetActorLocation();

	// get the AI controller
	EnemyController = Cast<AEnemyController>(GetController());

	if (PawnSensingComp)
	{
		// bind PawnSeen to OnSeePawn delegate
		PawnSensingComp->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	// if can patrol, do so
	if (CanPatrol())
	{
		SetEnemyCombatState(EEnemyCombatState::ECS_Patrolling);
		MoveToTarget(PatrolTarget);
	}

}

// called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	switch (AwarenessLevel)
	{
	case EEnemyAwarenessLevel::EAL_Passive:
		HandlePassiveStates();
		break;

	case EEnemyAwarenessLevel::EAL_Hostile:
		HandleHostileStates();
		break;
	}

	if (bIncapacitated) // adjustments for larger enemy type meshes when grounded
	{
		FVector CrawlingVerticalOffset = bNeedsExtraVerticalOffsetWhenDown ? FVector(0.f, 0.f, 8.f) : FVector(0.f, 0.f, 5.f);
		GetCapsuleComponent()->AddLocalOffset(CrawlingVerticalOffset);
	}
}


// detect overlap with player; set flags
void AEnemy::CombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == nullptr) { return; }

	if (OtherActor && IsAlive())
	{
		auto PC = Cast<APlayerCharacter>(OtherActor);

		if (PC)
		{
			bInAttackRange = true;

			if (EnemyController)
			{ EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), true); }

			if (AwarenessLevel == EEnemyAwarenessLevel::EAL_Hostile)
			{ CombatTarget = PC; }
		}
	}
}


// detect overlap with player ended; reset flags
void AEnemy::CombatRangeSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == nullptr) { return; }

	auto PC = Cast<APlayerCharacter>(OtherActor);

	if (PC)
	{
		bInAttackRange = false;

		if (EnemyController)
		{ EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), false); }
	}
}


// does this enemy have patrol targets assigned, and are they passive?
bool AEnemy::CanPatrol()
{
	return PatrolTargets.Num() > 0 && PatrolTarget && AwarenessLevel == EEnemyAwarenessLevel::EAL_Passive;
}


void AEnemy::CheckPatrolTarget()
{
	// have we reached current patrol target?
	if (InTargetRange(PatrolTarget, PatrolAcceptanceRadius))
	{
		// get a new patrol target
		PatrolTarget = UpdatePatrolTarget();

		// update state and set timer to head to new target
		SetEnemyCombatState(EEnemyCombatState::ECS_Idle);
		const float WaitTime = FMath::RandRange(PatrolIdleTimeMin, PatrolIdleTimeMax);
		GetWorldTimerManager().SetTimer(Patrol_TimerHandle, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

AActor* AEnemy::UpdatePatrolTarget()
{
		// filter out current active patrol target
		TArray<AActor*> ValidTargets;
		for (AActor* Target : PatrolTargets)
		{
			if (Target != PatrolTarget) 
			{ ValidTargets.AddUnique(Target); }
		}

		// select randomly from filtered valid targets
		const int32 NumPatrolTargets = ValidTargets.Num();
		if (NumPatrolTargets > 0)
		{
			const int32 SelectedTarget = FMath::RandRange(0, NumPatrolTargets - 1);
			return ValidTargets[SelectedTarget];
		}

	return nullptr;
}


void AEnemy::PatrolTimerFinished()
{
	if (AwarenessLevel != EEnemyAwarenessLevel::EAL_Hostile)
	{
		SetEnemyCombatState(EEnemyCombatState::ECS_Patrolling);
		EnemyController->SetFocus(PatrolTarget);
		GetWorldTimerManager().SetTimer(RotateTowards_TimerHandle, this, &AEnemy::MoveToCurrentPatrolTarget, PassiveRotateTowardsDelay);
	}
}


void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(Patrol_TimerHandle);
}


void AEnemy::ClearRotateTowardsTimer()
{
	GetWorldTimerManager().ClearTimer(RotateTowards_TimerHandle);
}

void AEnemy::ClearAggroAfterHitTimer()
{
	GetWorldTimerManager().ClearTimer(AggroAfterHit_TimerHandle);
}


void AEnemy::PawnSeen(APawn* SeenPawn)
{
	if (SeenPawn->ActorHasTag(FName("Dead"))) { return; }
	const bool bShouldChaseTarget =
		CombatState != EEnemyCombatState::ECS_Dead && bAlive &&
		CombatState != EEnemyCombatState::ECS_Chasing &&
		CombatState < EEnemyCombatState::ECS_Attacking &&
		SeenPawn->ActorHasTag(FName("Player"));

	if (bShouldChaseTarget)
	{
		// clear relevant active timers + stop current movement
		ClearPatrolTimer();
		ClearRotateTowardsTimer();
		ClearIdleSpeechCueTimer();
		EnemyController->StopMovement();

		// aggro
		CombatTarget = SeenPawn;
		bCanSeePlayer = true;
		bCanLookAtPlayer = true;
		SetEnemyAwarenessLevel(EEnemyAwarenessLevel::EAL_Hostile);
		RotateTowardsThenChasePlayer();

		// quick fade out to any playing speech + play random chasing cue
		if (ActiveSpeechAudio != nullptr && ActiveSpeechAudio->IsPlaying())
		{
			float FadeOutDuration = 0.5f;
			ActiveSpeechAudio->FadeOut(FadeOutDuration, 0.f);
			RandomSpeechCueToPlay = RandomChasingCue;
			GetWorldTimerManager().SetTimer(ChasingCue_TimerHandle, this, &AEnemy::PlayRandomSpeechCue, FadeOutDuration);
		}

		else
		{
			// immediately play a chasing cue
			RandomSpeechCueToPlay = RandomChasingCue;
			PlayRandomSpeechCue();
		}
	}
}


/// may or may not implement here
void AEnemy::PawnHeard(APawn* HeardPawn)
{
	if (HeardPawn->ActorHasTag(FName("Dead"))) { return; }
}

void AEnemy::HandlePassiveStates()
{
	if (!bAlive || AwarenessLevel != EEnemyAwarenessLevel::EAL_Passive) { return; }

	if (CanPatrol())
	{
		SetEnemyCombatState(EEnemyCombatState::ECS_Patrolling);
		HandlePatrollingState();
	}

	else
	{
		SetEnemyCombatState(EEnemyCombatState::ECS_Idle);
		HandleIdleState();
	}
}


void AEnemy::HandleHostileStates()
{
	if (!bAlive || !HasCombatTarget()) { return; }

	if (GetWorld()->TimeSince(LastPlayerLOSCheckTime) > PlayerLOSCheckFrequency)
	{
		CheckPlayerLOS();

		if (!bCanSeePlayer && GetWorldTimerManager().IsTimerActive(TargetLoss_TimerHandle) == false)
		{ GetWorldTimerManager().SetTimer(TargetLoss_TimerHandle, this, &AEnemy::LoseTarget, TargetLossDelay); }
	}

	if (CombatTarget->ActorHasTag(FName("Dead")) || IsPlayerOutsideCombatRadius()) { LoseInterestInPlayer(); }

	else if (IsPlayerOutsideAttackRange() && !IsPlayerInsideLungeAttackRange() && !IsEnemyChasing())
	{
		ClearAttackTimer();
		if (!IsEnemyEngaged()) { ChasePlayer(); }
	}

	else if (IsEnemyChasing() && !CanAttack() && !CanLungeAttack()) { HandleChasingState(); }

	else if (CanLungeAttack())
	{
		SetEnemyCombatState(EEnemyCombatState::ECS_Attacking);
		if (PreviousCombatState == EEnemyCombatState::ECS_Chasing) { LungeAttack(); }
		else { StartAttackTimer(); }
	}

	else if (CanAttack())
	{
		SetEnemyCombatState(EEnemyCombatState::ECS_Attacking);
		if (PreviousCombatState == EEnemyCombatState::ECS_Chasing) { Attack(); }
		else { StartAttackTimer(); }
	}
}


void AEnemy::MoveToCurrentCombatTarget()
{
	if (CombatTarget) 
	{ MoveToTarget(CombatTarget); }
}

void AEnemy::SetIncapacitated()
{
	bIncapacitated = true;
	GetCharacterMovement()->MaxWalkSpeed = PassiveWalkSpeed;
	GetCharacterMovement()->RotationRate = IncapacitatedRotationRate;
	AttackRange = CrawlingAttackRange;
	MoveToAcceptanceRadius = CrawlingMoveToAcceptanceRadius;
	EnemyController->StopMovement();
	MoveToCurrentCombatTarget();
	PlayIncapacitatedSFX();

}

// checks if within acceptance radius distance to target
bool AEnemy::InTargetRange(AActor* Target, float AcceptanceRadius)
{
	if (Target == nullptr) { return false; }

	const float DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= AcceptanceRadius;
}


void AEnemy::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr || !bAlive) { return; }
	
	// stop current movement
	EnemyController->StopMovement();

	// setup move request params
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(MoveToAcceptanceRadius);
	MoveRequest.SetUsePathfinding(true);

	EnemyController->MoveTo(MoveRequest);
}


void AEnemy::MoveToCurrentPatrolTarget()
{
	if (PatrolTarget && AwarenessLevel != EEnemyAwarenessLevel::EAL_Hostile) { MoveToTarget(PatrolTarget); }
}

// setter for enemy health
float AEnemy::ModifyHealth(const float Delta)
{
	const float OldHealth = Health;
	Health = FMath::Clamp<float>(Health + Delta, 0.0f, MaxHealth);

	return Health - OldHealth;
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bCanTakeDamage)
	{
		// in case enemy is currently attacking and has been interrupted by taking damage, ensure we still call AttackEnd() (otherwise triggered by attack anim notify event)
		AttackEnd();

		// store reference to player character
		if (APlayerCharacter* PC = Cast<APlayerCharacter>(DamageCauser))
		{ PlayerCharacter = PC; }
		
		// interrupt any relevant audio (playing or pending play);
		if (ActiveSpeechAudio != nullptr) { ActiveSpeechAudio->Stop(); }
		if (ActiveCrawlingAudio != nullptr) { ActiveCrawlingAudio->Stop(); }
		GetWorldTimerManager().ClearTimer(ChasingCue_TimerHandle);

		// call parent function
		Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

		// take health damage
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
			GetWorldTimerManager().SetTimer(TakeDamage_TimerHandle, this, &AEnemy::ResetCanTakeDamage, TakeDamageDelay);

			// play random hurt cue
			RandomSpeechCueToPlay = RandomHurtCue;
			PlayRandomSpeechCue();
			
			// play the appropriate hit reaction anim
			FHitReact HitReactData = GetHitReactToPlay();
			PlayAnimMontage(HitReactData.MontageToPlay, 1.f, HitReactData.SectionToPlay);

			// note duration of section in case we need to set an aggro timer
			int32 SectionToPlayIndex = HitReactData.MontageToPlay->GetSectionIndex(HitReactData.SectionToPlay);
			float AnimDuration = HitReactData.MontageToPlay->GetSectionLength(SectionToPlayIndex) - .25f;
			if (AnimDuration <= 0.f) { AnimDuration = .5f; }

			// play physics simulated hit react on torso + certain bones only
			if (bShouldPlayPhysicalHitReact) // determined in GetHitReactToPlay()
			{
				PlayPhysicalHitReact();
				bShouldPlayPhysicalHitReact = false;
			}

			// aggro upon receiving damage if not already hostile
			if (AwarenessLevel != EEnemyAwarenessLevel::EAL_Hostile)
			{
				CombatTarget = EventInstigator->GetPawn();
				GetWorldTimerManager().SetTimer(AggroAfterHit_TimerHandle, this, &AEnemy::AggroAfterHit, AnimDuration);
			}

			// already hostile
			else
			{ GetWorldTimerManager().SetTimer(HitReact_TimerHandle, this, &AEnemy::MoveToCurrentCombatTarget, AnimDuration); }

			return DamageAmount;
		}
	}

	return 0.0f;
}


void AEnemy::Die(AActor* Causer)
{
	// clear any active audio / pending timers + stop movement
	if (ActiveSpeechAudio != nullptr && ActiveSpeechAudio->IsPlaying()) { ActiveSpeechAudio->Stop(); }
	if (ActiveCrawlingAudio != nullptr && ActiveCrawlingAudio->IsPlaying()) { ActiveCrawlingAudio->Stop(); }
	ClearIdleSpeechCueTimer();
	ClearChasingSpeechCueTimer();
	ClearAttackTimer();
	ClearRotateTowardsTimer();
	ClearAggroAfterHitTimer();
	ClearPatrolTimer();
	EnemyController->StopMovement();
	EnemyController->SetFocus(NULL);

	// play death anim
	float AnimDuration = PlayDeathMontage();
	
	RandomSpeechCueToPlay = RandomDeathCue;	
	PlayRandomSpeechCue();
	GetWorldTimerManager().SetTimer(DeathEnd_TimerHandle, this, &AEnemy::DeathEnd, 2.f);

	// change status
	bAlive = false;
	SetEnemyCombatState(EEnemyCombatState::ECS_Dead);
	bCanLookAtPlayer = false;
	StopRespawn();

	SetIgnoreBloodChannel(); // BP; ignore blood trace channel collisions
}


void AEnemy::DeathEnd()
{
	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	CapsuleComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBloodPoolBP();
}


void AEnemy::StartRagdoll()
{
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

	if (!bIsRagdoll)
	{
		UCharacterMovementComponent* CharacterComp = Cast<UCharacterMovementComponent>(GetMovementComponent());
		if (CharacterComp)
		{
			CharacterComp->StopMovementImmediately();
			CharacterComp->DisableMovement();
			CharacterComp->SetComponentTickEnabled(false);
		}

		// apply ragdoll profile specific (in terms of strength) physics impulse on the bone of the enemy skeleton mesh hit 
		ApplyDeathblowImpulseBP();
		bIsRagdoll = true;
	}
}


void AEnemy::AggroAfterHit()
{
	// turn to face direction attacked from
	EnemyController->SetFocus(CombatTarget);
	MoveToCurrentCombatTarget();
}

ELastHitDirection AEnemy::GetLastHitDirection()
{
	// determine direction of last hit
	const FVector Forward = GetActorForwardVector();  // returns a normalized vector
	// lower impact point to the enemy's actor location Z (for debug)
	const FVector ImpactLowered(LastHitImpactPoint.X, LastHitImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();
	const FVector ToPlayer = (LastHitPlayerLocation - GetActorLocation()).GetSafeNormal();

	const double CosTheta = FVector::DotProduct(Forward, ToPlayer);
	// take inverse cosine (arc-cosine) of cos(theta) to get just theta
	double Theta = FMath::Acos(CosTheta);
	// convert from radians to degrees
	Theta = FMath::RadiansToDegrees(Theta);


	// if CrossProduct points down, Theta should be negative
	const FVector CrossProduct = FVector::CrossProduct(Forward, ToPlayer);
	if (CrossProduct.Z < 0) { Theta *= -1.f; }

	ELastHitDirection LastHitDirection;

	// strictly: -45.f && 45.f
	if (Theta >= -60.f && Theta < 60.f) 
	{ LastHitDirection = ELastHitDirection::ELHD_Front; }

	// strictly: -135.f && -45.f
	else if (Theta >= -120.f && Theta < -60.f)
	{ LastHitDirection = ELastHitDirection::ELHD_Left; }

	// strictly: 45.f && 135.f
	else if (Theta >= 60.f && Theta < 120.f)
	{ LastHitDirection = ELastHitDirection::ELHD_Right; }

	else
	{ LastHitDirection = ELastHitDirection::ELHD_Back; }

	LastHitTheta = Theta;

	return LastHitDirection;
}

EBoneHitReactValue AEnemy::GetLastBoneHitMapping()
{
	// check if last bone hit (set by weapon when hit determined) was limb or torso
	EBoneHitReactValue BoneHitReact = EBoneHitReactValue::EBHR_Torso;
	for (auto& Bone : BoneHitReactMap)
	{
		if (LastBoneHit == Bone.Key)
		{
			bLastHitWasLimb = true;
			BoneHitReact = Bone.Value;
			break;
		}

		bLastHitWasLimb = false;
	}

	return BoneHitReact;
}

FHitReact AEnemy::GetHitReactToPlay()
{
	// get direction and bone mapping of last hit
	ELastHitDirection LastHitDirection = GetLastHitDirection();
	EBoneHitReactValue LastBoneHitReact = GetLastBoneHitMapping();

	// determine which montage and section to play
	UAnimMontage* MontageToPlay;
	FName SectionToPlay;

	// leg anims are ok to play from any direction, as are torso anims
	if (LastHitDirection != ELastHitDirection::ELHD_Front || LastBoneHitReact == EBoneHitReactValue::EBHR_Torso)
	{
		MontageToPlay = HitReactsLimbs;

		if (LastBoneHitReact == EBoneHitReactValue::EBHR_LeftLeg)
		{ 
			LeftLegHitCounter ++;
			if (LeftLegHitCounter >= 2) 
			{ 
				MontageToPlay = HitReactsLimbs;
				SectionToPlay = FName("LeftLeg");
				LeftLegHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
			}
		}

		else if (LastBoneHitReact == EBoneHitReactValue::EBHR_RightLeg)
		{ 
			RightLegHitCounter ++;
			if (RightLegHitCounter >= 2) 
			{
				MontageToPlay = HitReactsLimbs;
				SectionToPlay = FName("RightLeg");
				RightLegHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
			}
		}

		else
		{
			// torso; play appropriate directional torso anim
			MontageToPlay = HitReactsTorso;
			SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
			bShouldPlayPhysicalHitReact = true;
		}
	}

	// hit a limb from the front:: all anims valid
	else
	{
		MontageToPlay = HitReactsLimbs;

		switch (LastBoneHitReact)
		{
		case EBoneHitReactValue::EBHR_Head:
			HeadHitCounter++;
			if (HeadHitCounter >= 2)
			{
				SectionToPlay = FName("Head");
				HeadHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
			}
			break;

		case EBoneHitReactValue::EBHR_LeftArm:
			LeftArmHitCounter++;
			if (LeftArmHitCounter >= 2)
			{
				SectionToPlay = FName("LeftArm");
				LeftArmHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
				bEnableLeftArmProfile = true;
			}
			break;

		case EBoneHitReactValue::EBHR_LeftLeg:
			LeftLegHitCounter++;
			if (LeftLegHitCounter >= 2)
			{
				MontageToPlay = HitReactsLimbs;
				SectionToPlay = FName("LeftLeg");
				LeftLegHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
				bEnableLeftLegProfile = true;
			}
			break;

		case EBoneHitReactValue::EBHR_RightArm:
			RightArmHitCounter++;
			if (RightArmHitCounter >= 2)
			{
				SectionToPlay = FName("RightArm");
				RightArmHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
				bEnableRightArmProfile = true;	
			}
			break;

		case EBoneHitReactValue::EBHR_RightLeg:
			RightLegHitCounter++;
			if (RightLegHitCounter >= 2)
			{
				MontageToPlay = HitReactsLimbs;
				SectionToPlay = FName("RightLeg");
				RightLegHitCounter = 0;
				EnemyController->StopMovement();
			}
			else
			{
				MontageToPlay = HitReactsTorso;
				SectionToPlay = GetTorsoHitReactSectionName(LastHitDirection);
				bShouldPlayPhysicalHitReact = true;
				bEnableLeftLegProfile = true;
			}
			break;
		}
	}

	FHitReact HitReactData;
	HitReactData.MontageToPlay = MontageToPlay;
	HitReactData.SectionToPlay = SectionToPlay;

	return HitReactData;
}

void AEnemy::PlayRandomSpeechCue()
{
	if (RandomSpeechCueToPlay != nullptr)
	{
		// interrupt any relevant audio (playing or pending play; attack cue played on AnimNotify from BP);
		if (ActiveSpeechAudio != nullptr) { ActiveSpeechAudio->Stop(); }
		if (ActiveCrawlingAudio != nullptr) { ActiveCrawlingAudio->Stop(); }
		ClearIdleSpeechCueTimer();
		GetWorldTimerManager().ClearTimer(ChasingCue_TimerHandle);

		// play sound cue
		ActiveSpeechAudio = UGameplayStatics::SpawnSoundAttached(RandomSpeechCueToPlay, GetRootComponent());
		
		// make value for close mouth timer
		float SpeechDuration = ActiveSpeechAudio->GetSound()->GetDuration();
		SpeechDuration -= 1.5f;
		if (SpeechDuration <= 1.5f) { SpeechDuration = 1.5f; }

		// set morph target on timeline (BP) to open mouth (validated in BP)
		OpenMouth();

		// clear any existing + set close mouth timer
		GetWorldTimerManager().ClearTimer(CloseMouth_TimerHandle);
		GetWorldTimerManager().SetTimer(CloseMouth_TimerHandle, this, &AEnemy::CloseMouth, SpeechDuration);
	}
}

void AEnemy::ClearIdleSpeechCueTimer()
{
	GetWorldTimerManager().ClearTimer(IdleCue_TimerHandle);
}

void AEnemy::ClearChasingSpeechCueTimer()
{
	GetWorldTimerManager().ClearTimer(ChasingCue_TimerHandle);
}	

void AEnemy::HandleIdleState()
{
	// periodically play random idle speech
	if (!GetWorldTimerManager().IsTimerActive(IdleCue_TimerHandle) && bCanPlayIdleSpeech)
	{
		float IdleCueInterval = FMath::RandRange(IdleCueIntervalMin, IdleCueIntervalMax);
		float Deviation = FMath::RandRange(-2.f, 2.f);

		if (ActiveSpeechAudio != nullptr)
		{
			if (!ActiveSpeechAudio->IsPlaying())

			{
				RandomSpeechCueToPlay = RandomIdleCue;
				GetWorldTimerManager().SetTimer(IdleCue_TimerHandle, this, &AEnemy::PlayRandomSpeechCue, IdleCueInterval + Deviation);
			}
		}
		else
		{
			RandomSpeechCueToPlay = RandomIdleCue;
			GetWorldTimerManager().SetTimer(IdleCue_TimerHandle, this, &AEnemy::PlayRandomSpeechCue, IdleCueInterval + Deviation);
		}
	}
}


void AEnemy::HandlePatrollingState()
{
	// keep patrol target updated
	CheckPatrolTarget();
	HandleIdleState();
}

void AEnemy::HandleChasingState()
{
	// periodically play random chasing speech
	if (!GetWorldTimerManager().IsTimerActive(ChasingCue_TimerHandle))
	{
		float ChasingCueInterval = FMath::RandRange(ChasingCueIntervalMin, ChasingCueIntervalMax);
		float Deviation = FMath::RandRange(-1.f, 1.f);

		if (ActiveSpeechAudio != nullptr)
		{
			if (!ActiveSpeechAudio->IsPlaying())
			{
				RandomSpeechCueToPlay = RandomChasingCue;
				GetWorldTimerManager().SetTimer(ChasingCue_TimerHandle, this, &AEnemy::PlayRandomSpeechCue, ChasingCueInterval + Deviation);
			}
		}
		else
		{
			RandomSpeechCueToPlay = RandomChasingCue;
			GetWorldTimerManager().SetTimer(ChasingCue_TimerHandle, this, &AEnemy::PlayRandomSpeechCue, ChasingCueInterval + Deviation);
		}
	}
}


void AEnemy::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}


int32 AEnemy::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) { return -1; }

	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

float AEnemy::PlayAttackMontage()
{
	int32 Selection;
	float AnimDuration;

	if (bIncapacitated)
	{
		Selection = PlayRandomMontageSection(CrawlingAttackMontage, CrawlingAttackMontageSections);
		AnimDuration = CrawlingAttackMontage->GetSectionLength(Selection);
	}

	else
	{
		Selection = PlayRandomMontageSection(AttackCloseMontage, AttackCloseMontageSections);
		AnimDuration = AttackCloseMontage->GetSectionLength(Selection);
	}

	return AnimDuration;
}


float AEnemy::PlayLungeAttackMontage()
{
	int32 Selection;
	float AnimDuration;

	Selection = PlayRandomMontageSection(AttackDistanceMontage, AttackDistanceMontageSections);
	AnimDuration = AttackDistanceMontage->GetSectionLength(Selection);

	return AnimDuration;
}


float AEnemy::PlayDeathMontage()
{
	// play appropriate death anim + update death pose to match
	int32 Selection;
	float AnimDuration;

	Selection = PlayRandomMontageSection(DeathMontage, DeathMontageBackwardOnlySections);
	AnimDuration = PlayAnimMontage(DeathFallBackwardsMontage);

	return AnimDuration;
}

void AEnemy::LoseInterestInPlayer()
{
	if (AwarenessLevel == EEnemyAwarenessLevel::EAL_Passive) { return; }
	bCanLookAtPlayer = false;
	CombatTarget = nullptr;
	EnemyController->SetFocus(NULL);
	EnemyController->StopMovement();
	SetEnemyAwarenessLevel(EEnemyAwarenessLevel::EAL_Passive);
	SetEnemyCombatState(EEnemyCombatState::ECS_Idle);
	ClearAttackTimer();
	ClearChasingSpeechCueTimer();
	if (CanPatrol()) { StartPatrolling(); }

	else // wander away after brief delay
	{ GetWorldTimerManager().SetTimer(WanderDelay_TimerHandle, this, &AEnemy::WanderAway, 2.f); }
}


void AEnemy::StartPatrolling()
{
	SetEnemyCombatState(EEnemyCombatState::ECS_Patrolling);
	EnemyController->SetFocus(PatrolTarget);
	GetWorldTimerManager().SetTimer(RotateTowards_TimerHandle, this, &AEnemy::MoveToCurrentPatrolTarget, PassiveRotateTowardsDelay);
}

void AEnemy::ChasePlayer()
{
	SetEnemyCombatState(EEnemyCombatState::ECS_Chasing);
	MoveToCurrentCombatTarget();
}


void AEnemy::RotateTowardsThenChasePlayer()
{
	SetEnemyCombatState(EEnemyCombatState::ECS_Chasing);
	EnemyController->SetFocus(CombatTarget);
	GetWorldTimerManager().SetTimer(RotateTowards_TimerHandle, this, &AEnemy::MoveToCurrentCombatTarget, HostileRotateTowardsDelay);
}


bool AEnemy::CanAttack()
{
	return IsPlayerInsideAttackRange() && !IsEnemyAttacking() && !IsEnemyEngaged() && bAlive && IsFacingPlayer();
}


bool AEnemy::CanLungeAttack()
{
	return !IsPlayerInsideAttackRange() && IsPlayerInsideLungeAttackRange() && !bIncapacitated && !IsEnemyAttacking() && !IsEnemyEngaged() && bAlive && IsFacingPlayer();
}


void AEnemy::Attack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		LoseInterestInPlayer();
		return;
	}
	
	SetEnemyCombatState(EEnemyCombatState::ECS_Engaged);
	float AnimDuration = PlayAttackMontage();
	float LoseTrackingDelay = AnimDuration * .75f;
	GetWorldTimerManager().SetTimer(LoseTracking_TimerHandle, this, &AEnemy::LosePlayerTracking, LoseTrackingDelay);
	GetWorldTimerManager().SetTimer(Attacking_TimerHandle, this, &AEnemy::AttackEnd, AnimDuration);
}


void AEnemy::LungeAttack()
{
	if (CombatTarget && CombatTarget->ActorHasTag(FName("Dead")))
	{
		LoseInterestInPlayer();
		return;
	}

	SetEnemyCombatState(EEnemyCombatState::ECS_Engaged);
	float AnimDuration = PlayLungeAttackMontage();
	float LoseTrackingDelay = AnimDuration * .75f;
	GetWorldTimerManager().SetTimer(LoseTracking_TimerHandle, this, &AEnemy::LosePlayerTracking, LoseTrackingDelay);
	GetWorldTimerManager().SetTimer(Attacking_TimerHandle, this, &AEnemy::AttackEnd, AnimDuration);
}


// called to reset engaged flags; used at end of an attack animation
void AEnemy::AttackEnd()
{
	SetEnemyCombatState(EEnemyCombatState::ECS_Holding);
	EnemyController->SetFocus(CombatTarget);
	HandleHostileStates();
}


void AEnemy::StartAttackTimer()
{
	const float AttackDelay = FMath::RandRange(AttackDelayMin, AttackDelayMax);
	GetWorldTimerManager().SetTimer(AttackTimer_TimerHandle, this, &AEnemy::Attack, AttackDelay);
}


void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer_TimerHandle);
}


// additional LOS validation - are we, roughly, facing player?
bool AEnemy::IsFacingPlayer()
{
	const FVector EnemyForward = GetActorForwardVector(); 
	const FVector PlayerForward = CombatTarget->GetActorForwardVector();
	float DotP = FVector::DotProduct(PlayerForward, EnemyForward);

	if (DotP < 0) {	return true; }
	else { return false; }
}


void AEnemy::LosePlayerTracking()
{
	EnemyController->SetFocus(NULL);
}


// validating space in front of enemy
bool AEnemy::IsClearFront()
{
	FHitResult FrontTraceHit;
	FVector EnemyLoc = GetActorLocation();
	FVector LoweredEnemyLoc = FVector(EnemyLoc.X, EnemyLoc.Y, (EnemyLoc.Z - 50.f));
	const FVector TraceStart = LoweredEnemyLoc + (GetActorForwardVector() * 10.f);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * 50.f);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 40.f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, FrontTraceHit, true, FLinearColor::Gray, FLinearColor::Blue, 0.f);

	return !Hit;
}


// validating space behind enemy
bool AEnemy::IsClearBehind()
{
	FHitResult BehindTraceHit;
	FVector EnemyLoc = GetActorLocation();
	FVector LoweredEnemyLoc = FVector(EnemyLoc.X, EnemyLoc.Y, (EnemyLoc.Z - 50.f));
	const FVector TraceStart = LoweredEnemyLoc + (GetActorForwardVector() * -10.f);
	const FVector TraceEnd = TraceStart + (GetActorForwardVector() * -50.f);

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	const bool Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, 40.f, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::None, BehindTraceHit, true, FLinearColor::Gray, FLinearColor::Blue, 0.f);

	return !Hit;
}


// after player death, moving to random location nearby
void AEnemy::WanderAway()
{
	const FVector OriginLocation = GetActorLocation();
	FVector RandomLocation;
	bool bRandomLocationFound = UNavigationSystemV1::K2_GetRandomReachablePointInRadius(this, OriginLocation, RandomLocation, 500.f);

	if (bRandomLocationFound)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalLocation(RandomLocation);
		MoveRequest.SetAcceptanceRadius(MoveToAcceptanceRadius);
		MoveRequest.SetUsePathfinding(true);
		EnemyController->MoveTo(MoveRequest);
	}
}


// can we see the player?
void AEnemy::CheckPlayerLOS()
{
	if (CombatTarget == nullptr)
	{
		bCanSeePlayer = false;
		return;
	}

	LastPlayerLOSCheckTime = GetWorld()->GetTimeSeconds();

	FHitResult HitResult;
	FVector TraceStart = GetActorLocation();
	FVector TraceEnd = CombatTarget->GetActorLocation();
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, QueryParams);
	
	bCanSeePlayer = !bHit;
}


void AEnemy::LoseTarget()
{
	if (bCanSeePlayer) { return; }

	LoseInterestInPlayer();
}

FName AEnemy::GetTorsoHitReactSectionName(ELastHitDirection LastHitDirection)
{
	FName SectionToPlay;
	switch (LastHitDirection)
	{
	case ELastHitDirection::ELHD_Front:
		SectionToPlay = FName("FromFront");
		break;
	case ELastHitDirection::ELHD_Back:
		SectionToPlay = FName("FromBack");
		break;
	case ELastHitDirection::ELHD_Left:
		SectionToPlay = FName("FromLeft");
		break;
	case ELastHitDirection::ELHD_Right:
		SectionToPlay = FName("FromRight");
		break;

	default:
		SectionToPlay = FName("FromFront");
		break;
	}

	return SectionToPlay;
}