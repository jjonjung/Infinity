// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionBase.h"
#include "Skill1_DoctorStrange.generated.h"


USTRUCT(BlueprintType)
struct FDoctorStrangeSkillSaveData
{
	GENERATED_BODY()


	FTransform PastTransform;

	
	int32 SavedHp = 0;
	
};
UCLASS()
class INFINITYFIGHTER_API USkill1_DoctorStrange : public UActionBase
{
    GENERATED_BODY()

public:
    USkill1_DoctorStrange();
    class AMyDoctorStrange* MyOwner;
    virtual bool CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const override;
    virtual void Activate_Implementation(ACharacterBase* Owner) override;
    void MoveToPast();

	UFUNCTION(BlueprintCallable)
	void StartTimeTravel();

    FTimerHandle skill1TimerHandle;
	UPROPERTY(EditDefaultsOnly)
	class UMaterial* timeMaterial;  
};
