// Copyright 2022 Andrew Creekmore, Danny Chung, Brittany Legget


#include "../UserInterface/InteractionWidget.h"
#include "../Components/InteractionComponent.h"


void UInteractionWidget::UpdateInteractionWidget(class UInteractionComponent* InteractionComponent)
{
	OwningInteractionComponent = InteractionComponent;
}
