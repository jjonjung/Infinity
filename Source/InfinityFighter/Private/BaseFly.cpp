// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseFly.h"

#include "CharacterActionStatComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TimerManager.h"
#include "ActionComponent.h"

bool UBaseFly::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
    // Allow activation when NOT on cooldown (standard behavior)
    if (!Super::CanActivate_Implementation(Owner, FailReason))
    {
        return false;
    }
    // Optional: block if already flying
    if (Owner && Owner->GetCharacterMovement() && Owner->GetCharacterMovement()->IsFlying())
    {
        FailReason = TEXT("Already flying");
        return false;
    }
    return true;
}

void UBaseFly::Activate_Implementation(ACharacterBase* Owner)
{
    // Ensure this action persists until End/Cancel is called
    bAutoFinish = false;
    CachedOwner = Owner;
	Owner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	Owner->ActionStatComp->SetInFly(true);
	if (Owner->FlyingObject)
	{
		Owner->FlyingObject->SetHiddenInGame(false);
		Owner->FlyingObject->SetVisibility(true, true);
	}

	// Tune flying movement speed and control
	if (auto* Move = Owner->GetCharacterMovement())
	{
		Move->MaxFlySpeed = 1200.f;
		Move->BrakingDecelerationFlying = 2048.f;
		Move->AirControl = 1.0f;
	}

	// Limit flying to 5 seconds, then end
	Owner->GetWorldTimerManager().SetTimer(
		TimerHandle,
		FTimerDelegate::CreateWeakLambda(this, [this, Owner]()
		{
			// Defensive: ensure owner still valid
			if (IsValid(Owner))
			{
				EndFly();
			}
		}),
		5.0f, false);
	
}

void UBaseFly::EndFly()
{
    ACharacterBase* Owner = CachedOwner.Get();
    if (!Owner) return;

    if (Owner->GetWorldTimerManager().IsTimerActive(TimerHandle))
    {
        Owner->GetWorldTimerManager().ClearTimer(TimerHandle);
    }

    if (auto* Move = Owner->GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
    }
    Owner->ActionStatComp->SetInFly(false);
    if (Owner->FlyingObject)
    {
        Owner->FlyingObject->SetVisibility(false, true);
        Owner->FlyingObject->SetHiddenInGame(true);
    }

    if (UActionComponent* AC = Owner->ActionComponent)
    {
        AC->FinishActionNow(GetActionTag(), this);
    }
}

void UBaseFly::Cancel_Implementation(ACharacterBase* Owner)
{
    CachedOwner = Owner;
    EndFly();
}
