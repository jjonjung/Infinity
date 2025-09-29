// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "EndingGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API AEndingGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	// 엔딩 위젯 (WBP_Ceremony 같은 UUserWidget BP 클래스)
	UPROPERTY(EditDefaultsOnly, Category="Ceremony")
	TSubclassOf<class UUserWidget> CeremonyWidgetClass;

	virtual void BeginPlay() override;
};
