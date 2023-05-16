// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../Components/InteractionComponent.h"
#include "../UserInterface/InteractionWidget.h"


UInteractionComponent::UInteractionComponent()
{
	// no need for tick
	SetComponentTickEnabled(false);

	// set defaults
	InteractionTime = 0.f;
	InteractionDistance = 100.f;
	bShouldShowInteractPrompt = true;
	bHideInteractPromptOnInteract = true;
	bHideOutlineOnInteract = true;
	bHideOutlineOnEndFocus = true;
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(400, 100);
	bDrawAtDesiredSize = true;

	// interaction prompt active but hidden by default (until player approaches and "focuses" on it)
	SetActive(true);
	SetHiddenInGame(true);

	SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

	if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
	{ InteractionWidget->OwningInteractionComponent = this; }

}


void UInteractionComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (APlayerCharacter* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	Interactors.Empty();
}


bool UInteractionComponent::CanInteract(class APlayerCharacter* PlayerCharacter) const
{
	const bool bPlayerAlreadyInteracting = Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && PlayerCharacter != nullptr;
}


void UInteractionComponent::RefreshWidget()
{
	if (!bHiddenInGame)
	{
		if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
		{ InteractionWidget->UpdateInteractionWidget(this); }
	}
}


void UInteractionComponent::BeginFocus(class APlayerCharacter* PlayerCharacter)
{
	// validate
	if (!IsActive() || !GetOwner() || !PlayerCharacter) { return; }

	// call delegate
	OnBeginFocus.Broadcast(PlayerCharacter);

	// show UI interaction prompt widget's input-specific (kb/m or gamepad input) icon
	if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
	{
		InteractionWidget->OwningInteractionComponent = this;
		InteractionWidget->SwitchActiveIcon(true);
	}

	// show outline around object
	TArray<UActorComponent*> Components;
	GetOwner()->GetComponents(UPrimitiveComponent::StaticClass(), Components);

	for (auto& VisualComp : Components)
	{
		if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
		{ Prim->SetRenderCustomDepth(true); }
	}

	RefreshWidget();
}


void UInteractionComponent::EndFocus(class APlayerCharacter* PlayerCharacter)
{
	// call delegate
	OnEndFocus.Broadcast(PlayerCharacter);

	// switch UI interaction prompt widget back to general (non-input specific) icon
	if (UInteractionWidget* InteractionWidget = Cast<UInteractionWidget>(GetUserWidgetObject()))
	{ InteractionWidget->SwitchActiveIcon(false); }

	// hide outline around object
	if (bHideOutlineOnEndFocus)
	{
		TArray<UActorComponent*> Components;
		GetOwner()->GetComponents(UPrimitiveComponent::StaticClass(), Components);

		for (auto& VisualComp : Components)
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{ Prim->SetRenderCustomDepth(false); }
		}
	}
}


void UInteractionComponent::BeginInteract(class APlayerCharacter* PlayerCharacter)
{
	Interactors.AddUnique(PlayerCharacter);
	OnBeginInteract.Broadcast(PlayerCharacter);

	// hide UI interaction prompt widget (can be controlled in blueprint case by case)
	if (bHideInteractPromptOnInteract)
	{ SetHiddenInGame(true); }

	// hide item outline on interact (so items not immediately picked up, e.g. chests, don't stay outlined past interact point)
	if (bHideOutlineOnInteract)
	{
		TArray<UActorComponent*> Components;
		GetOwner()->GetComponents(UPrimitiveComponent::StaticClass(), Components);

		for (auto& VisualComp : Components)
		{
			if (UPrimitiveComponent* Prim = Cast<UPrimitiveComponent>(VisualComp))
			{ Prim->SetRenderCustomDepth(false); }
		}
	}
}


void UInteractionComponent::EndInteract(class APlayerCharacter* PlayerCharacter)
{
	Interactors.RemoveSingle(PlayerCharacter);
	OnEndInteract.Broadcast(PlayerCharacter);
}


void UInteractionComponent::Interact(class APlayerCharacter* PlayerCharacter)
{
	OnInteract.Broadcast(PlayerCharacter);
}


// returns how far along into the act of interaction the player is (for interactions that require holds)
float UInteractionComponent::GetInteractPercentage()
{
	if (Interactors.IsValidIndex(0))
	{
		if (APlayerCharacter* Interactor = Interactors[0])
		{
			if (Interactor && Interactor->IsInteracting())
			{ return 1.0f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractionTime); }
		}
	}

	return 0.0f;
}


// sets name and refreshes widget
void UInteractionComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}


// sets action verb and refreshes widget
void UInteractionComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

