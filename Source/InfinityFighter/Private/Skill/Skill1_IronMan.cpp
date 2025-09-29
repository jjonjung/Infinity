// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/Skill1_IronMan.h"
#include "Base/CharacterBase.h"
#include "Chatacter/MyIronMan.h"
#include "CharacterActionStatComponent.h"
#include "ActionComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

USkill1_IronMan::USkill1_IronMan()
{
    // ActionBase 기본 설정
    bAutoFinish = false;  // 수동으로 종료 관리
    SetCooldown(3.0f);    // 3초 쿨다운

    // 리펄서 물리 설정
    RepulsorImpulseForce = 8000.0f;
    RecoilForce = 1500.0f;
    MaxEffectiveRange = 100000.0f;  // 1000m
    BeamRadius = 80.0f;
    BaseDamage = 75.0f;
    ChargeTime = 0.3f;
    BeamDuration = 0.5f;

    // 소켓 이름
    LeftHandSocketName = FName("hand_l");
    RightHandSocketName = FName("hand_r");

    // 애니메이션 몽타주 설정
    static ConstructorHelpers::FObjectFinder<UAnimMontage> HandFireMontageRef(
        TEXT("/Script/Engine.AnimMontage'/Game/BluePrint/RealCharacter/IronMan/MyIronManironMan_HandFire_Anim_Montage.MyIronManironMan_HandFire_Anim_Montage'")
    );
    if (HandFireMontageRef.Succeeded())
    {
        ActionMontage = HandFireMontageRef.Object;
    }

    IronManOwner = nullptr;
}


void USkill1_IronMan::Activate_Implementation(ACharacterBase* Owner)
{
    

    IronManOwner = Cast<AMyIronMan>(Owner);
    if (!IronManOwner)
    {
        UE_LOG(LogTemp, Error, TEXT("Hand Repulsors: Owner is not IronMan!"));
        return;
    }

    // 애니메이션 재생
    //PlayRepulsorAnimation();
    Owner->PlayActionMontage(ActionMontage);

    // 충전 효과
    if (ChargeEffectMaterial && IronManOwner->GetMesh())
    {
        ChargeMID = UMaterialInstanceDynamic::Create(ChargeEffectMaterial, this);
        if (ChargeMID)
        {
            ChargeMID->SetScalarParameterValue(TEXT("pression"), 10.0f);
            IronManOwner->GetMesh()->SetOverlayMaterial(ChargeMID);
        }
    }

    // 충전 시간 후 리펄서 발사
    GetWorld()->GetTimerManager().SetTimer(SkillTimerHandle, this,
        &USkill1_IronMan::ExecuteRepulsorBlast, ChargeTime, false);

    UE_LOG(LogTemp, Warning, TEXT("IronMan Hand Repulsors Activated!"));
}

bool USkill1_IronMan::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
    // ActionBase의 기본 검사 (쿨다운 체크)
    if (!Super::CanActivate_Implementation(Owner, FailReason))
    {
        return false;
    }

    // 스킬이 이미 실행 중인지 체크
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(SkillTimerHandle))
    {
        FailReason = TEXT("Hand Repulsors already active");
        return false;
    }

    // IronMan인지 체크
    if (!Cast<AMyIronMan>(Owner))
    {
        FailReason = TEXT("Only IronMan can use Hand Repulsors");
        return false;
    }

    return true;
}

bool USkill1_IronMan::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason)
{
    // ActionBase 기본 비용 체크
    if (!Super::PayCost_Implementation(Owner, FailReason))
    {
        return false;
    }

    // 추가 비용 체크 (예: 에너지, 마나 등)
    // 현재는 비용 없음
    return true;
}


void USkill1_IronMan::ExecuteRepulsorBlast()
{
    if (!IronManOwner)
    {
        return;
    }

    // 1. 리펄서 방향 계산 (손에서 전방으로)
    FVector RepulsorDirection = GetRepulsorDirection();

    // 2. 반작용 적용 (자신을 뒤로 밀어냄)
    ApplyRecoilToSelf(RepulsorDirection);

    // 3. 레이캐스트로 타겟 탐지 및 물리적 힘 전달
    PerformRepulsorRaycast();

    // 4. VFX 및 SFX 재생
    PlayRepulsorEffects();

    // 5. 빔 지속시간 후 종료
    GetWorld()->GetTimerManager().SetTimer(SkillTimerHandle, [this]()
    {
        End_Implementation(IronManOwner);
    }, BeamDuration, false);

    UE_LOG(LogTemp, Warning, TEXT("리펄서 빔 발사! 방향: %s"), *RepulsorDirection.ToString());
}

void USkill1_IronMan::ApplyRecoilToSelf(const FVector& RepulsorDirection)
{
    if (!IronManOwner || !IronManOwner->GetCharacterMovement())
    {
        return;
    }

    // 반작용 대 솔 제3법칙: 모든 작용에는 반대의 반작용이 존재
    FVector RecoilDirection = -RepulsorDirection;
    FVector RecoilImpulse = RecoilDirection * RecoilForce;

    // 캐릭터 무브먼트에 임펄스 적용
    IronManOwner->GetCharacterMovement()->AddImpulse(RecoilImpulse, true);

    UE_LOG(LogTemp, Warning, TEXT("반작용 적용: %s"), *RecoilImpulse.ToString());
}

void USkill1_IronMan::PerformRepulsorRaycast()
{
    if (!IronManOwner)
    {
        return;
    }

    // 손 위치에서 레이캐스트 시작
    FVector LeftHandLocation = GetHandWorldLocation(true);   // 왼손
    FVector RightHandLocation = GetHandWorldLocation(false); // 오른손
    FVector RepulsorDirection = GetRepulsorDirection();

    // 두 손에서 레이캐스트 수행
    TArray<FVector> HandLocations = {LeftHandLocation, RightHandLocation};

    for (const FVector& HandLocation : HandLocations)
    {
        FVector EndLocation = HandLocation + (RepulsorDirection * MaxEffectiveRange);

        // 레이캐스트 설정
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(IronManOwner);
        QueryParams.bTraceComplex = false;
        QueryParams.bReturnPhysicalMaterial = true;

        // 스피어 스윕 레이캐스트 (빔 두께 시뮬레이션)
        TArray<FHitResult> HitResults;
        bool bHit = GetWorld()->SweepMultiByChannel(
            HitResults,
            HandLocation,
            EndLocation,
            FQuat::Identity,
            ECC_Pawn,
            FCollisionShape::MakeSphere(BeamRadius),
            QueryParams
        );

        if (bHit)
        {
            for (const FHitResult& Hit : HitResults)
            {
                AActor* HitActor = Hit.GetActor();
                if (!HitActor || HitActor == IronManOwner)
                {
                    continue;
                }

                // 거리 상관없이 최대 힘 전달 (리펄서는 에너지 빔)
                float Distance = FVector::Dist(HandLocation, Hit.Location);
                float ForceToApply = CalculateDistanceBasedForce(Distance);

                // 데미지 적용
                if (ACharacterBase* TargetCharacter = Cast<ACharacterBase>(HitActor))
                {
                    TargetCharacter->TakeDamage(BaseDamage, IronManOwner);
                }

                // 물리적 임펄스 전달
                ApplyRepulsorImpulseToTarget(HitActor, Hit.Location, RepulsorDirection);

                UE_LOG(LogTemp, Warning, TEXT("리펄서 타격: %s, 거리: %.1fcm, 힘: %.1f"),
                    *HitActor->GetName(), Distance, ForceToApply);
            }
        }

        /*// 디버그 시각화
        #if WITH_EDITOR
        DrawDebugLine(GetWorld(), HandLocation, EndLocation, FColor::Cyan, false, 2.0f, 0, 5.0f);
        DrawDebugSphere(GetWorld(), HandLocation, BeamRadius, 12, FColor::Red, false, 2.0f);
        #endif*/
    }
}

void USkill1_IronMan::ApplyRepulsorImpulseToTarget(AActor* Target, const FVector& ImpulseLocation, const FVector& ImpulseDirection)
{
    if (!Target)
    {
        return;
    }

    // 임펄스 벡터 계산 - AddImpulseAtLocation 사용
    FVector ImpulseVector = ImpulseDirection.GetSafeNormal() * RepulsorImpulseForce;

    // 캐릭터인 경우 CharacterMovement로 적용
    if (ACharacter* Character = Cast<ACharacter>(Target))
    {
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            // CharacterMovement는 AddImpulseAtLocation을 지원하지 않으므로 AddImpulse 사용
            MovementComp->AddImpulse(ImpulseVector, true);
        }
    }
    // 일반 액터(물리 오브젝트)인 경우
    else if (UPrimitiveComponent* PrimitiveComp = Target->FindComponentByClass<UPrimitiveComponent>())
    {
        if (PrimitiveComp->IsSimulatingPhysics())
        {
            // AddImpulseAtLocation 사용 - 정확한 지점에 힘 전달
            PrimitiveComp->AddImpulseAtLocation(ImpulseVector, ImpulseLocation);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("타겟 %s에 리펄서 임펄스 전달: %s (위치: %s)"),
        *Target->GetName(), *ImpulseVector.ToString(), *ImpulseLocation.ToString());
}

void USkill1_IronMan::PlayRepulsorEffects()
{
    if (!IronManOwner)
    {
        return;
    }

    // VFX 생성 - P_ky_laser01 사용
    if (RepulsorVFX)
    {
        FVector LeftHandLocation = GetHandWorldLocation(true);
        FVector RightHandLocation = GetHandWorldLocation(false);
        FVector RepulsorDirection = GetRepulsorDirection();

        // 두 손에서 VFX 생성
        FRotator EffectRotation = RepulsorDirection.Rotation();
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), RepulsorVFX, LeftHandLocation, EffectRotation);
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), RepulsorVFX, RightHandLocation, EffectRotation);
    }

    // SFX 재생
    if (RepulsorSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(GetWorld(), RepulsorSFX, IronManOwner->GetActorLocation());
    }
}

void USkill1_IronMan::PlayRepulsorAnimation()
{
    if (!IronManOwner) return;

    if (ActionMontage)
    {
        IronManOwner->PlayActionMontage(ActionMontage);
        UE_LOG(LogTemp, Log, TEXT("리펄서 애니메이션 재생"));
    }
    if (!ActionMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("리펄서 애니메이션 없음"));
    }
}

FVector USkill1_IronMan::GetRepulsorDirection() const
{
    if (!IronManOwner)
    {
        return FVector::ForwardVector;
    }

    // 아이언맨이 바라보는 방향 (손에서 나가는 방향)
    return IronManOwner->GetActorForwardVector();
}

FVector USkill1_IronMan::GetHandWorldLocation(bool bLeftHand) const
{
    if (!IronManOwner || !IronManOwner->GetMesh())
    {
        return FVector::ZeroVector;
    }

    FName SocketName = bLeftHand ? LeftHandSocketName : RightHandSocketName;
    return IronManOwner->GetMesh()->GetSocketLocation(SocketName);
}

float USkill1_IronMan::CalculateDistanceBasedForce(float Distance) const
{
    // 리펄서는 에너지 빔이므로 거리에 상관없이 일정한 힘
    // 단, 최대 범위 내에서만 효과적
    if (Distance > MaxEffectiveRange)
    {
        return 0.0f;
    }

    // 거리에 따른 약간의 감쇠 (현실적인 물리 효과)
    float DistanceRatio = Distance / MaxEffectiveRange;
    float ForceMultiplier = FMath::Clamp(1.0f - (DistanceRatio * 0.3f), 0.7f, 1.0f);

    return RepulsorImpulseForce * ForceMultiplier;
}

void USkill1_IronMan::End_Implementation(ACharacterBase* Owner)
{
    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SkillTimerHandle);
    }

    // 시각적 효과 제거
    if (IronManOwner && IronManOwner->GetMesh())
    {
        IronManOwner->GetMesh()->SetOverlayMaterial(nullptr);
    }

    // 동적 머티리얼 정리
    if (ChargeMID)
    {
        ChargeMID->SetScalarParameterValue(TEXT("pression"), 1.0f);
        ChargeMID = nullptr;
    }

    // ActionComponent에 완료 알림
    if (IronManOwner && IronManOwner->ActionComponent)
    {
        IronManOwner->ActionComponent->FinishActionNow(GetActionTag(), this);
    }

    UE_LOG(LogTemp, Warning, TEXT("Hand Repulsors Skill Ended"));

    // ActionBase 기본 종료 처리 (델리게이트 브로드캐스트, 쿨다운 시작)
    Super::End_Implementation(Owner);
}

void USkill1_IronMan::Cancel_Implementation(ACharacterBase* Owner)
{
    // 타이머 정리
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(SkillTimerHandle);
    }

    // 시각적 효과 제거
    if (IronManOwner && IronManOwner->GetMesh())
    {
        IronManOwner->GetMesh()->SetOverlayMaterial(nullptr);
    }

    // 동적 머티리얼 정리
    if (ChargeMID)
    {
        ChargeMID->SetScalarParameterValue(TEXT("pression"), 1.0f);
        ChargeMID = nullptr;
    }

    UE_LOG(LogTemp, Warning, TEXT("Hand Repulsors Skill Cancelled"));

    // ActionBase 기본 취소 처리 (델리게이트 브로드캐스트)
    Super::Cancel_Implementation(Owner);
}
