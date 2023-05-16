// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "EscapeRoomProjectGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ESCAPEROOMPROJECT_API UEscapeRoomProjectGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION()
		virtual void BeginLoadingScreen(const FString& MapName);
	UFUNCTION()
		virtual void EndLoadingScreen(UWorld* InLoadedWorld);
	
};
