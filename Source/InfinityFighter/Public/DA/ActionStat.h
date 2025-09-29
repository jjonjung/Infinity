// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ActionStat.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API UActionStat : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|State")
	float AttackDamage;//기본 공격 데미지 
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|State")
	float MaxHP;// 최대 HP
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|State")
	int32 MaxBullet;// 총 가지고 있을 수 있는 탄창 
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|State")
	int32 BulletBox;//한탄창에 얼마나 쏠 수 있는가
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category="Action|State")
	float AttackSpeed; //1초에 몇발?
	
	
	
};
