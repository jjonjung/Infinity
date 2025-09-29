// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ShootAction.h"
#include "IronManRouter.generated.h"

/**
 * 아이언맨 전용 ShootAction - hand_lSocket에서 총알을 발사하고 전용 애니메이션을 재생
 */
UCLASS()
class INFINITYFIGHTER_API UIronManRouter : public UShootAction
{
	GENERATED_BODY()

public:
	// ActionBase 가상 함수들 재정의
	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual bool PayCost_Implementation(ACharacterBase* Owner, FString& FailReason) override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	virtual void End_Implementation(ACharacterBase* Owner) override;
	virtual void Cancel_Implementation(ACharacterBase* Owner) override;

	// 총알 발사 위치 소켓 이름 (hand_lSocket)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IronMan")
	FName MuzzleSocketName = TEXT("hand_lSocket");

	// 손 발사 애니메이션 몽타주 에셋 참조
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IronMan")
	TSoftObjectPtr<UAnimMontage> HandFireMontage;

protected:

private:
	// 소켓 위치와 회전을 가져오는 헬퍼 함수
	bool GetSocketTransform(ACharacterBase* Owner, FTransform& OutTransform) const;

	// 손 발사 애니메이션을 재생하는 헬퍼 함수
	void PlayHandFireAnimation(ACharacterBase* Owner) const;

	// 애니메이션 몽타주를 재생하는 헬퍼 함수
	void PlayAnimationMontage(ACharacterBase* Owner, UAnimMontage* AnimMontage) const;
};