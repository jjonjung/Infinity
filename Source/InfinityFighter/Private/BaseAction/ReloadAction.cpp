// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseAction//ReloadAction.h"

#include <gsl/pointers>

#include "CharacterActionStatComponent.h"
#include "Base/CharacterBase.h"

bool UReloadAction::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	return Super::CanActivate_Implementation(Owner, FailReason);
}

bool UReloadAction::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason)
{
	UE_LOG(LogTemp,Warning,TEXT("재장전 조건 검사"));
	if (Owner->ActionStatComp->GetCurrentBullet()>0)
	{
		return true;
	}
	return false;
}

void UReloadAction::Activate_Implementation(ACharacterBase* Owner)
{
	UE_LOG(LogTemp,Warning,TEXT("재장전 실행"));
	int32 CurrentBulllet = Owner->ActionStatComp->GetCurrentBullet();
	int32 OneBulletBox = Owner->ActionStatComp->GetBulletBox();
	Owner->ActionStatComp->SetCurrentBulletBox(FMath::Min(CurrentBulllet, OneBulletBox));
}

void UReloadAction::End_Implementation(ACharacterBase* Owner)
{
	Super::End_Implementation(Owner);
}

void UReloadAction::Cancel_Implementation(ACharacterBase* Owner)
{
	Super::Cancel_Implementation(Owner);
}
