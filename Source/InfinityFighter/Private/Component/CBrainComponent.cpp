// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/CBrainComponent.h"
#include "CharacterActionStatComponent.h"
#include "PhysicsAssetUtils.h"
#include "Base/CharacterBase.h"
#include "Component/AiBrainComponent.h"
#include "Component/InputProxyComponent.h"
#include "Router/CMoveRouter.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"


// Sets default values for this component's properties
UCBrainComponent::UCBrainComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	
	PrimaryComponentTick.bCanEverTick = true; // AI 모드에서만 유효하게 사용
	
	// ...

}


// Called when the game starts
void UCBrainComponent::BeginPlay()
{
	Super::BeginPlay();

	// 자동으로 AI/Player 모드 결정
	AutoDetectBrainMode();

	// ...
	MoveRouter = GetOwner() ? GetOwner()->FindComponentByClass<UCMoveRouter>() : nullptr;
	ActionRouter = GetOwner() ? GetOwner() -> FindComponentByClass<UActionRouter>() : nullptr;
	InputProxy = GetOwner() ? GetOwner() -> FindComponentByClass<UInputProxyComponent>() : nullptr;

	if (InputProxy)
	{
		InputProxy->OnIntent.AddDynamic(this, &UCBrainComponent::RecievedIntent);
	}

	// AI 모드일 때 플레이어 InputProxy 찾아서 공격 감지 연결
	if (Mode == EBrainMode::AI) 
	{
		// 플레이어 Pawn 찾기
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			PlayerInputProxy = PlayerPawn->FindComponentByClass<UInputProxyComponent>();
			if (PlayerInputProxy)
			{
				// 플레이어 공격 입력 감지 이벤트 연결 (단일 함수로 처리)
				PlayerInputProxy->OnIntent.AddDynamic(this, &UCBrainComponent::OnPlayerAttackInput);
				UE_LOG(LogTemp, Warning, TEXT("[CBrainComponent] 플레이어 입력 감지 연결 완료"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("[CBrainComponent] 플레이어 InputProxy를 찾을 수 없습니다"));
			}
		}
	}

	if (Mode == EBrainMode::AI)
	{
		UE_LOG(LogTemp, Warning, TEXT("AI 모드로 초기화 시작"));

		AIBrain = NewObject<UAiBrainComponent>(this);
		AIBrain->InitializeAI();

		/*GetWorld()->GetTimerManager().SetTimer(
		   TargetRefreshHandle, this, &UCBrainComponent::RefreshTarget,
		   TargetRefreshInterval, true);*/
	}
}

void UCBrainComponent::RefreshTarget()
{
	//UE_LOG(LogTemp, Warning, TEXT("재탐색"));
	
	if (TargetActor.IsValid()) return;
	TargetActor = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

// Called every frame
void UCBrainComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                     FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bBrainActive)
	{
		return;
	}
		
	if (Mode == EBrainMode::AI && AIBrain)
	{
		AIBrain->ExecuteCurrentState(AIBrain->CurrentEnemyState);
	}

}

bool UCBrainComponent::BaseMove()
{

	// Player 모드가 아니거나 라우터 없음 → 그냥 종료
	if (Mode != EBrainMode::Player || !MoveRouter) return true;

	AActor* Self = GetOwner();
	if (!Self)
	{
		UE_LOG(LogTemp, Error, TEXT("[Brain] BaseMove: Owner is null"));
		return true;
	}

	// TargetActor 없거나 무효 → 재획득 시도
	if (!TargetActor.IsValid())
	{
		if (GetWorld()->GetFirstPlayerController() != nullptr)//UWorld* World = GetWorld()
		{
			UWorld* World = GetWorld();
			TargetActor = Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
		}
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[Brain] BaseMove: TargetActor invalid, skip this frame"));
		return true;
	}

	const FVector ToTarget = TargetActor->GetActorLocation() - Self->GetActorLocation();
	const FVector Dir = ToTarget.GetSafeNormal();

	const float ForwardAmt = FVector::DotProduct(Dir, Self->GetActorForwardVector()) * AiMoveScale;
	const float RightAmt   = FVector::DotProduct(Dir, Self->GetActorRightVector())   * AiMoveScale;

	MoveRouter->HandleMoveInput(ForwardAmt, RightAmt);
	return false;

}

void UCBrainComponent::OnMoveInput(const FVector2D& Axis2D)
{

	if (Mode != EBrainMode::Player || !MoveRouter) return;
	
	// 뒤로 이동할 때 (S키) 벽 모드 해제 처리
	if (Axis2D.Y < 0.f)
	{
		auto OwnerBase = Cast<ACharacterBase>(GetOwner());
		if (OwnerBase && OwnerBase->ActionStatComp)
		{
			if (OwnerBase->ActionStatComp->GetInWall())
			{
				OwnerBase->ExitWallMode();
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Brain] OnMoveInput: OwnerBase 또는 ActionStatComp가 null"));
		}
	}
	
	// 모든 방향 이동 처리 (앞, 뒤, 좌, 우)
	MoveRouter->HandleMoveInput(Axis2D.Y, Axis2D.X); // (Forward, Right)
	
	
}

void UCBrainComponent::RecievedIntent(FName IntentSignal)
{
	//UE_LOG(LogTemp, Warning, TEXT("[RecievedIntent] 호출됨: %s"), *IntentSignal.ToString());

	if (!ActionRouter)
	{
		UE_LOG(LogTemp, Error, TEXT("[RecievedIntent] ActionRouter가 null입니다!"));
		return;
	}

	if (ActionRouter->IsValidIntent(IntentSignal))
	{
		UE_LOG(LogTemp, Warning, TEXT("[RecievedIntent] 유효한 Intent, 실행: %s"), *IntentSignal.ToString());
		ActionRouter->ExcuteActionByIntent(IntentSignal);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[RecievedIntent] 유효하지 않은 Intent: %s"), *IntentSignal.ToString());
	}
}

void UCBrainComponent::OnLookInput(const FVector2D& LookAxis)
{
	if (Mode != EBrainMode::Player || !MoveRouter) return;
	MoveRouter->HandleLookInput(LookAxis.X, LookAxis.Y); // (Yaw좌우, Pitch상하)
}

void UCBrainComponent::OnPlayerAttackInput(FName IntentSignal)
{
	// 플레이어의 공격 입력 감지
	if (IntentSignal == FName("Input.Fire") ||
		IntentSignal == FName("Input.Skill1") ||
		IntentSignal == FName("Input.Skill2"))
	{
		UE_LOG(LogTemp, Warning, TEXT("[CBrainComponent] 플레이어 공격 감지: %s"), *IntentSignal.ToString());

		// AI 모드이고 AIBrain이 있으면 회피 신호 전송
		if (Mode == EBrainMode::AI && AIBrain)
		{
			AIBrain->OnPlayerAttackDetected();
		}
	}
}

bool UCBrainComponent::CehckPlayerMode()
{
	if (Mode==EBrainMode::Player)
	{
		return true;
	}
	return false;
}

void UCBrainComponent::AutoDetectBrainMode()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("CBrainComponent: Owner is not a Pawn"));
		Mode = EBrainMode::AI; // 기본값으로 AI 설정
		return;
	}

	// GameMode에서 DefaultPawnClass 확인
	AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (!GameMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("CBrainComponent: GameMode not found"));
		Mode = EBrainMode::AI;
		return;
	}

	// 현재 폰이 Default Pawn인지 확인
	bool bIsDefaultPawn = false;

	// 방법 1: PlayerController가 소유한 폰인지 확인
	if (OwnerPawn->GetController() && OwnerPawn->GetController()->IsA<APlayerController>())
	{
		bIsDefaultPawn = true;
	}

	// 방법 2: GameMode의 DefaultPawnClass와 비교
	if (!bIsDefaultPawn && GameMode->DefaultPawnClass)
	{
		if (OwnerPawn->GetClass() == GameMode->DefaultPawnClass ||
			OwnerPawn->GetClass()->IsChildOf(GameMode->DefaultPawnClass))
		{
			// PlayerPawn과 같은 폰이면 PlayerController가 있을 때만 Player 모드
			if (OwnerPawn->GetController() && OwnerPawn->GetController()->IsA<APlayerController>())
			{
				bIsDefaultPawn = true;
			}
		}
	}

	// 결과에 따라 모드 설정
	if (bIsDefaultPawn)
	{
		Mode = EBrainMode::Player;
		UE_LOG(LogTemp, Warning, TEXT("CBrainComponent: %s는 Player 모드로 설정됨"), *OwnerPawn->GetName());
	}
	else
	{
		Mode = EBrainMode::AI;
		UE_LOG(LogTemp, Warning, TEXT("CBrainComponent: %s는 AI 모드로 설정됨"), *OwnerPawn->GetName());
	}
}
