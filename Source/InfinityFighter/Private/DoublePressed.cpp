// Fill out your copyright notice in the Description page of Project Settings.

#include "DoublePressed.h"

#include <gsl/pointers>

#include "AttachWall.h"
#include "CharacterActionStatComponent.h"
#include "SpawnWall.h"
#include "Camera/CameraComponent.h"
#include "Chaos/Utilities.h"
#include "GameFramework/CharacterMovementComponent.h"

bool UDoublePressed::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{
    FVector Start = Owner->GetActorLocation();
    FVector end = Start + Owner->GetActorForwardVector()*80.f;
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    FCollisionObjectQueryParams ObjParams;
    ObjParams.AddObjectTypesToQuery(ECC_GameTraceChannel1);
    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, end, ECC_GameTraceChannel1, Params);

    
    FColor LineColor = bHit ? FColor::Red : FColor::Green;
   // DrawDebugLine(GetWorld(), Start, end, LineColor, false, 2.0f, 0, 2.0f);

    if (bHit)
    {
       DrawDebugPoint(GetWorld(), Hit.ImpactPoint, 10.0f, FColor::Yellow, false, 2.0f);
       UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *Hit.GetActor()->GetFName().ToString());
       auto Hitwall = Cast<AAttachWall>(Hit.GetActor());
       if (Hitwall)
       {
          CurrentWall = Hitwall;
          FVector wallRightDir = CurrentWall->GetActorRightVector();
          FVector ownerRightDir = Owner->GetActorRightVector();
          float dot = FVector::DotProduct(wallRightDir, ownerRightDir);
          UE_LOG(LogTemp, Warning, TEXT("dot: %f"), dot);
          float dot_abs = FMath::Abs(dot);
       	  CurrentWallNormal=Hit.Normal;
       	  HitPoint = Hit.ImpactPoint;
          // 벽과 캐릭터가 충분히 평행하지 않으면 붙을 수 없음
         
          
          return true;
       }
       else
       {
       	Owner -> SetIsRunning(true);
       	Owner -> GetCharacterMovement()->MaxWalkSpeed=1000.f;
       	FailReason = "Hit object is not a wall";
          FailReason = "Hit object is not a wall";
          return false;
       }
    }
	GEngine->AddOnScreenDebugMessage(-1,5,FColor::Red,"RunMode");
	Owner -> SetIsRunning(true);
	Owner -> GetCharacterMovement()->MaxWalkSpeed=1000.f;
	FailReason = "Hit object is not a wall";
    FailReason = "No wall detected";
    return false;
}

void UDoublePressed::Activate_Implementation(ACharacterBase* Owner)
{
	GEngine->AddOnScreenDebugMessage(-1,5,FColor::Red,"hitwall");
    
	// 벽에서 살짝 떨어진 거리로 캐릭터 위치 조정 + 지면에서 띄우기
	if (CurrentWall)
	{
		FVector WallForward = CurrentWall->GetActorForwardVector();
		FVector OwnerLocation = Owner->GetActorLocation();
		FVector WallLocation = CurrentWall->GetActorLocation();
		FVector OwnerToWall = (WallLocation - OwnerLocation).GetSafeNormal();
        
		float DotWithWallForward = FVector::DotProduct(OwnerToWall, CurrentWall->GetActorRightVector());
        
		float DesiredDistance = 50.0f;
		float LiftHeight = 30.0f; 
        
		FVector NewLocation;
        
		if (DotWithWallForward > 0)
		{
			NewLocation = HitPoint - CurrentWall->GetActorRightVector() * DesiredDistance;
		}
		else
		{
			NewLocation = HitPoint + CurrentWall->GetActorRightVector() * DesiredDistance;
		}
        
		// Z축(높이)을 현재보다 LiftHeight만큼 위로
		NewLocation.Z = OwnerLocation.Z + LiftHeight;
       
		// 위치 설정
		Owner->SetActorLocation(NewLocation);
       
		// 벽 방향을 미리 계산해서 고정
		FixedWallTangent = CurrentWall->GetActorRightVector();
		UE_LOG(LogTemp, Log, TEXT("Fixed wall tangent: %s"), *FixedWallTangent.ToString());
       
		UE_LOG(LogTemp, Log, TEXT("Character lifted %f units from ground and positioned %f units from wall"), LiftHeight, DesiredDistance);
	}
    
	// 벽에 평행하게 캐릭터 회전 (위치 설정 후에)
	MakeParallel(Owner);
    
	// 회전 고정 설정 - 벽붙기 중에는 더 이상 회전하지 않도록
	Owner->GetCharacterMovement()->bUseControllerDesiredRotation = false;
	Owner->GetCharacterMovement()->bOrientRotationToMovement = false;
    
	// 물리 설정
	//Owner->GetCharacterMovement()->GravityScale = 0.0f;
    
	// 벽붙기 상태 설정
	Owner->ActionStatComp->SetInWall(true); 
	Owner->SetWallClimbMode(true);
}

void UDoublePressed::ClearWall()
{
    CurrentWall = nullptr;
}

FRotator UDoublePressed::MakeParallel(ACharacterBase* Owner)
{
	FVector TargetForward = -CurrentWallNormal;
    
	FRotator TargetRotation = TargetForward.Rotation();
	Owner->SetActorRotation(TargetRotation);
	UE_LOG(LogTemp, Log, TEXT("Character rotated using wall normal: %s"), *CurrentWallNormal.ToString());
    
	return TargetRotation;
}