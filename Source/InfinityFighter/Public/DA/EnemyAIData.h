// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnemyAIData.generated.h"

/**
 * Enemy AI FSM 설정을 위한 DataAsset
 * 다양한 난이도와 AI 타입별로 설정 가능
 */
UCLASS(BlueprintType, Blueprintable)
class INFINITYFIGHTER_API UEnemyAIData : public UDataAsset
{
	GENERATED_BODY()

public:
	UEnemyAIData();

	// ========== 기본 스탯 ==========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Stats")
	int32 MaxHP = 3;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Stats")
	float AIMoveScale = 1.0f;

	// ========== 공격 관련 ==========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Attack")
	float AttackDelayTime = 1.0f;

	// ========== 상태 관련 ==========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="States")
	float IdleDelayTime = 0.5f;

	// ========== 피해 관련 ==========
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	float KnockbackPower = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Damage")
	float DamageDelayTime = 1.5f;
};