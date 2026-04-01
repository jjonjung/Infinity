// Fill out your copyright notice in the Description page of Project Settings.


#include "Chatacter/MyDoctorStrange.h"
#include "CharacterActionStatComponent.h"
#include "Skill1_DoctorStrange.h"
#include "Camera/CameraActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Utility/ManagerController.h"


AMyDoctorStrange::AMyDoctorStrange()
{
    
}

void AMyDoctorStrange::SaveSkillData(FTransform PastTransform, int32 pastHp)
{

	FDoctorStrangeSkillSaveData saveData;
	saveData.PastTransform = PastTransform;
	saveData.SavedHp=pastHp;
	PushSnapshot(saveData);
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Black,TEXT("세이브 데이터 추가"));
	
}

void AMyDoctorStrange::RemoveLastData()
{
	FDoctorStrangeSkillSaveData removeItem;
	timeDatas.PopBack(removeItem);
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Black,TEXT("가장 최근에 넣은 데이터 제거"));
}

void AMyDoctorStrange::RemoveOldData()
{
	FDoctorStrangeSkillSaveData removeItem;
	timeDatas.PopFront(removeItem);
	//GEngine->AddOnScreenDebugMessage(-1,1.f,FColor::Black,TEXT("가장 오래된 데이터 제거"));
}


void AMyDoctorStrange::ClearQueue()
{
	timeDatas.Clear();
}

void AMyDoctorStrange::SaveDataAtTime()
{
    if (!CanSaveTime || !ActionStatComp)
    {
        return;
    }

    FTransform saveTransform = GetActorTransform();
    int32 saveHP = ActionStatComp->Get_CurrentHp();
    SaveSkillData(saveTransform,saveHP);
}

void AMyDoctorStrange::StopSaveTime()
{
	GetWorldTimerManager().ClearTimer(skillTimerHandle);
}

FDoctorStrangeSkillSaveData AMyDoctorStrange::RemoveForMove()
{
	FDoctorStrangeSkillSaveData moveTo;
	timeDatas.PopBack(moveTo);
	return moveTo;
}

void AMyDoctorStrange::StartSaveData()
{
    CanSaveTime = true;
    GetWorldTimerManager().SetTimer(skillTimerHandle,this,&AMyDoctorStrange::SaveDataAtTime, saveInterval, true);
}

void AMyDoctorStrange::PushSnapshot(const FDoctorStrangeSkillSaveData& SaveData)
{
    if (maxSavedFrames <= 0)
    {
        return;
    }

    while (timeDatas.Num() >= maxSavedFrames)
    {
        RemoveOldData();
    }

    timeDatas.PushBack(SaveData);
}


void AMyDoctorStrange::BeginPlay()
{
    Super::BeginPlay();
    // Register for match tracking if Blueprint assigned one
    if (MatchTeam == ETeam::None)
    {
        MatchTeam = ETeam::Red;
    }

    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GI = World->GetGameInstance())
        {
            if (UManagerController* MC = Cast<UManagerController>(GI))
            {
                if (MatchNickname != NAME_None)
                {
                    MC->RegisterPlayer(MatchNickname, MatchTeam);
                }
            }
        }
    }
    StartSaveData();
}

// ----------------- Rewind Smoothing -----------------

namespace
{
    static FORCEINLINE FQuat SlerpSafe(const FQuat& A, const FQuat& B, float Alpha)
    {
        FQuat Result = FQuat::Slerp(A, B, Alpha);
        Result.Normalize();
        return Result;
    }
}

void AMyDoctorStrange::StartRewindMode()
{
    bIsRewinding = true;
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->DisableMovement();
    }
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        bPrevIgnoreLookInput = PC->IsLookInputIgnored();
        SavedControlRotation = PC->GetControlRotation();
        bSavedControlRotationValid = true;
        PC->SetIgnoreLookInput(true); 
    }
}

void AMyDoctorStrange::StopRewindMode()
{
    bIsRewinding = false;
    rewindElapsed = 0.f;
    rewindDuration = 0.f;
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
    }
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->SetIgnoreLookInput(bPrevIgnoreLookInput); // restore previous state
        if (bSavedControlRotationValid)
        {
            PC->SetControlRotation(SavedControlRotation);
        }
    }
}

void AMyDoctorStrange::BeginRewindStep(const FTransform& TargetTransform, float Duration, int32 NewHp)
{
    if (!bIsRewinding)
    {
        StartRewindMode();
    }

    // Apply HP immediately to reflect rewind effect
    if (ActionStatComp)
    {
        ActionStatComp->Set_CurrentHp(NewHp);
    }

    rewindStartTransform = GetActorTransform();
    rewindTargetTransform = TargetTransform;
    rewindElapsed = 0.f;
    rewindDuration = FMath::Max(0.001f, Duration);
}

void AMyDoctorStrange::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsRewinding || rewindDuration <= 0.f)
    {
        return;
    }

    rewindElapsed += DeltaTime;
    const float Alpha = FMath::Clamp(rewindElapsed / rewindDuration, 0.f, 1.f);

    const FVector StartLoc = rewindStartTransform.GetLocation();
    const FVector TargetLoc = rewindTargetTransform.GetLocation();
    const FVector NewLoc = FMath::Lerp(StartLoc, TargetLoc, Alpha);

    // Move without sweep to avoid collision-induced jitter during rewind
    // Only update location; keep rotation/scale untouched
    SetActorLocation(NewLoc, false);

    if (Alpha >= 1.f)
    {
        // Snap exactly at the end of the step (location only)
        SetActorLocation(rewindTargetTransform.GetLocation(), false);
        rewindElapsed = 0.f;
        rewindDuration = 0.f;
    }
}


