// Fill out your copyright notice in the Description page of Project Settings.


#include "Router/ActionRouter.h"

// Sets default values for this component's properties
UActionRouter::UActionRouter()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// Default mapping for Fly (space double-tap) if not provided by DataAsset
	if (!RouterMap.Contains(FName("Input.Fly")))
	{
		RouterMap.Add(FName("Input.Fly"), FName("Action.Fly"));
		UE_LOG(LogTemp, Warning, TEXT("기본 매핑 추가: Input.Fly → Action.Fly"));
	}

	// ...
}


// Called when the game starts
void UActionRouter::BeginPlay()
{
	Super::BeginPlay();
	ActionComponent = GetOwner()->FindComponentByClass<UActionComponent>();

	// AI용 기본 매핑 추가 (DataAsset에 설정이 없을 경우 대비)
	if (!RouterMap.Contains(FName("Input.Fire")))
	{
		UE_LOG(LogTemp,Warning,TEXT("값 없어서 begin 에서 넣음."));
		RouterMap.Add(FName("Input.Fire"), FName("Action.Fire"));
		UE_LOG(LogTemp, Warning, TEXT("기본 매핑 추가: Input.Fire → Action.Fire"));
	}
	
	// ...
}


// Called every frame
void UActionRouter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UActionRouter::IsValidIntent(FName IntentName)
{
	if (RouterMap.Contains(IntentName))
	{
		UE_LOG(LogTemp,Warning,TEXT("인텐트에 맞는 액션이 있습니다."));
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("IntentName"), IntentName);

	UE_LOG(LogTemp,Warning,TEXT("인텐트에 맞는 액션이 없습니다."));
	return false;
}

bool UActionRouter::IsValidAction(const FName ActionTag)
{
	for (const auto& Action : RouterMap)
	{
		if (Action.Value == ActionTag)
		{
			UE_LOG(LogTemp,Warning,TEXT("액션 존재"));
			return true;
		}
	}
	return false;
}

void UActionRouter::RegisterActionRouter(FName IntentName, FName ActionRouterName)
{
	if (IsValidIntent(IntentName))
	{
		UE_LOG(LogTemp,Warning,TEXT("이미 존재하는 인텐트."));
	}
	else
	{
		RouterMap.Add(IntentName,ActionRouterName);
	}
}

void UActionRouter::RemoveActionRouterByIntentName(FName IntentName)
{
	if (IsValidIntent(IntentName))
	{
		RouterMap.Remove(IntentName);
		return;
	}
	UE_LOG(LogTemp,Warning,TEXT("존재하지 않는 인텐트."));
	return;

}

void UActionRouter::RemoveActionRouterByActionTag(FName ActionTag)
{
	// RouteMap은 IntentTag → ActionTag 매핑
	for (auto It = RouterMap.CreateIterator(); It; ++It)
	{
		if (It.Value() == ActionTag)
		{
			It.RemoveCurrent();
			UE_LOG(LogTemp, Warning, TEXT("ActionRouter: '%s' 라우팅 제거됨"), *ActionTag.ToString());
			return; // 하나만 제거 후 종료 (중복 매핑 없다면)
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ActionRouter: '%s' 관련 라우팅을 찾을 수 없음"), *ActionTag.ToString());
}

void UActionRouter::InitActionRouterMap(TArray<FName> IntentNames, TArray<FName> ActionTags)
{
	for (int i = 0; i < IntentNames.Num(); i++)
	{
		RegisterActionRouter(IntentNames[i], ActionTags[i]);
	}
}

void UActionRouter::ExcuteActionByIntent(FName Intent)
{
	UE_LOG(LogTemp, Warning, TEXT("ExcuteActionByIntent 성공"));
	if (!RouterMap.Contains(Intent))
	{
		UE_LOG(LogTemp, Warning, TEXT("RouterMap 값(%s): %s"),
		*Intent.ToString(),
		RouterMap.Contains(Intent) ? TEXT("true") : TEXT("false"));
		UE_LOG(LogTemp, Warning, TEXT("=== RouterMap 전체 출력 시작 ==="));
		for (const TPair<FName, FName>& Elem : RouterMap)
		{
			UE_LOG(LogTemp, Warning, TEXT("Key: %s | Value: %s"),
				*Elem.Key.ToString(),
				*Elem.Value.ToString());
		}
		UE_LOG(LogTemp, Warning, TEXT("=== RouterMap 전체 출력 끝 ==="));
		
		UE_LOG(LogTemp, Warning, TEXT("Intent %s → Action 없음"), *Intent.ToString());
		return;
	}
	if (!ActionComponent)
	{
		UE_LOG(LogTemp,Warning,TEXT("액션 컴포넌트가 없음"));
	}
	FName ActionTag = RouterMap[Intent];
	FString FailReason;

	UE_LOG(LogTemp,Warning,TEXT("액션 컴포넌트가 있고 RoutwrMap"));

	UE_LOG(LogTemp, Warning, TEXT("ActionTag: %s"), *ActionTag.ToString());
	UE_LOG(LogTemp, Warning, TEXT("ActionTag: %s | FailReason: %s"),
	*ActionTag.ToString(),
	*FailReason);
	
	if (ActionComponent->TryRunAction(ActionTag, FailReason))
	{
		UE_LOG(LogTemp, Warning, TEXT("실행 성공: %s"), *ActionTag.ToString());
	}
	else
	{
		
		UE_LOG(LogTemp, Warning,TEXT("실행 실패: %s (%s)"),
			   *ActionTag.ToString(), *FailReason);
	}

}

void UActionRouter::InitFromRouterDA(UActionRouterDA* AcRouterDa)
{
	for (const FRoutePair& Route : AcRouterDa->Routes)
	{
		RegisterActionRouter(Route.IntentTag, Route.ActionTag);
	}
}

