// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Base/CharacterBase.h"
#include "utility/TDeque.h"
#include "Skill1_DoctorStrange.h"
#include "MyDoctorStrange.generated.h"

/**
 * 
 */
UCLASS()
class INFINITYFIGHTER_API AMyDoctorStrange : public ACharacterBase
{
    GENERATED_BODY()

public :
	
	int32 qSize =0;

    TDeque<FDoctorStrangeSkillSaveData> timeDatas;
	
	AMyDoctorStrange();
	void SaveSkillData(FTransform PastTransform,int32 pastHp);
	void RemoveLastData();
	void RemoveOldData();
	void ClearQueue();
    void SaveDataAtTime();
    void StopSaveTime();
    FDoctorStrangeSkillSaveData RemoveForMove();
    void StartSaveData();
    
    // Rewind smoothing controls
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TimeRewind")
    float saveInterval = 0.05f; // how often to sample while recording

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TimeRewind")
    float rewindStepDuration = 0.05f; // duration to blend to each snapshot during rewind

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TimeRewind")
    int32 maxSavedFrames = 80; // history depth

    // Begin one interpolation step toward TargetTransform during rewind
    void BeginRewindStep(const FTransform& TargetTransform, float Duration, int32 NewHp);
    // Toggle rewinding mode (enable/disable movement etc.)
    void StartRewindMode();
    void StopRewindMode();

private:
    // Internal rewind state (per-character)
    bool bIsRewinding = false;
    FTransform rewindStartTransform;
    FTransform rewindTargetTransform;
    float rewindElapsed = 0.f;
    float rewindDuration = 0.f;
    bool bPrevIgnoreLookInput = false;
    bool bSavedControlRotationValid = false;
    FRotator SavedControlRotation;
	

	FTimerHandle skillTimerHandle;
	
	uint32 CurrentIndex = 0;
	bool CanSaveTime=true;

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;



};
