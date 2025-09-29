// Fill out your copyright notice in the Description page of Project Settings.

#include "Chatacter/MySpiderMan.h"
#include "ActionComponent.h"
#include "Component/InputProxyComponent.h"
#include "Router/ActionRouter.h"
#include "Engine/Engine.h"
#include "Skill/Skill1_SpiderManSwing.h"

AMySpiderMan::AMySpiderMan()
{
	// 기본 설정
	PrimaryActorTick.bCanEverTick = true;
}

void AMySpiderMan::BeginPlay()
{
	Super::BeginPlay();

	
	FTimerHandle SetupTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(SetupTimerHandle, [this]()
	{
		SetupSpiderManActions();
	}, 0.5f, false); // 0.5초 지연으로 컴포넌트 초기화 대기
}

void AMySpiderMan::SetupSpiderManActions()
{
	// ActionComponent 유효성 체크
	if (!ActionComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MySpiderMan: ActionComponent가 없습니다."));
		return;
	}

	// Skill1_SpiderManSwing 액션 생성 및 등록 (Action.Skill1 태그 사용)
	USkill1_SpiderManSwing* SpiderManSwingSkill = Cast<USkill1_SpiderManSwing>(
		ActionComponent->CreateAndRegister(USkill1_SpiderManSwing::StaticClass(), FName("Action.Skill1"))
	);

	if (SpiderManSwingSkill)
	{
		// 기본 설정값들은 블루프린트에서 설정하거나 여기서 추가 설정 가능
		UE_LOG(LogTemp, Log, TEXT("MySpiderMan: Skill1_SpiderManSwing 액션이 등록되었습니다."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("MySpiderMan: Skill1_SpiderManSwing 액션 생성에 실패했습니다."));
	}

	// ActionRouter에 Input.Skill1 → Action.Skill1 매핑 추가
	if (UActionRouter* ActionRouterComp = FindComponentByClass<UActionRouter>())
	{
		ActionRouterComp->RegisterActionRouter(FName("Input.Skill1"), FName("Action.Skill1"));
		UE_LOG(LogTemp, Log, TEXT("MySpiderMan: Input.Skill1 → Action.Skill1 매핑 추가"));
	}

}

void AMySpiderMan::OnInputIntent(FName IntentTag)
{
	// ActionComponent를 통해 액션 실행
	if (ActionComponent)
	{
		FString FailReason;
		bool bSuccess = ActionComponent->TryRunAction(IntentTag, FailReason);

		if (bSuccess)
		{
			UE_LOG(LogTemp, Log, TEXT("MySpiderMan: Intent '%s' 실행 성공"), *IntentTag.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("MySpiderMan: Intent '%s' 실행 실패: %s"), *IntentTag.ToString(), *FailReason);
		}
	}
}
