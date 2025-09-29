// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/CBrainComponent.h"
#include "DA/EnemyAIData.h"
#include "AiBrainComponent.generated.h"

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	Idle UMETA(DisplayName = "IdleState"),
	Move,
	Attack,
	Damage,
	Die,
};

UENUM(BlueprintType)
enum class EAIMovementPattern : uint8
{
	ChaoticMovement UMETA(DisplayName = "Chaotic Movement + Fire"),
	StrafingJump UMETA(DisplayName = "Strafe + Jump + Skill1"),
	CoverAttack UMETA(DisplayName = "Cover + Attack")
};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class INFINITYFIGHTER_API UAiBrainComponent : public UCBrainComponent
{
	GENERATED_BODY()

public:
	UAiBrainComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// ========== AI DataAsset 설정 ==========
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Brain|AI|Config")
	UEnemyAIData* AIData;

	
	// AI 상태 머신 관련 속성들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain|AI|FSM")
	EEnemyState CurrentEnemyState = EEnemyState::Idle;

	UPROPERTY(BlueprintReadOnly, Category="Brain|AI|FSM")
	int32 CurrentHP = 3;

	// AI 패턴 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Brain|AI|Pattern", Meta = (DisplayName = "AI Movement Pattern"))
	EAIMovementPattern CurrentPattern = EAIMovementPattern::ChaoticMovement; // 0: 혼란스런 이동+Fire, 1: 좌우이동+점프+Skill1, 2: 엄폐+공격

	// 회피 및 은신 관련
	UPROPERTY(BlueprintReadOnly, Category="Brain|AI|Dodge")
	bool bIsPlayerAttacking = false;

	UPROPERTY(BlueprintReadOnly, Category="Brain|AI|Dodge")
	float LastDodgeTime = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Brain|AI|Hide")
	bool bIsHiding = false;

	UPROPERTY(BlueprintReadOnly, Category="Brain|AI|Hide")
	FVector HidePosition;

	// 내부 타이머
	float CurrentTime = 0.0f;
	FVector KnockbackTargetPos;

	// AI 상태 머신 함수들
	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	void ExecuteCurrentState(EEnemyState NewState);

	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM") 
	void UpdateAIStateMachine(EEnemyState NewState);

	// 각 상태별 처리 함수들
	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	void IdleState();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	void MoveState();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	void AttackState();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	void DamageState();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	void DieState();

	// 거리 계산 헬퍼 함수
	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	bool IsInAttackRange() const;

	UFUNCTION(BlueprintCallable, Category="Brain|AI|FSM")
	float GetDistanceToTarget() const;

	// 피해 처리
	UFUNCTION(BlueprintCallable, Category="Brain|Damage")
	void OnDamageProcess(const FVector& HitDirection, float DamageAmount = 1.0f);

	// 회피 및 은신 함수들
	UFUNCTION(BlueprintCallable, Category="Brain|AI|Dodge")
	void OnPlayerAttackDetected();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|Dodge")
	void ExecuteDodgeMovement();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|Hide")
	bool FindHidingSpot();

	UFUNCTION(BlueprintCallable, Category="Brain|AI|Hide")
	void ExecuteHidingBehavior();

	// 새로운 패턴별 행동 함수들
	void ExecuteChaoticMovementPattern(ACharacter* Character, const FVector& ToTarget, float Distance);
	void ExecuteStrafingJumpPattern(ACharacter* Character, const FVector& ToTarget, float Distance);
	void ExecuteCoverAttackPattern(ACharacter* Character, const FVector& ToTarget, float Distance);

	// AI 초기화 헬퍼 함수
	void InitializeAI();

private:
	// SpiderMan과 IronMan 상호작용 헬퍼 함수들
	void CheckForSpiderManPullAndTriggerIronManSkill(ACharacter* Character);
	bool IsSpiderManUsingPullSkill(ACharacter* SpiderManCharacter);
	void TriggerIronManSkill(ACharacter* IronManCharacter);
};