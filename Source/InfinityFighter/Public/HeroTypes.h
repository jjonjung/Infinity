// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HeroTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EHeroType : uint8
{
	IronMan        UMETA(DisplayName="IronMan"),
	DoctorStrange  UMETA(DisplayName="DoctorStrange"),
	SpiderMan      UMETA(DisplayName="SpiderMan")
};

UCLASS()
class INFINITYFIGHTER_API UHeroTypes : public UGameInstance
{
	GENERATED_BODY()
};
