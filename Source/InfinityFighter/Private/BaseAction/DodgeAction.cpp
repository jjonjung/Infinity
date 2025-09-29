// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAction/DodgeAction.h"

#include "ActionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UDodgeAction::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	 if (!bOnCooldown) // 쿨다운이 아니면서 
	 {
	 	const FName SlotName = FName("DefaultSlot");
		 if (!Owner->GetMesh()->GetAnimInstance()->Montage_IsPlaying(ActionMontage)) // 몽타주 실행중이 아니라면
		 {
			 return true;
		 }
		 else
		 {
			 return false;
		 };
	 }
	 return false;
}

bool UDodgeAction::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason)
{
	UE_LOG(LogTemp,Warning,TEXT("구르기 조건 검사"));
	return true;
}

void UDodgeAction::Activate_Implementation(ACharacterBase* Owner)
{
	
		UE_LOG(LogTemp,Warning,TEXT("아 제발~"));
		
		auto* montage  = GetActionMontage();
		
		Owner->PlayActionMontage(montage);
	
		if (montage ==nullptr)
		{
			UE_LOG(LogTemp,Warning,TEXT("어이"));
		}
		UE_LOG(LogTemp,Warning,TEXT("구르기 실행"));
	
	
}

void UDodgeAction::End_Implementation(ACharacterBase* Owner)
{
	Super::End_Implementation(Owner);
}

void UDodgeAction::Cancel_Implementation(ACharacterBase* Owner)
{
	UE_LOG(LogTemp,Warning,TEXT("구르기 방해됨"));
	Super::Cancel_Implementation(Owner);
}
