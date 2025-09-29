// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Action_Test.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API UAction_Test : public UActionBase
{
	GENERATED_BODY()
	UAction_Test();
public:
	int32 testCount =3;
	int32 CurrentCount =0 ;
	
	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual bool PayCost_Implementation(ACharacterBase* Owner, FString& FailReason) override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	virtual void End_Implementation(ACharacterBase* Owner) override;
	virtual void Cancel_Implementation(ACharacterBase* Owner) override;
	
protected:
	
};
