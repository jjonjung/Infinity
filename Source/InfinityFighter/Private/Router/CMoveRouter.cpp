// Fill out your copyright notice in the Description page of Project Settings.


#include "Router/CMoveRouter.h"

#include "CharacterActionStatComponent.h"
#include "Base/CharacterBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"


// Sets default values for this component's properties
UCMoveRouter::UCMoveRouter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UCMoveRouter::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OwnerChar = Cast<ACharacter>(GetOwner());
	OwnerCharacterBase = Cast<ACharacterBase>(GetOwner());
	
}

// Called every frame
void UCMoveRouter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UCMoveRouter::HandleMoveInput(float Forward, float Right)
{
	// 최적화 관점: 미세 노이즈 제거로 불필요한 연산/네트워크 트래픽 방지
	if (!OwnerChar.IsValid()) return;
	ACharacter* C = OwnerChar.Get();
	
	if (C)
	{
		bool bIsInWall = OwnerCharacterBase->ActionStatComp->GetInWall(); // 이런 함수가 있다고 가정
        
		if (bIsInWall)
		{
			// 벽붙기 상태에서의 이동 처리
			HandleWallMovement(C, Forward, Right);
		}
		else
		{
			if (OwnerCharacterBase->GetIsRunning() && Forward<=0.1f)
			{
				
				C->GetCharacterMovement()->MaxWalkSpeed = 600.f;  
				OwnerCharacterBase->SetIsRunning(false);
			}
			if (!FMath::IsNearlyZero(Forward)) //조건 체크때문에 넣었다기보단, 불필요한 호출/노이즈 방지
			{
			
				C->AddMovementInput(C->GetActorForwardVector(), Forward);
			}
			if (!FMath::IsNearlyZero(Right))
			{
				if (OwnerCharacterBase->GetIsRunning())
				{
					Right*=0.2f;
				}
				C->AddMovementInput(C->GetActorRightVector(), Right);
			}
		}
		

	}
}

FVector UCMoveRouter::GetWallTangentVector(ACharacter*  C)
{
	return C->GetActorRightVector();
}

void UCMoveRouter::HandleWallMovement(ACharacter* C, float Forward, float Right)
{
	
	if (!FMath::IsNearlyZero(Right))
	{
		
		FVector WallTangent = GetWallTangentVector(C); 
		C->AddMovementInput(WallTangent, Right);
	}
    
	if (!FMath::IsNearlyZero(Forward))
	{
		
		C->AddMovementInput(FVector::UpVector, Forward);
	}
}

void UCMoveRouter::HandleLookInput(float Yaw, float Pitch)
{
	// 최적화 관점: 미세 노이즈 제거로 불필요한 연산 방지
	if (!OwnerChar.IsValid()) return;
	ACharacter* C = OwnerChar.Get();
	if (C)
	{
		
		//감도조절 float MouseSensitivity = 1.f;
		if (!FMath::IsNearlyZero(Yaw)) // 좌우 회전 
		{
			C->AddControllerYawInput(Yaw); //* MouseSensitivity
		}
		if (!FMath::IsNearlyZero(Pitch)) // 상하 회전
		{
			C->AddControllerPitchInput(-Pitch);
		}
	}
}
