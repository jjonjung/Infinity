// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Skill1_IronMan.generated.h"

/**
 * IronMan Hand Repulsors Skill - Physics-based implementation
 * 물리 기반 아이언맨 핸드 리펄서 스킬
 * - 손에서 독립적인 힘 발생 (F=ma)
 * - AddImpulseAtLocation으로 타겟에 힘 전달
 * - 반작용(Recoil)로 자신이 뒤로 밀림
 * - 최대 1000m 거리에서 넉백 효과
 */
UCLASS()
class INFINITYFIGHTER_API USkill1_IronMan : public UActionBase
{
	GENERATED_BODY()

public:
	USkill1_IronMan();

	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual bool PayCost_Implementation(ACharacterBase* Owner, FString& FailReason) override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	virtual void End_Implementation(ACharacterBase* Owner) override;
	virtual void Cancel_Implementation(ACharacterBase* Owner) override;
	
protected:
	// Hand Repulsors 물리 설정
	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Physics")
	float RepulsorImpulseForce = 8000.0f;  // 리펄서 임펄스 힘 (F = ma)

	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Physics")
	float RecoilForce = 1500.0f;  // 반작용 힘 (자신에게 가해지는 힘)

	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Physics")
	float MaxEffectiveRange = 100000.0f;  // 최대 효과 범위 (1000m = 100000cm)

	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Physics")
	float BeamRadius = 80.0f;  // 빔 반지름

	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Damage")
	float BaseDamage = 75.0f;  // 기본 데미지

	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Timing")
	float ChargeTime = 0.3f;  // 충전 시간

	UPROPERTY(EditDefaultsOnly, Category = "Hand Repulsors|Timing")
	float BeamDuration = 0.5f;  // 빔 지속시간

	// VFX 및 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UParticleSystem* RepulsorVFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class USoundBase* RepulsorSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	class UMaterial* ChargeEffectMaterial;

	// 소켓 이름
	UPROPERTY(EditDefaultsOnly, Category = "Hand Sockets")
	FName LeftHandSocketName = FName("hand_l");

	UPROPERTY(EditDefaultsOnly, Category = "Hand Sockets")
	FName RightHandSocketName = FName("hand_r");

	UPROPERTY() UMaterialInstanceDynamic* ChargeMID = nullptr;

private:
	class AMyIronMan* IronManOwner;
	FTimerHandle SkillTimerHandle;
	TArray<FHitResult> CachedHitResults;
	TSet<TWeakObjectPtr<AActor>> HitActorsThisShot;

	// 스킬 실행 함수들
	UFUNCTION()
	void ExecuteRepulsorBlast();

	// 물리 기반 리펄서 시스템
	void ApplyRecoilToSelf(const FVector& RepulsorDirection);
	void PerformRepulsorRaycast();
	void ApplyRepulsorImpulseToTarget(AActor* Target, const FVector& ImpulseLocation, const FVector& ImpulseDirection);

	// VFX 및 애니메이션
	void PlayRepulsorEffects();
	void PlayRepulsorAnimation();

	// 물리 계산 함수들
	FVector GetRepulsorDirection() const;
	FVector GetHandWorldLocation(bool bLeftHand = true) const;
	float CalculateDistanceBasedForce(float Distance) const;
};
