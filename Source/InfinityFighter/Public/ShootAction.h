// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "BulletTest.h"
#include "Base/Projectile.h"
#include "ShootAction.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API UShootAction : public UActionBase
{
	GENERATED_BODY()

public:
	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual bool PayCost_Implementation(ACharacterBase* Owner, FString& FailReason) override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	virtual void End_Implementation(ACharacterBase* Owner) override;
	virtual void Cancel_Implementation(ACharacterBase* Owner) override;
public :
	UPROPERTY(EditAnywhere)
	int32 useBullet=1;// 한번 사용할떼 소모되는 탄창 양
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ProjectFactory; // 소환할 탄창
	

};
