// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HpUi.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API UHpUi : public UUserWidget
{
	GENERATED_BODY()
public :
	virtual void NativeConstruct() override;
	UFUNCTION(BlueprintImplementableEvent)
	void HpUiUpdate(float hpPercent);
};
