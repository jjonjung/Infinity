// Fill out your copyright notice in the Description page of Project Settings.

#include "IronManRouter.h"
#include "CharacterActionStatComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"

bool UIronManRouter::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	// 기본 쿨다운 체크
	return !bOnCooldown;
}

bool UIronManRouter::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason)
{
	// 기본 ShootAction과 동일한 탄약 소모 로직
	int32 CurrentBulletBox = Owner->ActionStatComp->GetCurrentBulletBox();
	if (CurrentBulletBox >= useBullet)
	{
		Owner->ActionStatComp->SetCurrentBulletBox(CurrentBulletBox - useBullet);
		UE_LOG(LogTemp, Warning, TEXT("아이언맨 탄약 소모! 남은 탄약 = %d"), CurrentBulletBox - useBullet);
		return true;
	}
	else
	{
		FailReason = TEXT("탄약 부족");
		return false;
	}
}

void UIronManRouter::Activate_Implementation(ACharacterBase* Owner)
{
	AController* Controller = Owner->GetController();
	if (!Controller) return;

	// 카메라 컴포넌트 찾기
	UCameraComponent* Camera = Owner->FindComponentByClass<UCameraComponent>();
	if (!Camera) return;

	// hand_lSocket에서 발사 위치 가져오기
	FTransform SocketTransform;
	if (!GetSocketTransform(Owner, SocketTransform))
	{
		// 소켓을 찾을 수 없으면 기본 ShootAction 방식 사용
		UE_LOG(LogTemp, Warning, TEXT("hand_lSocket을 찾을 수 없어 기본 발사 방식을 사용합니다."));
		Super::Activate_Implementation(Owner);
		return;
	}

	// 화면 중앙에서 레이캐스트를 통해 실제 조준점 계산
	FVector CameraLocation = Camera->GetComponentLocation();
	FVector CameraForward = Camera->GetForwardVector();

	// 조준 레이캐스트 (화면 중앙에서 먼 거리까지)
	FVector TraceStart = CameraLocation;
	FVector TraceEnd = CameraLocation + (CameraForward * 10000.0f); // 10km 먼 거리

	FHitResult HitResult;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(Owner); // 자기 자신은 무시

	// 레이캐스트 실행
	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		CollisionParams
	);

	// 실제 조준점 결정 (히트했으면 히트 지점, 아니면 먼 거리)
	FVector TargetLocation = bHit ? HitResult.ImpactPoint : TraceEnd;

	// hand_lSocket 위치에서 조준점으로의 방향 계산
	FVector MuzzleLocation = SocketTransform.GetLocation();
	FVector FireDirection = (TargetLocation - MuzzleLocation).GetSafeNormal();
	FRotator FireRotation = UKismetMathLibrary::MakeRotFromX(FireDirection);

	// 스폰 트랜스폼 설정
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(MuzzleLocation);
	SpawnTransform.SetRotation(FireRotation.Quaternion());
	SpawnTransform.SetScale3D(FVector::OneVector);

    // 발사체 생성 (Owner/Instigator 지정)
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Cast<APawn>(Owner);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(ProjectFactory, SpawnTransform, SpawnParams);
    if (Projectile)
    {
        Projectile->SetProjectileOwner(Owner);
    }

	// 아이언맨 손 발사 애니메이션 재생
	PlayHandFireAnimation(Owner);

	// 디버그 로그
	UE_LOG(LogTemp, Warning, TEXT("아이언맨 손에서 발사: Target = %s, Direction = %s"),
		*TargetLocation.ToString(), *FireDirection.ToString());
}

void UIronManRouter::End_Implementation(ACharacterBase* Owner)
{
	// 기본 종료 처리
	Super::End_Implementation(Owner);
}

void UIronManRouter::Cancel_Implementation(ACharacterBase* Owner)
{
	// 기본 취소 처리
	Super::Cancel_Implementation(Owner);
}

bool UIronManRouter::GetSocketTransform(ACharacterBase* Owner, FTransform& OutTransform) const
{
	// 캐릭터의 메시 컴포넌트 가져오기
	USkeletalMeshComponent* MeshComp = Owner->GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComponent를 찾을 수 없습니다."));
		return false;
	}

	// hand_lSocket이 존재하는지 확인
	if (!MeshComp->DoesSocketExist(MuzzleSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("소켓 '%s'를 찾을 수 없습니다."), *MuzzleSocketName.ToString());
		return false;
	}

	// 소켓의 월드 트랜스폼 가져오기
	OutTransform = MeshComp->GetSocketTransform(MuzzleSocketName, RTS_World);
	return true;
}

void UIronManRouter::PlayHandFireAnimation(ACharacterBase* Owner) const
{
	// 애니메이션 몽타주가 설정되어 있으면 재생
	if (ActionMontage)
	{
		Owner->PlayActionMontage(ActionMontage);
		UE_LOG(LogTemp, Log, TEXT("아이언맨 손 발사 몽타주 재생"));
		return;
	}

	// 전용 HandFireMontage가 설정되어 있는지 확인
	if (!HandFireMontage.IsValid())
	{
		// 하드코딩된 몽타주 경로로 로드 시도
		const FString MontageePath = TEXT("/Script/Engine.AnimMontage'/Game/BluePrint/RealCharacter/IronMan/Anim/My_IronManironMan_HandFire_Anim_Montage.My_IronManironMan_HandFire_Anim_Montage'");
		UAnimMontage* AnimMontage = LoadObject<UAnimMontage>(nullptr, *MontageePath);

		if (!AnimMontage)
		{
			UE_LOG(LogTemp, Warning, TEXT("아이언맨 손 발사 몽타주를 로드할 수 없습니다: %s"), *MontageePath);
			return;
		}

		// 로드된 몽타주로 재생
		PlayAnimationMontage(Owner, AnimMontage);
		return;
	}

	// 설정된 애니메이션 몽타주 로드 및 재생
	UAnimMontage* AnimMontage = HandFireMontage.LoadSynchronous();
	if (AnimMontage)
	{
		PlayAnimationMontage(Owner, AnimMontage);
	}
}

void UIronManRouter::PlayAnimationMontage(ACharacterBase* Owner, UAnimMontage* AnimMontage) const
{
	if (!AnimMontage || !Owner) return;

	// 메시 컴포넌트와 애니메이션 인스턴스 가져오기
	USkeletalMeshComponent* MeshComp = Owner->GetMesh();
	if (!MeshComp)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkeletalMeshComponent를 찾을 수 없습니다."));
		return;
	}

	UAnimInstance* AnimInstance = MeshComp->GetAnimInstance();
	if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimInstance를 찾을 수 없습니다."));
		return;
	}

	// 애니메이션 몽타주 재생
	float MontageLength = AnimInstance->Montage_Play(AnimMontage, 1.0f);
	if (MontageLength > 0.0f)
	{
		UE_LOG(LogTemp, Log, TEXT("아이언맨 손 발사 몽타주 재생: %s (길이: %.2f초)"), *AnimMontage->GetName(), MontageLength);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("아이언맨 손 발사 몽타주 재생 실패: %s"), *AnimMontage->GetName());
	}
}
