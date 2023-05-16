// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../DebugMacros.h"
#include "Enemy.generated.h"

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EEnemyAwarenessLevel : uint8
{
	EAL_Passive		UMETA(DisplayName = "Passive"),
	EAL_Hostile		UMETA(DisplayName = "Hostile"),

	EAL_MAX			UMETA(DisplayName = "DefaultMAX")
};


UENUM(BlueprintType)
enum class EEnemyCombatState : uint8
{
	ECS_Dead		UMETA(DisplayName = "Dead"),
	ECS_Idle		UMETA(DisplayName = "Idle"),
	ECS_Patrolling	UMETA(DisplayName = "Patrolling"),
	ECS_Chasing		UMETA(DisplayName = "Chasing"),
	ECS_Attacking	UMETA(DisplayName = "Attacking"),
	ECS_Engaged		UMETA(DisplayName = "Engaged"),
	ECS_Holding		UMETA(DisplayName = "Holding"),

	ECS_MAX			UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EBoneHitReactValue : uint8
{
	EBHR_Torso		UMETA(DisplayName = "Torso"),
	EBHR_Head		UMETA(DisplayName = "Head"),
	EBHR_LeftArm	UMETA(DisplayName = "Left Arm"),
	EBHR_RightArm	UMETA(DisplayName = "Right Arm"),
	EBHR_LeftLeg	UMETA(DisplayName = "Left Leg"),
	EBHR_RightLeg	UMETA(DisplayName = "Right Leg"),

	EBHR_MAX		UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class ELastHitDirection : uint8
{
	ELHD_Front		UMETA(DisplayName = "Front"),
	ELHD_Back		UMETA(DisplayName = "Back"),
	ELHD_Left		UMETA(DisplayName = "Left"),
	ELHD_Right		UMETA(DisplayName = "Right"),

	ELHD_MAX		UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FHitReact
{
	GENERATED_BODY()

	UAnimMontage* MontageToPlay;
	FName SectionToPlay;
};

UENUM(BlueprintType)
enum EDeathPose
{
	EDP_CrawlingPose1		UMETA(DisplayName = "CrawlingPose1"),
	EDP_BackwardPose1		UMETA(DisplayName = "BackwardPose1"),
	EDP_ForwardPose1		UMETA(DisplayName = "ForwardPose1"),

	EDP_MAX			UMETA(DisplayName = "DefaultMAX")
};


UCLASS()
class ESCAPEROOMPROJECT_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	
	// sets default values for this character's properties
	AEnemy();

	class AEnemyController* EnemyController;

	UPROPERTY(VisibleAnywhere)
	class UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(EditAnywhere, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
	class UBehaviorTree* BehaviorTree;

	UPROPERTY(BlueprintReadOnly, BlueprintReadOnly, Category = "AI")
	class USphereComponent* CombatRangeSphere;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	class AActor* AssignedZone;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	FVector SpawnLocation;

	/*
	*  movement and rotation speeds, awareness levels, combat states
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float PassiveWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float HostileWalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FRotator PassiveRotationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FRotator HostileRotationRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	FRotator IncapacitatedRotationRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EEnemyAwarenessLevel AwarenessLevel;

	UFUNCTION(BlueprintCallable)
	void SetEnemyAwarenessLevel(EEnemyAwarenessLevel NewAwarenessLevel);

	FORCEINLINE EEnemyAwarenessLevel GetEnemyAwarenessLevel() { return AwarenessLevel; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EEnemyCombatState CombatState;

	UFUNCTION(BlueprintCallable)
	void SetEnemyCombatState(EEnemyCombatState NewCombatState);

	FORCEINLINE EEnemyCombatState GetEnemyCombatState() { return CombatState; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	EEnemyCombatState PreviousCombatState;

	/*
	*  health, alive and damage stats
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bAlive;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	bool bIsRagdoll;

	FTimerHandle StartRagdoll_TimerHandle;

	/*
	*	perception variables
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float PeripheralVisionAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float PerceptionRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float VisionRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float VisionAggroRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float HearingAggroRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float TargetLossDelay;

	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float DistanceToPlayerCharacter;

	float LastPlayerLOSCheckTime;

	float PlayerLOSCheckFrequency;

	FTimerHandle TargetLoss_TimerHandle;

	/*
	*	navigation variables
	*/

	// currently active patrol target
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI Navigation")
	AActor* PatrolTarget;

	// available patrol targets for this enemy instance
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navigation")
	float PatrolAcceptanceRadius;

	FTimerHandle Patrol_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navigation")
	float PatrolIdleTimeMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navigation")
	float PatrolIdleTimeMax;

	FTimerHandle RotateTowards_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navigation")
	float PassiveRotateTowardsDelay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navigation")
	float HostileRotateTowardsDelay;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 LeftLegHitCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RightLegHitCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 LeftArmHitCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 RightArmHitCounter;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int32 HeadHitCounter;

	/*
	*  physical animation profiles to allow each limb to participate more heavily in simulation when hit 
	*  (and only then, so as not to throw off everything else too much)
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableLeftArmProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRightArmProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableLeftLegProfile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bEnableRightLegProfile;

	// deprecated, no longer a feature but leaving for reference
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Navigation")
	bool bIncapacitated;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Navigation")
	bool bNeedsExtraVerticalOffsetWhenDown;

	FTimerHandle WanderDelay_TimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Navigation")
	bool bCanSeePlayer;

	/*
	*  combat variables
	*/

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	AActor* CombatTarget;

	// maximum distance in which to remain hostile
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CombatRadius;

	// maximum distance in which to initiate an attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackRange;

	// maximum distance in which to initiate a lunge attack
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float LungeAttackRange;

	// maximum distance in which to initiate an attack while incapacitated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CrawlingAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float MoveToAcceptanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CrawlingMoveToAcceptanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bStaggered;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanLookAtPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bCanTakeDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	bool bInAttackRange;

	// new attack timer stuff
	FTimerHandle AttackTimer_TimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelayMin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float AttackDelayMax;

	// old, deprecated attack timer stuff
	FTimerHandle Attacking_TimerHandle;

	FTimerHandle AttackDelay_TimerHandle;

	FTimerHandle TakeDamage_TimerHandle;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float TakeDamageDelay;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<UDamageType> DamageTypeClass;

	// a map of bone -> hit react enum
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	TMap<FName, EBoneHitReactValue> BoneHitReactMap;

	FTimerHandle HitReact_TimerHandle;

	FTimerHandle AggroAfterHit_TimerHandle;

	bool bShouldPlayPhysicalHitReact;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TorsoPhysicalHitReactStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadPhysicalHitReactStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ArmPhysicalHitReactStrength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float LegPhysicalHitReactStrength;

	bool bLastHitWasLimb;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FHitResult LastHitResult;

	FName LastBoneHit;

	FVector LastHitImpactPoint;
	FVector LastHitPlayerLocation;

	double LastHitTheta;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TEnumAsByte<EDeathPose> DeathPose;

	FTimerHandle DeathEnd_TimerHandle;

	FTimerHandle LoseTracking_TimerHandle;

	/*
	*  combat anims
	*/

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* HitReactsTorso;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* HitReactsLimbs;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* HitReactsCrawling;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* AttackCloseMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> AttackCloseMontageSections;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* AttackDistanceMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> AttackDistanceMontageSections;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* CrawlingAttackMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> CrawlingAttackMontageSections;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageAllSections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageBackwardOnlySections;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> DeathMontageForwardOnlySections;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DeathDirectionalMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DeathFallBackwardsMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* DeathFallForwardsMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	UAnimMontage* CrawlingDeathMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TArray<FName> CrawlingDeathMontageSections;

	/*
	*  combat FX
	*/

	// FX for blood impact particles (spawned on bullet impact)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* BloodImpactParticles1;

	// FX for blood impact particles (spawned on bullet impact)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* BloodImpactParticles2;

	// FX for blood pooling (spawned underneath enemy after death)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* BloodPool;

	/*
	*  sound FX
	*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	bool bCanPlayIdleSpeech;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class UAudioComponent* ActiveSpeechAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class UAudioComponent* ActiveCrawlingAudio;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundBase* RandomSpeechCueToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundBase* RandomIdleCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundCue* RandomChasingCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundCue* RandomHurtCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundCue* RandomDeathCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundCue* RandomAttackShortCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	class USoundCue* RandomAttackLongCue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	float IdleCueIntervalMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	float IdleCueIntervalMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	float ChasingCueIntervalMin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	float ChasingCueIntervalMax;

	FTimerHandle IdleCue_TimerHandle;

	FTimerHandle ChasingCue_TimerHandle;

	FTimerHandle CloseMouth_TimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SFX")
	bool bMouthOpen;


protected:

	// called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	

	// called every frame
	virtual void Tick(float DeltaTime) override;

	void DetermineCombatState();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsAlive() { return ( bAlive && CombatState != EEnemyCombatState::ECS_Dead); }

	UFUNCTION()
	void CombatRangeSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void CombatRangeSphereOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// getter for behavior tree
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }

	/*
	*  state management
	*/

	void HandlePassiveStates();

	void HandleHostileStates();

	void HandleIdleState();

	void HandlePatrollingState();

	void HandleChasingState();

	/*
	*  speech
	*/

	UFUNCTION(BlueprintCallable)
	void PlayRandomSpeechCue();

	void ClearIdleSpeechCueTimer();

	void ClearChasingSpeechCueTimer();

	/*
	*  navigation
	*/

	// checks if within acceptance radius distance to patrol target
	bool InTargetRange(AActor* Target, float AcceptanceRadius);

	void MoveToTarget(AActor* Target);

	void MoveToCurrentPatrolTarget();

	// do we have assigned patrol points? if so, can patrol
	bool CanPatrol();

	void CheckPatrolTarget();

	AActor* UpdatePatrolTarget();

	void PatrolTimerFinished();

	void ClearPatrolTimer();

	void ClearRotateTowardsTimer();

	void ClearAggroAfterHitTimer();

	/*
	*  perception
	*/

	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);

	UFUNCTION()
	void PawnHeard(APawn* HeardPawn);

	/*
	*  manage morph targets
	*/

	UFUNCTION(BlueprintImplementableEvent)
	void OpenMouth();

	UFUNCTION(BlueprintImplementableEvent)
	void CloseMouth();

	/*
	*  combat
	*/

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasCombatTarget() { return CombatTarget != nullptr; }

	void MoveToCurrentCombatTarget();

	void SetIncapacitated();

	UFUNCTION(BlueprintCallable)
	float ModifyHealth(const float Delta);

	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AActor* Causer);

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	UFUNCTION(BlueprintCallable)
	void StartRagdoll();

	FORCEINLINE void ResetCanTakeDamage() { bCanTakeDamage = true; }

	void AggroAfterHit();

	ELastHitDirection GetLastHitDirection();

	EBoneHitReactValue GetLastBoneHitMapping();

	FHitReact GetHitReactToPlay();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayBloodHitFX(FHitResult Hit);

	UFUNCTION(BlueprintImplementableEvent)
	void PlayIncapacitatedSFX();


	FORCEINLINE void ResetEngaged() { SetEnemyCombatState(EEnemyCombatState::ECS_Attacking); }

	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);

	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	float PlayAttackMontage();

	float PlayLungeAttackMontage();

	float PlayDeathMontage();

	void LoseInterestInPlayer();

	void StartPatrolling();

	void ChasePlayer();

	void RotateTowardsThenChasePlayer();

	FORCEINLINE bool IsPlayerOutsideCombatRadius() { return !InTargetRange(CombatTarget, CombatRadius); }

	FORCEINLINE bool IsPlayerOutsideAttackRange() { return !InTargetRange(CombatTarget, AttackRange); }

	FORCEINLINE bool IsPlayerOutsideLungeAttackRange() { return !InTargetRange(CombatTarget, LungeAttackRange); }

	FORCEINLINE bool IsPlayerInsideAttackRange() { return InTargetRange(CombatTarget, AttackRange); }

	FORCEINLINE bool IsPlayerInsideLungeAttackRange() { return InTargetRange(CombatTarget, LungeAttackRange); }

	FORCEINLINE bool IsEnemyChasing() { return CombatState == EEnemyCombatState::ECS_Chasing; }

	FORCEINLINE bool IsEnemyAttacking() { return CombatState == EEnemyCombatState::ECS_Attacking; }

	FORCEINLINE bool IsEnemyEngaged() { return CombatState == EEnemyCombatState::ECS_Engaged; }

	UFUNCTION(BlueprintCallable)
	bool CanAttack();

	UFUNCTION(BlueprintCallable)
	bool CanLungeAttack();

	void Attack();

	void LungeAttack();

	void StartAttackTimer();

	void ClearAttackTimer();

	bool IsFacingPlayer();

	void LosePlayerTracking();

	bool IsClearFront();

	bool IsClearBehind();

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnBloodPoolBP();

	void WanderAway();

	void CheckPlayerLOS();

	void LoseTarget();

	UFUNCTION(BlueprintImplementableEvent)
	void StopRespawn();

	UFUNCTION(BlueprintImplementableEvent)
	void SetIgnoreBloodChannel();

	UFUNCTION(BlueprintImplementableEvent)
	void PlayPhysicalHitReact();

	FName GetTorsoHitReactSectionName(ELastHitDirection LastHitDirection);

	UFUNCTION(BlueprintImplementableEvent)
	void ApplyDeathblowImpulseBP();

};