// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "AttachWall.h"
#include "DoublePressed.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API UDoublePressed : public UActionBase
{
	GENERATED_BODY()

public:
	virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
	virtual void Activate_Implementation(ACharacterBase* Owner) override;
	mutable AAttachWall* CurrentWall;
	mutable FVector CurrentWallNormal;
	mutable FVector HitPoint;
	FVector FixedWallTangent;
	void ClearWall();
	FRotator MakeParallel(ACharacterBase* Owner);
	
};
