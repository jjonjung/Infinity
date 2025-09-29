// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "BaseFly.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API UBaseFly : public UActionBase
{
    GENERATED_BODY()

public:
    virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
    virtual void Activate_Implementation(ACharacterBase* Owner) override;
    virtual void Cancel_Implementation(ACharacterBase* Owner) override;
    FTimerHandle TimerHandle;
    void EndFly();
private:
    TWeakObjectPtr<ACharacterBase> CachedOwner;
};
