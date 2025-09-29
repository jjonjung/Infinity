// Fill out your copyright notice in the Description page of Project Settings.


#include "HpUi.h"

#include "Base/CharacterBase.h"

void UHpUi::NativeConstruct()
{
	ACharacterBase* ownerBase = Cast<ACharacterBase>(GetOwningPlayerPawn());
	if (ownerBase)
	{
		UE_LOG(LogTemp,Warning,TEXT("HpUi::NativeConstruct"));
	}
}
