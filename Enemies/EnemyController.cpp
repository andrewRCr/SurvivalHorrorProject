// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Enemies/EnemyController.h"
#include "../Enemies/Enemy.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"


AEnemyController::AEnemyController()
{
	// construct blackboard component
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
	// assert valid; halt execution if not
	check(BlackboardComponent);

	// construct behavior tree component
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
	// assert valid; halt execution if not
	check(BlackboardComponent);

}


void AEnemyController::BeginPlay()
{
	Super::BeginPlay();
}


void AEnemyController::OnPossess(APawn* InPawn)
{
	// call to parent function
	Super::OnPossess(InPawn);

	if (InPawn == nullptr) { return; }

	AEnemy* Enemy = Cast<AEnemy>(InPawn);

	if (Enemy)
	{
		if (Enemy->GetBehaviorTree())
		{
			// initialize blackboard component
			BlackboardComponent->InitializeBlackboard(*(Enemy->GetBehaviorTree()->BlackboardAsset));
		}
	}
}
