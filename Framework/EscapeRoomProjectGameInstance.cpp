// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Framework/EscapeRoomProjectGameInstance.h"
#include <MoviePlayer/Public/MoviePlayer.h>

void UEscapeRoomProjectGameInstance::Init()
{
	Super::Init();

	FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UEscapeRoomProjectGameInstance::BeginLoadingScreen);
	FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UEscapeRoomProjectGameInstance::EndLoadingScreen);
}

void UEscapeRoomProjectGameInstance::BeginLoadingScreen(const FString& InMapName)
{
	if (!IsRunningDedicatedServer())
	{
		FLoadingScreenAttributes LoadingScreen;
		LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
		LoadingScreen.WidgetLoadingScreen = FLoadingScreenAttributes::NewTestLoadingScreenWidget();

		GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
	}
}

void UEscapeRoomProjectGameInstance::EndLoadingScreen(UWorld* InLoadedWorld)
{

}
