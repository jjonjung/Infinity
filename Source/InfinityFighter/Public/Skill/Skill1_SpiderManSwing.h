// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Character.h"
#include "Skill1_SpiderManSwing.generated.h"

// 전방 선언
class USkeletalMeshComponent;

/**
 * 스파이더맨 스타일 스윙 액션
 */
UCLASS(Blueprintable, BlueprintType)
class INFINITYFIGHTER_API USkill1_SpiderManSwing : public UActionBase
{
	GENERATED_BODY()

public:
	USkill1_SpiderManSwing();

	// UActionBase 오버라이드
	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	virtual void End_Implementation(ACharacterBase* Owner) override;
	virtual void Cancel_Implementation(ACharacterBase* Owner) override;

protected:
	// 스윙 관련 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float MaxSwingDistance = 1500.0f; // 최대 웹 거리

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float SwingForce = 2000.0f; // 스윙 힘

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float ReleaseVelocityMultiplier = 1.5f; // 놓을 때 속도 배수

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float SwingDamping = 0.98f; // 스윙 감쇠

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Settings")
	float MinHeightForSwing = 100.0f; // 스윙 최소 높이

	// 공격 관련 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float SwingDamage = 50.0f; // 스윙 공격 데미지

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float AttackRadius = 100.0f; // 공격 범위

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float KnockbackForce = 1000.0f; // 넉백 힘

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Attack")
	float MinAttackVelocity = 500.0f; // 공격 최소 속도

	// 현재 스윙 상태
	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	bool bIsSwinging = false;

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	FVector SwingAnchorPoint; // 웹이 붙은 위치

	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	float CurrentRopeLength = 0.0f;

	// 타겟 적 (끌어오기용)
	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	TWeakObjectPtr<ACharacterBase> TargetEnemy;

	// 벽 붙기 상태
	UPROPERTY(BlueprintReadOnly, Category = "Swing|State")
	bool bIsWallCrawling = false;

	// 스윙 물리
	FVector SwingVelocity;
	float SwingAngle = 0.0f;
	float SwingAngularVelocity = 0.0f;

	// 웹 라인 렌더링 컴포넌트 (런타임에서 생성)
	UPROPERTY()
	UStaticMeshComponent* WebLineComponent;

	// 웹라인 메시 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Swing|Visuals")
	int32 MeshSegmentCount = 10; // 웹라인을 따라 배치할 메시 개수

	// 타이머 핸들
	FTimerHandle SwingUpdateHandle;

	// 공격 관련 상태
	UPROPERTY(BlueprintReadOnly, Category = "Swing|Attack")
	TArray<AActor*> HitActors; // 이미 공격한 액터들 (중복 공격 방지)

private:
	// Skeletal Mesh 관련
	UPROPERTY()
	USkeletalMesh* WebLineMesh; // SKM_BoundingBoxEdge 메시

	UPROPERTY()
	TArray<USkeletalMeshComponent*> WebLineMeshComponents; // 웹라인을 따라 배치되는 메시들
	
private:
	// 스윙 관련 함수들
	bool FindSwingTarget(ACharacterBase* Owner, FVector& OutAnchorPoint);
	bool FindEnemyTarget(ACharacterBase* Owner, FVector& OutAnchorPoint);
	void StartSwinging(ACharacterBase* Owner);
	void UpdateSwing(ACharacterBase* Owner);
	void EndSwinging(ACharacterBase* Owner);
	void CreateWebLine(ACharacterBase* Owner);
	void UpdateWebLineVisual(ACharacterBase* Owner);
	void DestroyWebLine();

	// 웹라인 메시 관련 함수들
	void CreateWebLineMeshes(ACharacterBase* Owner);
	void UpdateWebLineMeshes(ACharacterBase* Owner);
	void DestroyWebLineMeshes();

	// 벽 붙기 및 적 끌어오기 함수들
	void StartWallCrawling(ACharacterBase* Owner);
	void UpdateWallCrawling(ACharacterBase* Owner);
	void StartEnemyPull(ACharacterBase* Owner);
	void UpdateEnemyPull(ACharacterBase* Owner);

	// 물리 계산
	void CalculateSwingPhysics(ACharacterBase* Owner, float DeltaTime);
	FVector GetSwingDirection(const FVector& PlayerPos, const FVector& AnchorPos) const;

	// 공격 관련 함수들
	void CheckSwingAttack(ACharacterBase* Owner);
	void PerformSwingAttack(ACharacterBase* Owner, ACharacterBase* Target);
	bool CanAttackTarget(ACharacterBase* Owner, ACharacterBase* Target) const;
};