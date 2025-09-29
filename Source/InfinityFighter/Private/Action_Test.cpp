// Fill out your copyright notice in the Description page of Project Settings.


#include "Action_Test.h"

#include "CharacterActionStatComponent.h"

UAction_Test::UAction_Test()
{
}

//실행이 가능한 상태인가?
bool UAction_Test::CanActivate_Implementation(ACharacterBase* Owner, FString& FailReason) const
{	
	if (!Owner) // 이컴포넌트가 등록된 owner 가 존재하지 않는 경우 
	{
		UE_LOG(LogTemp,Warning,TEXT("owner가 없다는디?"));
		return false;
	}
	auto* StatComp = Owner->FindComponentByClass<UCharacterActionStatComponent>();
	if (!StatComp) // 이 액션에 사용하기 위한 스텟 컴포넌트가 없는경우
	{
		UE_LOG(LogTemp,Warning,TEXT("StatComp가 없다는디?"));
		return false;
	}
	return true;
	
}

bool UAction_Test::PayCost_Implementation(ACharacterBase* Owner, FString& FailReason) 
{
	int32 Currnet = Owner->ActionStatComp->GetCurrentBullet();
	if (Currnet>=1)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("쓰기전 남은 탄창검사 %d"),Currnet)); // 화면출력
		return true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("지불하려는데 총알이 없네 %d"),Currnet));
		return false;
	}
	
	
}

// 실제 구현부
void UAction_Test::Activate_Implementation(ACharacterBase* Owner)
{
	GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Emerald,FString::Printf(TEXT("실행한다!")));
	int32 current = Owner->ActionStatComp->GetCurrentBullet();
	Owner->ActionStatComp->SetCurrentBullet(current-1);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("실행후 남은 탄창 %d"), current-1));
	
}
//실제 실행이 정상적으로 종료되고 나서 실행되는 함수 
void UAction_Test::End_Implementation(ACharacterBase* Owner)
{
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("실행이 아무 방해 없이 끝났을때 호출")));
	
}

//중간에 방해를 받아 실행되는 경우 
void UAction_Test::Cancel_Implementation(ACharacterBase* Owner)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("어떤 놈이 방해햇대")));
	
}
