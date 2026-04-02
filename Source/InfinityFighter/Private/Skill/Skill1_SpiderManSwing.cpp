// Fill out your copyright notice in the Description page of Project Settings.

#include "Skill/Skill1_SpiderManSwing.h"

#include "ActionComponent.h"
#include "Base/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/OverlapResult.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"

USkill1_SpiderManSwing::USkill1_SpiderManSwing()
{
	// 기본 설정
	ActionTag = FName("Input.Skill1");
	CooldownSec = 2.0f;
	bAutoFinish = false; // 수동으로 종료 관리

	// 초기 상태
	bIsSwinging = false;
	SwingVelocity = FVector::ZeroVector;
	WebLineComponent = nullptr;
	WebLineMesh = nullptr;

	// 공격 관련 기본값 설정
	SwingDamage = 10.0f;
	AttackRadius = 100.0f;
	KnockbackForce = 1000.0f;
	MinAttackVelocity = 500.0f;

	//Anim
	static ConstructorHelpers::FObjectFinder<UAnimMontage> HandFireMontageRef(
		TEXT("'/Game/BluePrint/RealCharacter/SpiderMan/SpiderMan_AttackspicerMan_Atrtack2_Unreal5_6_Anim_Montage.SpiderMan_AttackspicerMan_Atrtack2_Unreal5_6_Anim_Montage'")
	);
	if (HandFireMontageRef.Succeeded())
	{
		ActionMontage = HandFireMontageRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UAnimMontage> HandFireMontageRef2(
		TEXT("'/Game/BluePrint/RealCharacter/SpiderMan/Anim/Fly3fly3_Unreal5_6_Anim_Montage.Fly3fly3_Unreal5_6_Anim_Montage'")
	);
	if (HandFireMontageRef2.Succeeded())
	{
		ActionMontage2 = HandFireMontageRef2.Object;
	}

	// WebLine Skeletal Mesh 로드
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> WebLineMeshRef(
		TEXT("'/Game/CEJ/Asset/SKM_BoundingBoxEdge.SKM_BoundingBoxEdge'")
	);
	if (WebLineMeshRef.Succeeded())
	{
		WebLineMesh = WebLineMeshRef.Object;
		UE_LOG(LogTemp, Warning, TEXT("WebLine Skeletal Mesh 로드 성공"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("WebLine Skeletal Mesh 로드 실패"));
	}
}

bool USkill1_SpiderManSwing::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
	if (!Owner)
	{
		FailReason = "Owner is null";
		return false;
	}

	if (bIsSwinging)
	{
		FailReason = "Already swinging";
		return false;
	}


	// 스윙 타겟을 찾을 수 있는지 확인
	FVector TempAnchor;
	if (!const_cast<USkill1_SpiderManSwing*>(this)->FindSwingTarget(Owner, TempAnchor))
	{
		FailReason = "No valid swing target found";
		return false;
	}
	return !bOnCooldown;

	/*
	return Super::CanActivate_Implementation(Owner, FailReason);*/
}

void USkill1_SpiderManSwing::Activate_Implementation(ACharacterBase* Owner)
{
	Super::Activate_Implementation(Owner);

	UE_LOG(LogTemp, Warning, TEXT("스파이더맨 스윙 활성화!"));

	
	// 타겟 찾기 (적 우선, 벽 차선)
	if (FindSwingTarget(Owner, SwingAnchorPoint))
	{
		if (TargetEnemy.IsValid())
		{
			// 적을 찾았다면 끌어오기 모드
			Owner->PlayActionMontage(ActionMontage);
			StartEnemyPull(Owner);
			UE_LOG(LogTemp, Warning, TEXT("적 끌어오기 시작!"));
		}
		else
		{
			// 벽을 찾았다면 벽 붙기 모드
			Owner->PlayActionMontage(ActionMontage2);
			StartWallCrawling(Owner);
			UE_LOG(LogTemp, Warning, TEXT("벽 붙기 시작!"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("타겟을 찾을 수 없습니다"));
		End(Owner);
	}
}

void USkill1_SpiderManSwing::End_Implementation(ACharacterBase* Owner)
{
	UE_LOG(LogTemp, Warning, TEXT("스파이더맨 스윙 종료"));

	EndSwinging(Owner);
	Super::End_Implementation(Owner);

	Owner->ActionComponent->FinishActionNow(GetActionTag(),this);
}

void USkill1_SpiderManSwing::Cancel_Implementation(ACharacterBase* Owner)
{
	UE_LOG(LogTemp, Warning, TEXT("스파이더맨 스윙 취소"));

	EndSwinging(Owner);
	Super::Cancel_Implementation(Owner);
}

bool USkill1_SpiderManSwing::FindSwingTarget(ACharacterBase* Owner, FVector& OutAnchorPoint)
{
	if (!Owner) return false;

	UWorld* World = Owner->GetWorld();
	if (!World) return false;

	FVector StartLocation = Owner->GetActorLocation();

	FVector ForwardDirection = Owner->GetActorForwardVector();

	// 적을 찾아보기 (적 끌어오기 우선)
	if (FindEnemyTarget(Owner, OutAnchorPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("적 타겟 발견: %s"), *OutAnchorPoint.ToString());
		return true;
	}

	// 적이 없으면 벽 찾기 (벽 붙기)
	FVector UpDirection = FVector::UpVector;

	// 여러 방향으로 레이캐스트 시도 (벽 붙기용)
	TArray<FVector> SearchDirections;
	SearchDirections.Reserve(6);

	// 앞쪽, 위쪽, 대각선 방향들
	SearchDirections.Add(ForwardDirection); // 바로 앞
	SearchDirections.Add((ForwardDirection + UpDirection * 0.3f).GetSafeNormal());
	SearchDirections.Add((ForwardDirection + UpDirection * 0.7f).GetSafeNormal());
	SearchDirections.Add(UpDirection); // 바로 위

	// 좌우 방향들
	FVector RightDirection = Owner->GetActorRightVector();
	SearchDirections.Add((ForwardDirection * 0.7f + RightDirection * 0.5f).GetSafeNormal());
	SearchDirections.Add((ForwardDirection * 0.7f - RightDirection * 0.5f).GetSafeNormal());

	for (const FVector& Direction : SearchDirections)
	{
		FVector EndLocation = StartLocation + Direction * MaxSwingDistance;

		FHitResult HitResult;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);

		bool bHit = World->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_WorldStatic,
			QueryParams
		);

		if (bHit)
		{
			OutAnchorPoint = HitResult.Location;

			// 디버그 라인 그리기
			//DrawDebugLine(World, StartLocation, OutAnchorPoint, FColor::White, false, 5.0f, 0, 3.0f);
			//DrawDebugSphere(World, OutAnchorPoint, 20.0f, 12, FColor::Blue, false, 5.0f);

			UE_LOG(LogTemp, Warning, TEXT("벽 붙기 타겟 발견: %s"), *OutAnchorPoint.ToString());
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("유효한 타겟을 찾을 수 없습니다"));
	return false;
}

bool USkill1_SpiderManSwing::FindEnemyTarget(ACharacterBase* Owner, FVector& OutAnchorPoint)
{
	if (!Owner) return false;

	UWorld* World = Owner->GetWorld();
	if (!World) return false;

	FVector StartLocation = Owner->GetActorLocation();
	FVector ForwardDirection = Owner->GetActorForwardVector();

	// 앞 방향으로 적 탐지
	FVector EndLocation = StartLocation + ForwardDirection * MaxSwingDistance;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = World->LineTraceSingleByChannel(
		HitResult,
		StartLocation,
		EndLocation,
		ECC_Pawn,
		QueryParams
	);

	if (bHit)
	{
		// 적 캐릭터인지 확인
		if (ACharacterBase* Enemy = Cast<ACharacterBase>(HitResult.GetActor()))
		{
			if (Enemy != Owner) // 자신이 아닌 경우
			{
				OutAnchorPoint = Enemy->GetActorLocation();
				TargetEnemy = Enemy; // 타겟 적 저장

				// 디버그 표시
				//DrawDebugLine(World, StartLocation, OutAnchorPoint, FColor::White, false, 5.0f, 0, 3.0f);
				//DrawDebugSphere(World, OutAnchorPoint, 30.0f, 12, FColor::Red, false, 5.0f);

				return true;
			}
		}
	}

	return false;
}

void USkill1_SpiderManSwing::StartSwinging(ACharacterBase* Owner)
{
	if (!Owner) return;

	bIsSwinging = true;
	CurrentRopeLength = FVector::Dist(Owner->GetActorLocation(), SwingAnchorPoint);

	// 공격한 액터 목록 초기화
	HitActors.Reset();

	// 땅에 있다면 점프하여 공중으로 이동
	if (Owner->GetCharacterMovement()->IsMovingOnGround())
	{
		Owner->Jump();
		UE_LOG(LogTemp, Warning, TEXT("스윙 시작을 위해 점프 실행"));
	}

	// 초기 스윙 속도 설정
	SwingVelocity = Owner->GetVelocity();
	SwingAngularVelocity = 0.0f;

	// 중력 비활성화
	if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
	{
		Movement->GravityScale = 0.1f; // 완전히 끄지 않고 약간만
		Movement->AirControl = 1.0f; // 공중 제어 증가
	}

	// 웹 라인 생성
	CreateWebLine(Owner);

	// 웹 라인 메시들 생성
	CreateWebLineMeshes(Owner);

	// 업데이트 타이머 시작
	if (UWorld* World = Owner->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SwingUpdateHandle,
			FTimerDelegate::CreateUObject(this, &USkill1_SpiderManSwing::UpdateSwing, Owner),
			0.016f, // 60FPS
			true
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("스윙 시작! 로프 길이: %f"), CurrentRopeLength);
}

void USkill1_SpiderManSwing::UpdateSwing(ACharacterBase* Owner)
{
	if (!Owner || !bIsSwinging) return;

	float DeltaTime = 0.016f; // 60FPS 고정

	// 스윙 물리 계산
	CalculateSwingPhysics(Owner, DeltaTime);

	// 스윙 공격 체크
	CheckSwingAttack(Owner);

	// 웹 라인 시각적 업데이트
	UpdateWebLineVisual(Owner);

	// 웹 라인 메시 위치 업데이트
	UpdateWebLineMeshes(Owner);

	// 스윙 종료 조건 체크
	FVector PlayerLocation = Owner->GetActorLocation();
	FVector ToAnchor = SwingAnchorPoint - PlayerLocation;

	// 플레이어가 앵커점보다 높거나 너무 멀어지면 종료
	if (PlayerLocation.Z > SwingAnchorPoint.Z || ToAnchor.Size() > MaxSwingDistance * 1.5f)
	{
		UE_LOG(LogTemp, Warning, TEXT("스윙 자동 종료 - 높이나 거리 초과"));
		End(Owner);
		return;
	}

	// 땅에 닿으면 종료
	if (Owner->GetCharacterMovement()->IsMovingOnGround())
	{
		UE_LOG(LogTemp, Warning, TEXT("스윙 종료 - 땅에 닿음"));
		End(Owner);
		return;
	}
}

void USkill1_SpiderManSwing::CalculateSwingPhysics(ACharacterBase* Owner, float DeltaTime)
{
	FVector PlayerLocation = Owner->GetActorLocation();
	FVector ToAnchor = SwingAnchorPoint - PlayerLocation;
	float Distance = ToAnchor.Size();

	// 로프 장력 계산
	FVector RopeDirection = ToAnchor.GetSafeNormal();

	// 현재 속도
	FVector CurrentVelocity = Owner->GetVelocity();

	// 중력 적용
	FVector Gravity = FVector(0, 0, -980.0f) * DeltaTime;
	SwingVelocity = CurrentVelocity + Gravity;

	// 원형 운동을 위한 구속력 적용
	if (Distance > CurrentRopeLength)
	{
		// 로프 길이를 유지하기 위한 구속
		FVector RadialVelocity = FVector::DotProduct(SwingVelocity, RopeDirection) * RopeDirection;
		FVector TangentialVelocity = SwingVelocity - RadialVelocity;

		// 로프 방향 속도 제거 (팽팽함 유지)
		SwingVelocity = TangentialVelocity;

		// 위치 보정
		FVector CorrectPosition = SwingAnchorPoint - RopeDirection * CurrentRopeLength;
		Owner->SetActorLocation(CorrectPosition);
	}

	// 감쇠 적용
	SwingVelocity *= SwingDamping;

	// 새로운 속도 적용
	if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
	{
		Movement->Velocity = SwingVelocity;
	}
}

void USkill1_SpiderManSwing::EndSwinging(ACharacterBase* Owner)
{
	if (!Owner) return;

	bIsSwinging = false;
	bIsWallCrawling = false;
	TargetEnemy.Reset();

	// 중력 및 이동 모드 복원
	if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
	{
		Movement->GravityScale = 1.0f;
		Movement->AirControl = 0.05f; // 기본값으로 복원
		Movement->SetMovementMode(MOVE_Walking);

		// 스윙/벽붙기/끌어오기 종료 시 적절한 속도 부여
		if (bIsWallCrawling)
		{
			// 벽에서 떨어질 때는 약간의 뒤쪽 속도
			FVector BackwardVelocity = -Owner->GetActorForwardVector() * 300.0f;
			BackwardVelocity.Z = 200.0f;
			Movement->Velocity = BackwardVelocity;
		}
		else
		{
			// 일반 스윙 종료 시 추진력 부여
			FVector FinalVelocity = SwingVelocity * ReleaseVelocityMultiplier;
			FinalVelocity.Z = FMath::Max(FinalVelocity.Z, 300.0f); // 최소 상승력 보장
			Movement->Velocity = FinalVelocity;
		}
	}

	// 웹 라인 제거
	DestroyWebLine();

	// 웹 라인 메시들 제거
	// Reuse previously created mesh components instead of recreating them every activation.

	// 타이머 정리
	if (UWorld* World = Owner->GetWorld())
	{
		World->GetTimerManager().ClearTimer(SwingUpdateHandle);
	}

	UE_LOG(LogTemp, Warning, TEXT("스파이더맨 스킬 완전 종료"));
	Owner->ActionComponent->FinishActionNow(GetActionTag(),this);
}

void USkill1_SpiderManSwing::CreateWebLine(ACharacterBase* Owner)
{
	if (!Owner) return;

	// SKM_BoundingBoxEdge 메시로만 웹라인 구현
	UE_LOG(LogTemp, Warning, TEXT("웹 라인 생성 - SKM_BoundingBoxEdge 메시 사용"));
}

void USkill1_SpiderManSwing::UpdateWebLineVisual(ACharacterBase* Owner)
{
	if (!Owner) return;

	// 디버그용 앵커 포인트 표시와 라인 그리기
	if (UWorld* World = Owner->GetWorld())
	{
		//DrawDebugLine(World, Owner->GetActorLocation(), SwingAnchorPoint, FColor::White, false, 0.1f, 0, 3.0f);
		//DrawDebugSphere(World, SwingAnchorPoint, 10.0f, 8, FColor::Red, false, 0.1f);
	}
}

void USkill1_SpiderManSwing::DestroyWebLine()
{
	if (WebLineComponent)
	{
		WebLineComponent->DestroyComponent();
		WebLineComponent = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("웹 라인 제거 완료"));
}

FVector USkill1_SpiderManSwing::GetSwingDirection(const FVector& PlayerPos, const FVector& AnchorPos) const
{
	FVector ToAnchor = AnchorPos - PlayerPos;
	return FVector::CrossProduct(ToAnchor, FVector::UpVector).GetSafeNormal();
}

// ========== 공격 시스템 ==========

void USkill1_SpiderManSwing::CheckSwingAttack(ACharacterBase* Owner)
{
	if (!Owner) return;

	// 현재 속도가 공격 최소 속도 이상인지 확인
	float CurrentSpeed = Owner->GetVelocity().Size();
	if (CurrentSpeed < MinAttackVelocity)
	{
		return;
	}

	// 주변 적들 탐지
	UWorld* World = Owner->GetWorld();
	if (!World) return;

	FVector OwnerLocation = Owner->GetActorLocation();

	// 구체 충돌 체크
	CachedOverlapResults.Reset();
	CachedOverlapResults.Reserve(16);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = World->OverlapMultiByChannel(
		CachedOverlapResults,
		OwnerLocation,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(AttackRadius),
		QueryParams
	);

	if (bHit)
	{
		for (const FOverlapResult& Result : CachedOverlapResults)
		{
			if (ACharacterBase* Target = Cast<ACharacterBase>(Result.GetActor()))
			{
				// 이미 공격한 타겟이 아니고, 공격 가능한 타겟인지 확인
				const TWeakObjectPtr<AActor> TargetKey(Target);
				if (!HitActors.Contains(TargetKey) && CanAttackTarget(Owner, Target))
				{
					PerformSwingAttack(Owner, Target);
					HitActors.Add(TargetKey);

					UE_LOG(LogTemp, Warning, TEXT("스윙 공격 적중: %s"), *Target->GetName());
				}
			}
		}
	}

	// 디버그 구체 그리기
	DrawDebugSphere(World, OwnerLocation, AttackRadius, 12, FColor::Red, false, 0.1f);
}

void USkill1_SpiderManSwing::PerformSwingAttack(ACharacterBase* Owner, ACharacterBase* Target)
{
	if (!Owner || !Target) return;

	// 데미지 방향 계산
	FVector AttackDirection = (Target->GetActorLocation() - Owner->GetActorLocation()).GetSafeNormal();

	// 데미지 적용
	if (Target->ActionStatComp)
	{
		// 기존 데미지 시스템 사용 (CharacterActionStatComponent 참고)
		//Target->ActionStatComp->TakeDamage(SwingDamage, Owner, AttackDirection);
		UE_LOG(LogTemp, Warning, TEXT("스윙 데미지 적용: %f to %s"), SwingDamage, *Target->GetName());
	}

	// 넉백 효과 적용
	if (UCharacterMovementComponent* TargetMovement = Target->GetCharacterMovement())
	{
		FVector KnockbackVector = AttackDirection * KnockbackForce;
		KnockbackVector.Z += 300.0f; // 약간의 상승력 추가

		// 기존 속도에 넉백 벡터 추가
		TargetMovement->Velocity += KnockbackVector;

		UE_LOG(LogTemp, Warning, TEXT("넉백 적용: %s"), *KnockbackVector.ToString());
	}

	// 타격 효과 (파티클, 사운드 등을 나중에 추가 가능)
	if (UWorld* World = Owner->GetWorld())
	{
		// 타격 지점에 이펙트 표시
		FVector HitLocation = Target->GetActorLocation();
		DrawDebugSphere(World, HitLocation, 50.0f, 12, FColor::Yellow, false, 2.0f);

		// 충격파 효과
		for (int32 i = 0; i < 8; i++)
		{
			FVector EffectDirection = FVector(
				FMath::Cos(i * 45.0f * PI / 180.0f),
				FMath::Sin(i * 45.0f * PI / 180.0f),
				0.0f
			);
			DrawDebugLine(
				World,
				HitLocation,
				HitLocation + EffectDirection * 100.0f,
				FColor::Orange,
				false,
				1.0f,
				0,
				3.0f
			);
		}
	}
}

bool USkill1_SpiderManSwing::CanAttackTarget(ACharacterBase* Owner, ACharacterBase* Target) const
{
	if (!Owner || !Target) return false;

	// 자기 자신은 공격하지 않음
	if (Owner == Target) return false;

	// 같은 팀이면 공격하지 않음 (필요에 따라 팀 시스템 추가 가능)
	// 현재는 플레이어 vs AI 구조로 가정

	// 타겟이 살아있는지 확인
	/*if (Target->ActionStatComp && Target->GetCurrentHP() <= 0)
	{
		return false;
	}*/

	return true;
}

// ========== 벽 붙기 시스템 ==========
void USkill1_SpiderManSwing::StartWallCrawling(ACharacterBase* Owner)
{
	if (!Owner) return;

	bIsSwinging = false;
	bIsWallCrawling = true;

	// 중력 비활성화
	if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
	{
		Movement->GravityScale = 0.0f;
		Movement->SetMovementMode(MOVE_Flying);
	}

	// 웹 라인 생성
	CreateWebLine(Owner);

	// 웹 라인 메시들 생성
	CreateWebLineMeshes(Owner);

	// 벽으로 이동 시작
	if (UWorld* World = Owner->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SwingUpdateHandle,
			FTimerDelegate::CreateUObject(this, &USkill1_SpiderManSwing::UpdateWallCrawling, Owner),
			0.016f,
			true
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("벽 붙기 모드 시작"));
}

void USkill1_SpiderManSwing::UpdateWallCrawling(ACharacterBase* Owner)
{
	if (!Owner || !bIsWallCrawling) return;

	FVector OwnerLocation = Owner->GetActorLocation();
	FVector ToWall = SwingAnchorPoint - OwnerLocation;
	float DistanceToWall = ToWall.Size();

	// 벽에 도착했는지 확인
	if (DistanceToWall <= 50.0f)
	{
		// 벽에 붙기
		Owner->SetActorLocation(SwingAnchorPoint - ToWall.GetSafeNormal() * 30.0f);

		// 벽 방향으로 회전
		FRotator WallRotation = FRotationMatrix::MakeFromX(-ToWall.GetSafeNormal()).Rotator();
		Owner->SetActorRotation(WallRotation);

		// 벽 붙기 완료 - 3초 후 종료
		if (UWorld* World = Owner->GetWorld())
		{
			World->GetTimerManager().SetTimer(
				SwingUpdateHandle,
				FTimerDelegate::CreateLambda([this, Owner]() { End(Owner); }),
				1.0f,
				false
			);
		}

		UE_LOG(LogTemp, Warning, TEXT("벽에 붙기 완료! 3초 후 종료"));
		return;
	}

	// 벽으로 이동
	FVector MoveDirection = ToWall.GetSafeNormal();
	FVector NewVelocity = MoveDirection * SwingForce;

	if (UCharacterMovementComponent* Movement = Owner->GetCharacterMovement())
	{
		Movement->Velocity = NewVelocity;
	}

	// 웹 라인 업데이트
	UpdateWebLineVisual(Owner);

	// 웹 라인 메시 업데이트
	UpdateWebLineMeshes(Owner);
}

// ========== 적 끌어오기 시스템 ==========
void USkill1_SpiderManSwing::StartEnemyPull(ACharacterBase* Owner)
{
	if (!Owner || !TargetEnemy.IsValid()) return;

	bIsSwinging = false;
	bIsWallCrawling = false;

	// 웹 라인 생성
	CreateWebLine(Owner);

	// 웹 라인 메시들 생성
	CreateWebLineMeshes(Owner);

	// 적 끌어오기 시작
	if (UWorld* World = Owner->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SwingUpdateHandle,
			FTimerDelegate::CreateUObject(this, &USkill1_SpiderManSwing::UpdateEnemyPull, Owner),
			0.016f,
			true
		);
	}

	UE_LOG(LogTemp, Warning, TEXT("적 끌어오기 시작: %s"), *TargetEnemy->GetName());
}

void USkill1_SpiderManSwing::UpdateEnemyPull(ACharacterBase* Owner)
{
	if (!Owner || !TargetEnemy.IsValid()) return;

	ACharacterBase* Enemy = TargetEnemy.Get();
	FVector OwnerLocation = Owner->GetActorLocation();
	FVector EnemyLocation = Enemy->GetActorLocation();
	FVector ToOwner = OwnerLocation - EnemyLocation;
	float Distance = ToOwner.Size();

	// 적이 가까이 왔으면 종료
	if (Distance <= 150.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("적 끌어오기 완료!"));
		End(Owner);
		return;
	}

	// 적을 플레이어 쪽으로 끌어오기
	FVector PullDirection = ToOwner.GetSafeNormal();
	FVector PullVelocity = PullDirection * 1000;

	if (UCharacterMovementComponent* EnemyMovement = Enemy->GetCharacterMovement())
	{
		EnemyMovement->AddImpulse(PullVelocity, true);
	}

	// 웹 라인 업데이트 (적 위치로)
	SwingAnchorPoint = EnemyLocation;
	UpdateWebLineVisual(Owner);

	// 웹 라인 메시 업데이트
	UpdateWebLineMeshes(Owner);

	UE_LOG(LogTemp, Warning, TEXT("적 끌어오는 중... !!! 거리: %f"), Distance);
}

// ========== 웹라인 메시 시스템 ==========
void USkill1_SpiderManSwing::CreateWebLineMeshes(ACharacterBase* Owner)
{
	if (!Owner || !WebLineMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("Owner 또는 WebLineMesh가 null입니다"));
		return;
	}

	// 기존 메시들 정리
	DestroyWebLineMeshes();

	// 웹라인 길이 계산
	float WebLineLength = FVector::Dist(Owner->GetActorLocation(), SwingAnchorPoint);

	// 메시 간격 계산
	float MeshSpacing = WebLineLength / (MeshSegmentCount + 1);

	// 웹라인 방향 벡터
	FVector WebLineDirection = (SwingAnchorPoint - Owner->GetActorLocation()).GetSafeNormal();
	FVector StartLocation = Owner->GetActorLocation();

	// 메시 컴포넌트들 생성
	WebLineMeshComponents.Reserve(FMath::Max(WebLineMeshComponents.Num(), MeshSegmentCount));
	for (int32 i = 1; i <= MeshSegmentCount; ++i)
	{
		// 각 메시의 위치 계산
		FVector MeshLocation = StartLocation + (WebLineDirection * MeshSpacing * i);

		// SkeletalMeshComponent 생성
		USkeletalMeshComponent* MeshComp = NewObject<USkeletalMeshComponent>(Owner);
		if (MeshComp)
		{
			MeshComp->RegisterComponent();
			MeshComp->SetSkeletalMesh(WebLineMesh);

			// 월드에 배치
			MeshComp->SetWorldLocation(MeshLocation);

			// 웹라인 방향으로 회전
			FRotator MeshRotation = FRotationMatrix::MakeFromX(WebLineDirection).Rotator();
			MeshComp->SetWorldRotation(MeshRotation);

			// 스케일 설정 (선택사항)
			MeshComp->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));

			// 배열에 추가
			WebLineMeshComponents.Add(MeshComp);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("웹라인 메시 %d개 생성 완료 - 길이: %f"), MeshSegmentCount, WebLineLength);
}

void USkill1_SpiderManSwing::UpdateWebLineMeshes(ACharacterBase* Owner)
{
	if (!Owner || WebLineMeshComponents.Num() == 0)
		return;

	// 웹라인 길이 및 방향 재계산
	float WebLineLength = FVector::Dist(Owner->GetActorLocation(), SwingAnchorPoint);
	float MeshSpacing = WebLineLength / (MeshSegmentCount + 1);
	FVector WebLineDirection = (SwingAnchorPoint - Owner->GetActorLocation()).GetSafeNormal();
	FVector StartLocation = Owner->GetActorLocation();

	// 각 메시 위치 업데이트
	for (int32 i = 0; i < WebLineMeshComponents.Num(); ++i)
	{
		if (WebLineMeshComponents[i])
		{
			// 새로운 위치 계산
			FVector NewMeshLocation = StartLocation + (WebLineDirection * MeshSpacing * (i + 1));
			WebLineMeshComponents[i]->SetWorldLocation(NewMeshLocation);

			// 방향도 업데이트
			FRotator MeshRotation = FRotationMatrix::MakeFromX(WebLineDirection).Rotator();
			WebLineMeshComponents[i]->SetWorldRotation(MeshRotation);
		}
	}
}

void USkill1_SpiderManSwing::DestroyWebLineMeshes()
{
	// 모든 메시 컴포넌트 제거
	for (USkeletalMeshComponent* MeshComp : WebLineMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}

	// 배열 비우기
	WebLineMeshComponents.Empty();

	UE_LOG(LogTemp, Warning, TEXT("웹라인 메시들 제거 완료"));
}
