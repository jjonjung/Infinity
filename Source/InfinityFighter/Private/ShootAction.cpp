// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootAction.h"

#include <gsl/pointers>
#include "DrawDebugHelpers.h"
#include "CharacterActionStatComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"

bool UShootAction::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	return !bOnCooldown;
}

bool UShootAction::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason)
{
	int32 CurrentBulletBox = Owner->ActionStatComp->GetCurrentBulletBox();
	if (CurrentBulletBox>= useBullet)
	{
		Owner->ActionStatComp->SetCurrentBulletBox(CurrentBulletBox-useBullet);
		UE_LOG(LogTemp, Warning, TEXT("MyInt changed! New Value = %d"), CurrentBulletBox-1);
		return true;
	}
	else
	{
		return false;
	}
}

static const FName  MuzzleSkt   = TEXT("Muzzle");
static const float  TraceDist   = 100000.f;
static const float  SpawnOffset = 10.f;   // Mesh 앞 10
static const float  ChestHeight = 120.f;  // 가슴 높이 (조정 가능)

void UShootAction::Activate_Implementation(ACharacterBase* Owner)
{
	if (!Owner || !Owner->Camera || !ProjectFactory) return;

	// 카메라에서 조준점 계산
	FVector CameraLocation = Owner->Camera->GetComponentLocation();
	FVector CameraForward = Owner->Camera->GetForwardVector();
	FVector TraceEnd = CameraLocation + (CameraForward * 10000.0f); // 멀리 트레이스

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner); // 자기 자신 무시

	// 조준점 찾기
	FVector TargetPoint;
	if (GetWorld()->LineTraceSingleByChannel(HitResult, CameraLocation, TraceEnd, ECC_WorldStatic, QueryParams))
	{
		TargetPoint = HitResult.ImpactPoint; // 벽이나 오브젝트에 맞은 지점
	}
	else
	{
		TargetPoint = TraceEnd; // 아무것도 안 맞으면 멀리 있는 지점
	}

	// 캐릭터 메시에서 발사 위치 설정 (가슴 높이)
	FVector SpawnLocation = Owner->GetMesh()->GetComponentLocation();
	SpawnLocation.Z += 120.0f; // 가슴 높이로 올림
	SpawnLocation += Owner->GetActorForwardVector() * 50.0f; // 캐릭터 앞쪽으로

	// 발사 방향 = 메시 위치에서 조준점을 향하는 벡터
	FVector ShootDirection = (TargetPoint - SpawnLocation).GetSafeNormal();
	FRotator SpawnRotation = ShootDirection.Rotation();

    // 발사체 스폰 (Owner/Instigator를 스폰 시 지정하여 즉시 자기자신 무시)
    FTransform Transform = FTransform(SpawnRotation, SpawnLocation);
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Owner;
    SpawnParams.Instigator = Cast<APawn>(Owner);
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AProjectile* bullet = GetWorld()->SpawnActor<AProjectile>(ProjectFactory, Transform, SpawnParams);
    if (bullet)
    {
        bullet->SetProjectileOwner(Owner);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Projectile 스폰 실패"));
    }
}
	

	


void UShootAction::End_Implementation(ACharacterBase* Owner)
{
	Super::End_Implementation(Owner);
}

void UShootAction::Cancel_Implementation(ACharacterBase* Owner)
{
	Super::Cancel_Implementation(Owner);
}
