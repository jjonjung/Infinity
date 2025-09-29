// Fill out your copyright notice in the Description page of Project Settings.


#include "Skill1_DoctorStrange.h"

#include "CharacterActionStatComponent.h"
#include "Chatacter/MyDoctorStrange.h"
#include "Channels/MovieSceneTimeWarpChannel.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "ActionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

USkill1_DoctorStrange::USkill1_DoctorStrange()
{
    bAutoFinish = false; // Let ActionComponent manage cooldown when we explicitly finish
}


bool USkill1_DoctorStrange::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	// Block activation while rewinding is in progress
	if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(skill1TimerHandle))
	{
		FailReason = TEXT("Rewinding in progress");
		return false;
	}
	
	return !bOnCooldown;
}

void USkill1_DoctorStrange::Activate_Implementation(ACharacterBase* Owner)
{
    
    MyOwner = Cast<AMyDoctorStrange>(Owner);
    MyOwner->StartRewindMode();
    MyOwner->StopSaveTime();
    MyOwner->PlayActionMontage(ActionMontage);

    
    // MyOwner->GetMesh()->SetOverlayMaterial(timeMaterial);
    // GetWorld()->GetTimerManager().SetTimer(skill1TimerHandle,this,&USkill1_DoctorStrange::MoveToPast, MyOwner->rewindStepDuration, true);
}



void USkill1_DoctorStrange::MoveToPast()
{
    
    if (!MyOwner->timeDatas.IsEmpty())
    {
        FDoctorStrangeSkillSaveData lastData = MyOwner->RemoveForMove();
        MyOwner->BeginRewindStep(lastData.PastTransform, MyOwner->rewindStepDuration, lastData.SavedHp);
        
    }
    else
    {
        
        GetWorld()->GetTimerManager().ClearTimer(skill1TimerHandle);
        MyOwner->StopRewindMode();
      
        if ( MyOwner && MyOwner->ActionComponent)
        {

			MyOwner->GetMesh()->SetOverlayMaterial(nullptr);
			MyOwner->StartSaveData();
			MyOwner->ClearQueue();
            MyOwner->ActionComponent->FinishActionNow(GetActionTag(), this);
        	MyOwner->StopRewindMode();
            
        }
       
        
    }
    
    
    
}

void USkill1_DoctorStrange::StartTimeTravel()
{
	MyOwner->GetMesh()->SetOverlayMaterial(timeMaterial);
	GetWorld()->GetTimerManager().SetTimer(skill1TimerHandle,this,&USkill1_DoctorStrange::MoveToPast, MyOwner->rewindStepDuration, true);
}
