// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "MySpiderMan.generated.h"

class USkill1_SpiderManSwing;

/**
 *
 */
UCLASS()
class INFINITYFIGHTER_API AMySpiderMan : public ACharacterBase
{
	GENERATED_BODY()

public:
	AMySpiderMan();
	virtual void BeginPlay() override;

protected:
	// 스파이더맨 액션들 설정
	UFUNCTION()
	void SetupSpiderManActions();

	// 입력 이벤트 처리
	UFUNCTION()
	void OnInputIntent(FName IntentTag);
};
