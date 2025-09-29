// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAction//JumpAction.h"

#include "GameFramework/CharacterMovementComponent.h"

bool UJumpAction::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	return Super::CanActivate_Implementation(Owner, FailReason);
}

bool UJumpAction::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason)
{
	return Super::PayCost_Implementation(Owner, FailReason);
}

void UJumpAction::Activate_Implementation(ACharacterBase* Owner)
{
	Owner->Jump();
}

void UJumpAction::End_Implementation(ACharacterBase* Owner)
{
	Super::End_Implementation(Owner);
}

void UJumpAction::Cancel_Implementation(ACharacterBase* Owner)
{
	Super::Cancel_Implementation(Owner);
}
